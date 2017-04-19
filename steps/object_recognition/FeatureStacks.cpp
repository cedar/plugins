/*======================================================================================================================

    Copyright 2011, 2012, 2013 Institut fuer Neuroinformatik, Ruhr-Universitaet Bochum, Germany
 
    This file is part of cedar.

    cedar is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the
    Free Software Foundation, either version 3 of the License, or (at your
    option) any later version.

    cedar is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
    License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with cedar. If not, see <http://www.gnu.org/licenses/>.

========================================================================================================================

    Institute:   Ruhr-Universitaet Bochum
                 Institut fuer Neuroinformatik

    File:        FeatureStacks.cpp

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2014 01 28

    Description: Source file for the class keypoints::FeatureStacks.

    Credits:

======================================================================================================================*/

// CEDAR CONFIGURATION
#include "cedar/configuration.h"

// PROJECT INCLUDES
#include "steps/object_recognition/FeatureStacks.h"

// CEDAR INCLUDES
#include "cedar/auxiliaries/MatData.h"
#include "cedar/processing/typecheck/Matrix.h"

// SYSTEM INCLUDES
#include <keypoints/keypoints.h>

#if CV_MAJOR_VERSION <= 2
#include <opencv2/nonfree/nonfree.hpp>
#endif

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::proc::steps::FeatureStacks::FeatureStacks()
:
mColorLabels(new cedar::aux::MatData(cv::Mat())),
mColorCenters(new cedar::aux::MatData(cv::Mat())),
mOutput(new cedar::aux::MatData(cv::Mat())),
_mNumberOfScales(new cedar::aux::UIntParameter(this, "number of scales", 2)),
_mLambdasStart(new cedar::aux::DoubleParameter(this, "lambdas start", 8.0)),
_mLambdasEnd(new cedar::aux::DoubleParameter(this, "lambdas end", 64.0)),
_mSubsamplingFactor(new cedar::aux::DoubleParameter(this, "subsampling factor", 1.0, cedar::aux::DoubleParameter::LimitType::fromLower(1.0))),
_mPaletteFile(new cedar::aux::FileParameter(this, "palette", cedar::aux::FileParameter::READ, "")),
_mDither(new cedar::aux::BoolParameter(this, "enable dithering", true)),
_mUseSiftFeatures(new cedar::aux::BoolParameter(this, "use SIFT features", false)),
_mLuminanceThreshold(new cedar::aux::UIntParameter(this, "lightness threshold", 255, cedar::aux::UIntParameter::LimitType(0, 255)))
{
  auto image_slot = this->declareInput("image");
  cedar::proc::typecheck::Matrix image_check;
  image_check.addAcceptedDimensionality(2);
  image_check.addAcceptedType(CV_8UC3);
  image_slot->setCheck(image_check);


  this->declareInput("mask", false);

  this->declareBuffer("color labels", mColorLabels);
  this->declareBuffer("color centers", mColorCenters);

  this->declareOutput("feature stacks", mOutput);

  // mLambdas = makeLambdasLin(64, 64, 1);
  this->lambdasChanged();

  this->connect(this->_mNumberOfScales.get(), SIGNAL(valueChanged()), SLOT(lambdasChanged()));
  this->connect(this->_mLambdasStart.get(), SIGNAL(valueChanged()), SLOT(lambdasChanged()));
  this->connect(this->_mLambdasEnd.get(), SIGNAL(valueChanged()), SLOT(lambdasChanged()));

  this->connect(this->_mPaletteFile.get(), SIGNAL(valueChanged()), SLOT(paletteChanged()));
  this->connect(this->_mSubsamplingFactor.get(), SIGNAL(valueChanged()), SLOT(subsamplingChanged()));

  this->connect(this->_mDither.get(), SIGNAL(valueChanged()), SLOT(recompute()));
  this->connect(this->_mLuminanceThreshold.get(), SIGNAL(valueChanged()), SLOT(recompute()));
  this->connect(this->_mUseSiftFeatures.get(), SIGNAL(valueChanged()), SLOT(recompute()));
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

void cedar::proc::steps::FeatureStacks::recompute()
{
  this->onTrigger();
}

void cedar::proc::steps::FeatureStacks::subsamplingChanged()
{
  this->allocateOutput();
  this->onTrigger();
}

void cedar::proc::steps::FeatureStacks::paletteChanged()
{
  try
  {
    std::string filename = this->_mPaletteFile->getPath();
    std::cout << "Reading: " << filename << std::endl;
    cv::FileStorage f;
    f.open(filename.c_str(), cv::FileStorage::READ);
    f["labels"]  >> this->mColorLabels->getData();
    f["centers"] >> this->mColorCenters->getData();
    f.release();
  }
  catch (const cv::Exception& e)
  {
    cedar::aux::LogSingleton::getInstance()->warning
    (
      "The palette could not be loaded because a cv exception was caught while loading it. Exception: "
        + std::string(e.what()),
      "void keypoints::FeatureStacks::paletteChanged()"
    );
    return;
  }

  this->allocateOutput();
  this->onTrigger();
}

void cedar::proc::steps::FeatureStacks::lambdasChanged()
{
  mLambdas = vislab::keypoints::makeLambdasLin(this->_mLambdasStart->getValue(), this->_mLambdasEnd->getValue(), this->_mNumberOfScales->getValue());
  this->allocateOutput();
  this->onTrigger();
}

int cedar::proc::steps::FeatureStacks::getNumberOfColors() const
{
  cv::Mat& centers = this->mColorCenters->getData();

  if (centers.empty())
  {
    // no palette loaded yet, do nothing
    return 0;
  }

  return centers.rows;
}

void cedar::proc::steps::FeatureStacks::compute(const cedar::proc::Arguments&)
{
  // Load the image, in colour and in BW
  const cv::Mat& imgBGR = this->mInput->getData();
  cv::Mat& labels = this->mColorLabels->getData();
  cv::Mat& centers = this->mColorCenters->getData();

  if ( (labels.empty() && !this->_mUseSiftFeatures->getValue()) || centers.empty())
  {
    // no palette loaded yet, do nothing
    return;
  }

  int nrColors = this->getNumberOfColors();

  cv::Mat& matstack = this->mOutput->getData();
  matstack.setTo(0.0);

  // Convert to image to grayscale (for keypoints) and LAB (for colour histogram)
  cv::Mat imgLAB, imgGray;
  cvtColor(imgBGR, imgLAB, cv::COLOR_BGR2Lab);
  cvtColor(imgBGR, imgGray, cv::COLOR_BGR2GRAY);

  // Extract keypoints from the image
  for(unsigned scalenum=0; scalenum<this->mLambdas.size(); scalenum++)
  {
    std::vector<double> lambdas;
    lambdas.push_back(this->mLambdas[scalenum]);

    std::vector<vislab::keypoints::KPData> datas;
    std::vector<cv::KeyPoint> points = vislab::keypoints::keypoints(imgGray,lambdas,datas,8,true);

    if (points.empty())
    {
      continue;
    }

    // Get a stack of colour images as a feature vector
    std::vector<cv::Mat> stack;

    if (this->_mUseSiftFeatures->getValue())
    {
      stack = siftprototypefeatures(imgGray, points, centers, labels, nrColors);
    }
    else
    {
      stack = histograms(imgLAB, points, centers, labels, nrColors);
    }

    // Copy the stack of images into the 4D Matrix we created
    int mem_offset_0 = matstack.step[0] * static_cast<int>(scalenum);
    for(size_t color = 0; color < stack.size(); ++color)
    {
      memcpy
      (
        matstack.data + mem_offset_0 + matstack.step[1] * static_cast<int>(color),
        stack[color].data,
        stack[color].total() * sizeof(float)
      );
    }
  }

  // matstack now contains the 4d image description
}

void cedar::proc::steps::FeatureStacks::inputConnectionChanged(const std::string& inputName)
{
  cedar::aux::ConstMatDataPtr mat_data = boost::dynamic_pointer_cast<cedar::aux::ConstMatData>(this->getInput(inputName));
  if (inputName == "image")
  {
    this->mInput = mat_data;

    if (this->mInput)
    {
      this->allocateOutput();
    }
  }
  else if (inputName == "mask")
  {
    this->mMask = mat_data;
  }
}

void cedar::proc::steps::FeatureStacks::allocateOutput()
{
  if (!this->mInput)
  {
    return;
  }

  float scalefactor = static_cast<float>(this->_mSubsamplingFactor->getValue());

  const cv::Mat& imgBGR = this->mInput->getData();
  const int stacksize[] =
      {
        static_cast<int>(this->mLambdas.size()),
        static_cast<int>(this->getNumberOfColors()),
        static_cast<int>(imgBGR.rows / scalefactor),
        static_cast<int>(imgBGR.cols / scalefactor)
      };

  mOutput->setData(cv::Mat(4, stacksize, CV_32F, cv::Scalar(0)));

  this->emitOutputPropertiesChangedSignal("feature stacks");
}


std::vector<cv::Mat>
  cedar::proc::steps::FeatureStacks::siftprototypefeatures
  (
    cv::Mat img,
    std::vector<cv::KeyPoint> points,
    cv::Mat centers,
    cv::Mat /* labels */,
    int nrprotos
  )
{
#if defined (CV_MAJOR_VERSION) && CV_MAJOR_VERSION >= 3
#warning SIFT detection will not work: not implemented for opencv 3!
  return std::vector<cv::Mat>();
#else
  float scalefactor = this->_mSubsamplingFactor->getValue();

//  cv::Mat result(0,nrprotos,CV_32F);

  cv::SIFT extractor;
  cv::Mat descs;
//  extractor.compute(img, points, descs);
  extractor(img, cv::Mat(), points, descs, true);

  std::vector<cv::Mat> stack;
  for(int i=0; i<nrprotos; i++)
    stack.push_back(cv::Mat::zeros(img.rows/scalefactor, img.cols/scalefactor, CV_32F));

  // Get the Gauss-weighted colour histogram of the surrounding of each keypoint
  for(unsigned i=0; i<points.size(); i++)
  {
    cv::KeyPoint kp = points[i];
    cv::Mat siftdesc = descs.row(i);
//    double desc_norm = cv::norm(siftdesc);

//     std::cout << siftdesc << std::endl;
//     std::cout << centers << std::endl;
    std::vector<float> dists(nrprotos,0);
    for(int j=0; j<centers.rows && j<nrprotos; j++)
    {
//      dists.at(j) = 1000.0 / cv::norm(siftdesc-centers.row(j));
      dists.at(j) = 1000000.0 / std::pow(cv::norm(siftdesc-centers.row(j)), 2);
//      cv::Mat center = centers.row(j);
//      double center_norm = cv::norm(center);
//      dists.at(j) = center.dot(siftdesc) / (desc_norm * center_norm);
    }


    // Enter the histogram onto the stack of images
    for(int j=0; j<nrprotos; j++)
    {
      stack[j].at<float>(kp.pt.y/scalefactor,kp.pt.x/scalefactor) = dists[j];
    }
  }

  int ksize = points[0].size * 2/scalefactor; if(ksize % 2 == 0) ksize++;

  // Smear each histogram with a Gaussian to spread the descriptor around
  for(int j=0; j<nrprotos && j<static_cast<int>(stack.size()); j++)
    cv::GaussianBlur(stack[j], stack[j], cv::Size(ksize,ksize), 0, 0);

  return stack;
#endif
}


std::vector<cv::Mat>
  cedar::proc::steps::FeatureStacks::histograms
  (
    cv::Mat img,
    const std::vector<cv::KeyPoint>& points,
    cv::Mat centers,
    cv::Mat labels,
    int nrColors
  )
{
  cv::Mat labelimg, dithimg;

  unsigned int threshold_l = this->_mLuminanceThreshold->getValue();

  if (this->_mDither->getValue())
  {
    dithimg = floydSteinberg(img, centers, labels, labelimg);
  }

  float scalefactor = static_cast<float>(this->_mSubsamplingFactor->getValue());

  std::vector<cv::Mat> stack;
  for(unsigned i=0; i<static_cast<unsigned>(nrColors); i++)
      stack.push_back(cv::Mat::zeros(img.rows/scalefactor, img.cols/scalefactor, CV_32F));

  // Get the Gauss-weighted colour histogram of the surrounding of each keypoint
  for(unsigned i=0; i<points.size(); i++)
  {
    cv::KeyPoint kp = points[i];
    int radius = kp.size/2;
    float size2 = kp.size * kp.size;
    float sigma = kp.size/4;
    float twosigmasq = 2*sigma*sigma;

    std::vector<float> histogram(nrColors,0);

    for(int x=kp.pt.x-radius; x<=kp.pt.x+radius; x++)
    {
        if(x<0 || x>=dithimg.cols) continue;

        for(int y=kp.pt.y-radius; y<=kp.pt.y+radius; y++)
        {
            if(y<0 || y>=dithimg.rows) continue;

            auto lightness = img.at<cv::Vec3b>(y,x)[0];
            if (lightness > threshold_l)
            {
              continue;
            }

            if (this->mMask)
            {
              if (this->mMask->getData().at<unsigned char>(y, x) == 0.0)
              {
                continue;
              }
            }

            float distance2 = ((x-kp.pt.x)*(x-kp.pt.x)+(y-kp.pt.y)*(y-kp.pt.y));
            if(distance2>size2) continue;

            float weight = exp(distance2/twosigmasq);

            int index = labelimg.at<char>(y,x);

            histogram[index] += weight;
            if(index > nrColors) std::cout << "ERROR " << x << ", " << y << " = " << index << std::endl;
        }
    }

    // Enter the histogram onto the stack of images
    for(unsigned j=0; j<static_cast<unsigned>(nrColors); j++)
        if(kp.pt.y/scalefactor<stack[j].rows && kp.pt.y/scalefactor<stack[j].cols)
          stack[j].at<float>(kp.pt.y/scalefactor,kp.pt.x/scalefactor) = histogram[j]*10;
  }

  int ksize = points[0].size * 2/scalefactor; if(ksize % 2 == 0) ksize++;

  // Smear each histogram with a Gaussian to spread the descriptor around
  for(unsigned j=0; j<static_cast<unsigned>(nrColors) && j<stack.size(); j++)
      cv::GaussianBlur(stack[j], stack[j], cv::Size(ksize,ksize), 0, 0);

  // Display the dithering results, for fun!
  // cv::Mat fsBGR, outimg;
  // cvtColor(dithimg, fsBGR, CV_Lab2BGR);
  // drawKeypoints(fsBGR, points, outimg, Scalar::all(-1), 4);
  // imshow("fs",outimg); // floyd steinberg dithering
  // waitKey();

  return stack;
}

cv::Mat cedar::proc::steps::FeatureStacks::floydSteinberg(cv::Mat& imgOrig, cv::Mat& centers, cv::Mat& /*labels*/, cv::Mat& labelimg)
{
    // convert palette back to uchar
    cv::Mat palette;
    centers.convertTo(palette,CV_8UC3,1.0);

    cv::Mat img = imgOrig.clone();
    cv::Mat resImg = img.clone();
    cv::Mat resImg_nodither = img.clone();

    labelimg = cv::Mat::zeros(img.rows,img.cols,CV_8U);

    for(int i = 0; i < img.rows; i++ )
        for(int j = 0; j < img.cols; j++ )
        {
            int newindex = findClosestPaletteIndex(img.at<cv::Vec3b>(i,j), palette);
            cv::Vec3b newpixel = palette.at<cv::Vec3b>(newindex);
            resImg.at<cv::Vec3b>(i,j) = newpixel;
            resImg_nodither.at<cv::Vec3b>(i,j) = newpixel;
            labelimg.at<char>(i,j) = newindex;

            for(int k=0;k<3;k++)
            {
                int quant_error = (int)img.at<cv::Vec3b>(i,j)[k] - newpixel[k];
                if(i+1<img.rows)
                    img.at<cv::Vec3b>(i+1,j)[k] = std::min(255,std::max(0,(int)img.at<cv::Vec3b>(i+1,j)[k] + (7 * quant_error) / 16));
                if(i-1 > 0 && j+1 < img.cols)
                    img.at<cv::Vec3b>(i-1,j+1)[k] = std::min(255,std::max(0,(int)img.at<cv::Vec3b>(i-1,j+1)[k] + (3 * quant_error) / 16));
                if(j+1 < img.cols)
                    img.at<cv::Vec3b>(i,j+1)[k] = std::min(255,std::max(0,(int)img.at<cv::Vec3b>(i,j+1)[k] + (5 * quant_error) / 16));
                if(i+1 < img.rows && j+1 < img.cols)
                    img.at<cv::Vec3b>(i+1,j+1)[k] = std::min(255,std::max(0,(int)img.at<cv::Vec3b>(i+1,j+1)[k] + (1 * quant_error) / 16));
            }
        }
    return resImg;
    // return resImg_nodither;
}

float cedar::proc::steps::FeatureStacks::vec3bDist(cv::Vec3b a, cv::Vec3b b)
{
    return sqrt( pow((float)a[0]-b[0],2) + pow((float)a[1]-b[1],2) + pow((float)a[2]-b[2],2) );
}

cv::Vec3b cedar::proc::steps::FeatureStacks::findClosestPaletteColor(cv::Vec3b color, cv::Mat palette)
{
//    int i=0;
    int minI = 0;
//    cv::Vec3b diff = color - palette.at<Vec3b>(0);
    float minDistance = vec3bDist(color, palette.at<cv::Vec3b>(0));
    for (int i=0;i<palette.rows;i++)
    {
        float distance = vec3bDist(color, palette.at<cv::Vec3b>(i));
        if (distance < minDistance)
        {
            minDistance = distance;
            minI = i;
        }
    }
    return palette.at<cv::Vec3b>(minI);
}

int cedar::proc::steps::FeatureStacks::findClosestPaletteIndex(cv::Vec3b color, cv::Mat palette)
{
//    int i=0;
    int minI = 0;
//    cv::Vec3b diff = color - palette.at<Vec3b>(0);
    float minDistance = vec3bDist(color, palette.at<cv::Vec3b>(0));
    for (int i=0;i<palette.rows;i++)
    {
        float distance = vec3bDist(color, palette.at<cv::Vec3b>(i));
        if (distance < minDistance)
        {
            minDistance = distance;
            minI = i;
        }
    }
    return minI;
}
