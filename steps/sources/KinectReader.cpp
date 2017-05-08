/*
  This file is part of the action parsing project.

  Copyright (C) 2013 David Lobato <dav.lobato@gmail.com>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// LOCAL INCLUDES
#include "KinectReader.h"
#include "data_structures/AsyncGrabber.h"

// PROJECT INCLUDES
#include <pcl/point_types.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/io/openni_camera/openni_driver.h>

// SYSTEM INCLUDES
#include <iostream>
#include <sstream>
#include <cedar/auxiliaries/UIntParameter.fwd.h>
#include <cedar/auxiliaries/EnumParameter.h>
#include <cedar/auxiliaries/assert.h>
#include <cedar/auxiliaries/exceptions.h>
#include <cedar/auxiliaries/Log.h>

#include <QWriteLocker>


//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

struct cedar::proc::steps::KinectReader::PImpl
{
  boost::shared_ptr<pcl::AsyncGrabber> grabber;
  pcl::VoxelGrid<cedar::aux::RGBAPointCloud::PointType> downsampling_grid;
};

cedar::proc::steps::KinectReader::KinectReader()
  :
  cedar::proc::Step(true)																																				,
  mPointCloud(new cedar::aux::RGBAPointCloudData(cedar::aux::RGBAPointCloud())),
  mImage(new cedar::aux::MatData(cv::Mat::zeros(480, 640, CV_8UC3))),
  mDepthImage( new cedar::aux::MatData(cv::Mat::zeros(480, 640, CV_32FC1))),
  pImpl(new PImpl()),
  //mViewer("PCL OpenNI Table Top Segmentation"),
  _mKinectID(new cedar::aux::StringParameter(this,"Kinect device ID","")),
  _mGrabPC(new cedar::aux::BoolParameter(this,"Grab point cloud",false)),
  _mGrabRGBImage(new cedar::aux::BoolParameter(this,"Grab RGB image",true)),
  _mGrabDepthImage(new cedar::aux::BoolParameter(this,"Grab depth image",true)),
  _mRemoveNaNPoints(new cedar::aux::BoolParameter(this,"Remove NaN points",true)),
  _mRemoveNaNDepth(new cedar::aux::BoolParameter(this,"Remove NaN Points in Depth",true)),
  _mDownsample(new cedar::aux::BoolParameter(this,"Downsample",false)),
  _mDownsampleZMinLimit(new cedar::aux::DoubleParameter(this,
                        "Downsampling z min limit",
                        0.0,
                        0.0,
                        std::numeric_limits<double>::max())),
  _mDownsampleZMaxLimit(new cedar::aux::DoubleParameter(this,
                        "Downsampling z max limit",
                        2.0,
                        0.0,
                        std::numeric_limits<double>::max())),
  _mDownsampleLeafSize(new cedar::aux::DoubleParameter(this,
                       "Downsampling leaf size",
                       0.005,
                       0.0,
                       0.1))
{
  cedar::aux::LogSingleton::getInstance()->allocating(this);

  this->declareOutput("rgbapointcloud", mPointCloud);
  this->declareOutput("image", mImage);
  this->declareOutput("depthimage", mDepthImage);

  QObject::connect(_mKinectID.get(), SIGNAL(valueChanged()), this, SLOT(setKinectID()));
  QObject::connect(_mDownsample.get(), SIGNAL(valueChanged()), this, SLOT(setDownsamplingGrid()));
  QObject::connect(_mDownsampleZMinLimit.get(), SIGNAL(valueChanged()), this, SLOT(setDownsamplingGrid()));
  QObject::connect(_mDownsampleZMaxLimit.get(), SIGNAL(valueChanged()), this, SLOT(setDownsamplingGrid()));
  QObject::connect(_mDownsampleLeafSize.get(), SIGNAL(valueChanged()), this, SLOT(setDownsamplingGrid()));

  //downsampling grid
  setDownsamplingGrid();

  //openni initialization
  openni_wrapper::OpenNIDriver& driver = openni_wrapper::OpenNIDriver::getInstance();
  if (driver.getNumberDevices () > 0)
  {
    std::stringstream ss;
    for (unsigned device_idx = 0; device_idx < driver.getNumberDevices (); ++device_idx)
    {
      ss << "Device: " << device_idx + 1 << ", vendor: " << driver.getVendorName (device_idx) << ", product: " << driver.getProductName (device_idx)
         << ", connected: " << driver.getBus (device_idx) << " @ " << driver.getAddress (device_idx) << ", serial number: \'" << driver.getSerialNumber (device_idx) << "\'" << std::endl;
      //   cout << "device_id may be #1, #2, ... for the first second etc device in the list or" << endl
      //    << "         bus@address for the device connected to a specific usb-bus / address combination (works only in Linux) or" << endl
      //    << "         <serial-number> (only in Linux and for devices which provide serial numbers)"  << endl;
      // }
    }
    cedar::aux::LogSingleton::getInstance()->systemInfo(ss.str(),"PointCloudOpenniGrabber");

    _mKinectID->setValue(driver.getSerialNumber (0));//FIXME: fill a list with all the devices
    pImpl->grabber.reset(new pcl::AsyncGrabber(_mKinectID->getValue()));
    pImpl->grabber->start();
  }
}

cedar::proc::steps::KinectReader::~KinectReader()
{
  if (pImpl->grabber)
  {
    pImpl->grabber->stop();
  }
  cedar::aux::LogSingleton::getInstance()->freeing(this);
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

void cedar::proc::steps::KinectReader::compute(const cedar::proc::Arguments&)
{
  if(_mGrabPC->getValue())
  {
    cedar::aux::ConstRGBAPointCloudPtr grabbed_cloud(pImpl->grabber->getLatestCloud());
    if (grabbed_cloud) //if null, there is no new data since last grab
    {
      cedar::aux::RGBAPointCloudPtr temp_cloud(new cedar::aux::RGBAPointCloud);
      bool returnTemp = false;

      //if downsampling active
      if (_mDownsample->getValue())
      {
        pImpl->downsampling_grid.setInputCloud (grabbed_cloud);
        pImpl->downsampling_grid.filter(*temp_cloud);
        returnTemp = true;
      }

      if (_mRemoveNaNPoints->getValue())
      {
        std::vector<int> mapping; //unused but mandatory

        if(returnTemp)
        {
          removeNaNFromPointCloud(*temp_cloud, *temp_cloud, mapping);
        }
        else
        {
          removeNaNFromPointCloud(*grabbed_cloud, *temp_cloud, mapping);
          returnTemp = true;
        }
      }

      if(returnTemp)
      {
        this->mPointCloud->setData(*temp_cloud);
      }
      else
      {
        this->mPointCloud->setData(*grabbed_cloud);
      }
    }//grabbedCloud
  }//GrabPC

  if( _mGrabRGBImage->getValue())
  {
    boost::shared_ptr<openni_wrapper::Image> new_image(pImpl->grabber->getLatestImage());
    if(new_image)
    {
      cv::Mat &m_image = this->mImage->getData();
      m_image.create(new_image->getHeight(),new_image->getWidth(),CV_8UC3);
      new_image->fillRGB(m_image.cols, m_image.rows, m_image.ptr<unsigned char>(0));
      cv::cvtColor(m_image,m_image,CV_RGB2BGR);
    }
  }

  if( _mGrabDepthImage->getValue())
  {
    boost::shared_ptr<openni_wrapper::DepthImage> new_depthImage(pImpl->grabber->getLatestDepthImage());

    if(new_depthImage)
    {
      cv::Mat &m_depthimage = this->mDepthImage->getData();
      m_depthimage.create(new_depthImage->getHeight(), new_depthImage->getWidth(), CV_32FC1);
      new_depthImage->fillDepthImage( m_depthimage.cols, m_depthimage.rows, m_depthimage.ptr<float>(0));
    }

    if( _mRemoveNaNDepth->getValue())
    {
      removeNanPointFromDepthImage();
    }
  }
}

void cedar::proc::steps::KinectReader::setKinectID() {}//FIXME: implement

void cedar::proc::steps::KinectReader::setDownsamplingGrid()
{
  if (_mDownsample->getValue())
  {
    //downsampling active
    const double& zMinLimit = _mDownsampleZMinLimit->getValue();
    const double& zMaxLimit = _mDownsampleZMaxLimit->getValue();
    const double& leafSize = _mDownsampleLeafSize->getValue();

    //check params:
    if (zMinLimit >= zMaxLimit)
    {
      CEDAR_THROW(cedar::aux::RangeException,"Downsampling z limits is an invalid range");
      return;
    }

    QWriteLocker locker(&this->mPointCloud->getLock());//FIXME: should I use this lock??
    pImpl->downsampling_grid.setFilterFieldName("z");
    pImpl->downsampling_grid.setFilterLimits(zMinLimit, zMaxLimit);
    pImpl->downsampling_grid.setLeafSize(leafSize,leafSize,leafSize);//leaf size equal on each dimension
  }
}

void cedar::proc::steps::KinectReader::removeNanPointFromDepthImage()
{	
  cv::Mat &temp_image = this->mDepthImage->getData() ;

  for (int i = 0; i < temp_image.cols; ++i)
  {
    for (int j = 0; j < temp_image.rows; ++j)
    {
      //	This test is true only if the value of temp_image.at<float_t>(j,i) is NaN
      if(temp_image.at<float_t>(j,i) != temp_image.at<float_t>(j,i))
      {
        temp_image.at<float_t>(j,i) = 8.0f;
      }
    }
  }

  this->mDepthImage->setData(temp_image);
}
