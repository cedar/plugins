/*======================================================================================================================

    Copyright 2011, 2012, 2013, 2014, 2015 Institut fuer Neuroinformatik, Ruhr-Universitaet Bochum, Germany
 
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

    File:        KeypointEdgeHistogramExtractor.cpp

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2015 10 20

    Description: Source file for the class cedar::proc::steps::KeypointEdgeHistogramExtractor.

    Credits:

======================================================================================================================*/

// CEDAR CONFIGURATION
#include "cedar/configuration.h"

// CLASS HEADER
#include "steps/object_recognition/KeypointEdgeHistogramExtractor.h"

// CEDAR INCLUDES
#include "cedar/processing/typecheck/DerivedFrom.h"
#include "cedar/processing/typecheck/Matrix.h"
#include "cedar/processing/typecheck/SameSize.h"
#include "cedar/auxiliaries/math/sigmoids.h"
#include "cedar/auxiliaries/math/functions.h"
#include "cedar/auxiliaries/math/constants.h"
#include "cedar/auxiliaries/convolution/FFTW.h"

// SYSTEM INCLUDES

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::proc::steps::KeypointEdgeHistogramExtractor::KeypointEdgeHistogramExtractor()
:
mKeypointData(this, "keypoint data"),
mKeypoints(this, "keypoints"),
mEdgeLocations(this, "edge locations"),
mEdgeOrientations(this, "edge orientations"),
mGradientMagnitudes(this, "gradient magnitudes"),
mGradientOrientations(this, "gradient orientations"),
// buffers
mEdgeLocationsResampled(new cedar::aux::MatData(cv::Mat())),
mEdgeOrientationsResampled(new cedar::aux::MatData(cv::Mat())),
mGradientOrientationsResampled(new cedar::aux::MatData(cv::Mat())),
mGradientOrientationAcuities(new cedar::aux::MatData(cv::Mat())),
mGradientMagnitudesResampled(new cedar::aux::MatData(cv::Mat())),
mHistogramBuffer(new cedar::aux::MatData(cv::Mat())),
mKeypointDistanceBuffer(new cedar::aux::MatData(cv::Mat())),
// outputs
mKeypointHistograms(new cedar::aux::MatData(cv::Mat())),
mOrientationMap(new cedar::aux::MatData(cv::Mat())),
// parameters
_mEdgeStrengthThreshold(new cedar::aux::DoubleParameter(this, "edge strength threshold", 200.0f)),
_mGradientStrengthThreshold(new cedar::aux::DoubleParameter(this, "gradient strength threshold", 40.0f)),
_mKeypointRadiusMultiplier(new cedar::aux::DoubleParameter(this, "keypoint radius multiplier", 1.0f)),
_mSubsampling(new cedar::aux::UIntParameter(this, "subsampling factor", 1, cedar::aux::UIntParameter::LimitType::positive())),
_mUseGradient(new cedar::aux::BoolParameter(this, "use gradient", true)),
_mNormalize(new cedar::aux::BoolParameter(this, "normalize", true)),
_mRemoveKeypointsFromEdges(new cedar::aux::BoolParameter(this, "remove keypoints from edges", true)),
_mApplyBlurring(new cedar::aux::BoolParameter(this, "apply blurring", true)),
_mConvolution(new cedar::aux::conv::Convolution())
{
  // set the convolution engine to FFTW
  this->_mConvolution->setEngine(cedar::aux::conv::FFTWPtr(new cedar::aux::conv::FFTW()));

  this->declareBuffer("histogram", this->mHistogramBuffer);
  this->declareBuffer("keypoint location weight", this->mKeypointDistanceBuffer);
  this->declareBuffer("resampled edge locations", this->mEdgeLocationsResampled);
  this->declareBuffer("resampled orientation bins", this->mEdgeOrientationsResampled);
  this->declareBuffer("resampled gradient orientations", this->mGradientOrientationsResampled);
  this->declareBuffer("resampled gradient orientation acuities", this->mGradientOrientationAcuities);
  this->declareBuffer("resampled gradient magnitudes", this->mGradientMagnitudesResampled);
  this->declareOutput("keypoint histograms", this->mKeypointHistograms);
  this->declareOutput("orientation map", this->mOrientationMap);

  this->mKeypointData.getSlot()->setCheck(cedar::proc::typecheck::DerivedFrom<cedar::aux::KeypointData>());
  this->mKeypoints.getSlot()->setCheck(cedar::proc::typecheck::DerivedFrom<cedar::aux::KeypointListData>());

  cedar::proc::typecheck::SameSize size_check;
  size_check.addSlot(this->mEdgeLocations.getSlot());
  size_check.addSlot(this->mEdgeOrientations.getSlot());
  this->mEdgeLocations.getSlot()->setCheck(size_check);
  this->mEdgeOrientations.getSlot()->setCheck(size_check);

  /*cedar::proc::typecheck::Matrix scale_weight_check;
  scale_weight_check.addAcceptedDimensionality(1);
  scale_weight_check.addAcceptedType(CV_32F);
  this->mScaleWeights.getSlot()->setCheck(scale_weight_check);*/

  this->addConfigurableChild("convolution", this->_mConvolution);

  QObject::connect(this->_mSubsampling.get(), SIGNAL(valueChanged()), this, SLOT(samplingFactorChanged()));
  QObject::connect(this->_mEdgeStrengthThreshold.get(), SIGNAL(valueChanged()), this, SLOT(recompute()));
  QObject::connect(this->_mUseGradient.get(), SIGNAL(valueChanged()), this, SLOT(recompute()));
  QObject::connect(this->_mGradientStrengthThreshold.get(), SIGNAL(valueChanged()), this, SLOT(recompute()));
  QObject::connect(this->_mNormalize.get(), SIGNAL(valueChanged()), this, SLOT(recompute()));
  QObject::connect(this->_mRemoveKeypointsFromEdges.get(), SIGNAL(valueChanged()), this, SLOT(recompute()));
  QObject::connect(this->_mApplyBlurring.get(), SIGNAL(valueChanged()), this, SLOT(recompute()));
  QObject::connect(this->_mKeypointRadiusMultiplier.get(), SIGNAL(valueChanged()), this, SLOT(recompute()));
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

void cedar::proc::steps::KeypointEdgeHistogramExtractor::recompute()
{
  this->onTrigger();
}

void cedar::proc::steps::KeypointEdgeHistogramExtractor::updateOutputSize()
{
  cv::Mat old_output = this->mKeypointHistograms->getData().clone();
  // only do something if all inputs are set
  if (this->mKeypointData.get() && this->mKeypoints.get() && this->mEdgeLocations.get() && this->mEdgeOrientations.get())
  {
    int subsampling_factor = static_cast<int>(this->getSubsamplingFactor());

    const cv::Mat& input = this->mEdgeLocations.getData();
    const cedar::aux::KeypointData& keypoint_data = this->mKeypointData.getData();

    if (keypoint_data.getData().size() > 0)
    {
      int n_scales = keypoint_data.getData().size();
      int sizes_subsampled[4], sizes[4];
      sizes[0] = sizes_subsampled[0] = n_scales; // number of scales
      sizes[1] = sizes_subsampled[1] = keypoint_data.getData()[0].norientations; // number of orientations
      sizes[2] = input.size[1]; // columns of the input image
      sizes[3] = input.size[2]; // rows in the input image
      sizes_subsampled[2] = sizes[2] / subsampling_factor; // columns of the input image
      sizes_subsampled[3] = sizes[3] / subsampling_factor; // rows in the input image

      this->mKeypointHistograms->getData().create(4, sizes_subsampled, CV_32F);
      this->mHistogramBuffer->getData().create(4, sizes_subsampled, CV_32F);
      this->mOrientationMap->getData().create(4, sizes_subsampled, CV_32F);

      int distance_sizes[3];
      distance_sizes[0] = input.size[0];
      distance_sizes[1] = input.size[1] / subsampling_factor;
      distance_sizes[2] = input.size[2] / subsampling_factor;
      this->mKeypointDistanceBuffer->getData().create(3, distance_sizes, CV_32F);
    }
    else
    {
      this->mKeypointHistograms->setData(cv::Mat());
      this->mHistogramBuffer->setData(cv::Mat());
      this->mOrientationMap->setData(cv::Mat());
    }
  }
  else
  {
    // reset data & emit signal
    this->mKeypointHistograms->setData(cv::Mat());
    this->mHistogramBuffer->setData(cv::Mat());
    this->mOrientationMap->setData(cv::Mat());
  }

  if
  (
    !cedar::aux::math::matrixSizesEqual(old_output, this->mKeypointHistograms->getData())
    || this->mKeypointHistograms->getData().type() != old_output.type()
  )
  {
    this->emitOutputPropertiesChangedSignal("keypoint histograms");
    this->emitOutputPropertiesChangedSignal("orientation map");
  }
}

void cedar::proc::steps::KeypointEdgeHistogramExtractor::samplingFactorChanged()
{
  this->updateOutputSize();
  this->recompute();
}

void cedar::proc::steps::KeypointEdgeHistogramExtractor::inputConnectionChanged(const std::string&)
{
  this->updateOutputSize();
}

void cedar::proc::steps::KeypointEdgeHistogramExtractor::compute(const cedar::proc::Arguments&)
{
  if (this->mHistogramBuffer->isEmpty() || this->mKeypointHistograms->isEmpty())
  {
    return;
  }

  float keypoint_radius = this->_mKeypointRadiusMultiplier->getValue();
  float edge_threshold = this->_mEdgeStrengthThreshold->getValue();
  const auto& keypoint_data = this->mKeypointData.getData();
  const auto& keypoint_list_data = this->mKeypoints.getData();

  int subsampling_factor = static_cast<int>(this->getSubsamplingFactor());

  bool use_gradient = this->_mUseGradient->getValue();
  bool apply_blurring = this->_mApplyBlurring->getValue();

  if (keypoint_data.size() == 0)
  {
    return;
  }
  int norientations = keypoint_data[0].norientations;

  cv::Mat gradient_orientations = this->mGradientOrientations.getData();
  cv::Mat gradient_magnitudes = this->mGradientMagnitudes.getData();

  const cv::Mat& edge_orientations = this->mEdgeOrientations.getData();
  cv::Mat edge_locations = this->mEdgeLocations.getData();
  cv::Mat& keypoint_distance_buffer = this->mKeypointDistanceBuffer->getData();

  cv::Mat& gradient_orientation_acuities = this->mGradientOrientationAcuities->getData();

  keypoint_distance_buffer = 0.0f;

  // convert orientation map to bins
  cv::Mat orientation_bins = cv::Mat::zeros(edge_orientations.dims, edge_orientations.size, CV_16U);
  CEDAR_DEBUG_ASSERT(edge_orientations.type() == CV_64F);

  auto angle_to_orientation = 1.0f / cedar::aux::math::pi * static_cast<double>(norientations);
  for (int scale = 0; scale < edge_orientations.size[0]; ++scale)
  {
    for (int r = 0; r < orientation_bins.size[1]; ++r)
    {
      for (int c = 0; c < orientation_bins.size[2]; ++c)
      {
        double orientation = edge_orientations.at<double>(scale, r, c);
        unsigned short bin = static_cast<unsigned short>(orientation * angle_to_orientation);
        orientation_bins.at<unsigned short>(scale, r, c) = bin;
      }
    }
  }

  // convert gradient orientations to bins
  cv::Mat gradient_orientation_bins;

  if (use_gradient)
  {
    gradient_orientation_bins.create(gradient_orientations.dims, gradient_orientations.size, CV_16U);
//    gradient_orientation_bins = cv::Mat::zeros(gradient_orientations.dims, gradient_orientations.size, CV_16U);
//    gradient_orientation_bins.setTo(0.0);
    gradient_orientation_acuities.create(gradient_orientations.dims, gradient_orientations.size, CV_32F);
//    gradient_orientation_acuities.setTo(0.0);

    auto bins = static_cast<unsigned short>(norientations);
    angle_to_orientation = static_cast<double>(bins) / (cedar::aux::math::pi);
    double angle_per_bin = cedar::aux::math::pi / static_cast<double>(bins);

    for (int r = 0; r < gradient_orientation_bins.size[0]; ++r)
    {
      for (int c = 0; c < gradient_orientation_bins.size[1]; ++c)
      {
        double orientation = gradient_orientations.at<float>(r, c);
        unsigned short lower_bin = static_cast<unsigned short>(std::floor(orientation * angle_to_orientation));
        // enter the bin into the bin map
        gradient_orientation_bins.at<unsigned short>(r, c) = lower_bin % bins;

        // determine the acuity indicator, i.e., how far between the lower and higher bin the value lies
        // an acuity of 1 means the value is exactly on the bin; an acuity of 0 would mean that the value should be
        // entered in the next bin; 0.5 would mean the value lies exactly in between and should be entered into both
        // bins equally; and so forth
        double lower_angle = static_cast<double>(lower_bin) / angle_to_orientation;
        double angle_remainder = orientation - lower_angle;
        double acuity = 1.0 - angle_remainder / angle_per_bin;

        // the acuity should always be a value in [0, 1)
        CEDAR_DEBUG_NON_CRITICAL_ASSERT(acuity >= 0.0);
        CEDAR_DEBUG_NON_CRITICAL_ASSERT(acuity - 1.0 < std::numeric_limits<float>::epsilon());

        // write the value into the acuity map
        gradient_orientation_acuities.at<float>(r, c) = acuity;
      }
    }

    this->mGradientOrientationsResampled->setData(gradient_orientation_bins);
    this->mGradientMagnitudesResampled->setData(gradient_magnitudes);
  }

  // resample the orientation bins and edge locations
  cv::Mat& resampled_orientation_bins = this->mEdgeOrientationsResampled->getData();
  cv::Mat& resampled_edge_locations = this->mEdgeLocationsResampled->getData();
  if (subsampling_factor > 1)
  {
    int resampled_size[3];
    resampled_size[0] = orientation_bins.size[0];
    resampled_size[1] = orientation_bins.size[1] / subsampling_factor;
    resampled_size[2] = orientation_bins.size[2] / subsampling_factor;

    cv::Range ranges[3];
    ranges[1] = ranges[2] = cv::Range::all();

    // resample orientation bins
    resampled_orientation_bins.create(3, resampled_size, orientation_bins.type());

    for (int scale = 0; scale < orientation_bins.size[0]; ++scale)
    {
      ranges[0] = cv::Range(scale, scale + 1);
      cv::Mat scale_slice = orientation_bins(ranges);
      cv::Mat resampled_scale_slice = resampled_orientation_bins(ranges);

      cv::Mat scale_slice_2d = cv::Mat(scale_slice.size[1], scale_slice.size[2], scale_slice.type(), scale_slice.data);
      cv::Mat resampled_scale_slice_2d = cv::Mat(resampled_scale_slice.size[1], resampled_scale_slice.size[2], resampled_scale_slice.type(), resampled_scale_slice.data);

      cv::resize(scale_slice_2d, resampled_scale_slice_2d, cv::Size(resampled_scale_slice_2d.cols, resampled_scale_slice_2d.rows), cv::INTER_NEAREST);
    }

    // resample edge locations
    resampled_edge_locations.create(3, resampled_size, edge_locations.type());

    for (int scale = 0; scale < orientation_bins.size[0]; ++scale)
    {
      ranges[0] = cv::Range(scale, scale + 1);
      cv::Mat scale_slice = edge_locations(ranges);
      cv::Mat resampled_scale_slice = resampled_edge_locations(ranges);

      cv::Mat scale_slice_2d = cv::Mat(scale_slice.size[1], scale_slice.size[2], scale_slice.type(), scale_slice.data);
      cv::Mat resampled_scale_slice_2d = cv::Mat(resampled_scale_slice.size[1], resampled_scale_slice.size[2], resampled_scale_slice.type(), resampled_scale_slice.data);

      cv::resize(scale_slice_2d, resampled_scale_slice_2d, cv::Size(resampled_scale_slice_2d.cols, resampled_scale_slice_2d.rows), cv::INTER_CUBIC);
    }
  } // subsampling_factor > 1
  else
  {
    this->mEdgeLocationsResampled->setData(edge_locations.clone());
    this->mEdgeOrientationsResampled->setData(orientation_bins);
    resampled_orientation_bins = orientation_bins;
    resampled_edge_locations = edge_locations;
  } // subsampling_factor > 1


  cv::Mat& histograms = this->mHistogramBuffer->getData();
  cv::Mat& keypoint_histograms = this->mKeypointHistograms->getData();
  histograms = 0.0f;
  keypoint_histograms = 0.0f;

  std::map<float, int> scale_map;
  std::vector<cv::Mat> scale_gaussians(keypoint_data.size());
  std::vector<cv::Mat> scale_gaussians_subsampled(keypoint_data.size());
  std::vector<cv::Mat> scale_gaussians_subsampled_normalized(keypoint_data.size());
  std::vector<cv::Mat> keypoint_histogram_gaussians(keypoint_data.size());

  //!@todo These only change when the keypoint data or some parameters change...
  std::vector<unsigned int> sizes(2), sizes_subsampled(2), sizes_multiplied(2);
  std::vector<double> sigmas(2), sigmas_subsampled(2), sigmas_multiplied(2);
  std::vector<double> centers(2), centers_subsampled(2), centers_multiplied(2);
  float size_factor = 6.0f;
  for (int l = 0; l < static_cast<int>(keypoint_data.size()); ++l)
  {
    const vislab::keypoints::KPData& data = keypoint_data[l];
    scale_map[data.lambda] = l;
    float sigma = 0.56f * data.lambda; // see Terzic et al., 2013: sigma/lambda = 0.56
    float sigma_subsampled = sigma / static_cast<float>(subsampling_factor);
    sizes[0] = sizes[1] = static_cast<unsigned int>(size_factor * sigma);
    sizes_multiplied[0] = sizes_multiplied[1] = sizes[0] * keypoint_radius;
    sizes_subsampled[0] = sizes_subsampled[1] = static_cast<unsigned int>(size_factor * sigma_subsampled);
    sigmas[0] = sigmas[1] = sigma;
    sigmas_multiplied[0] = sigmas_multiplied[1] = sigma * keypoint_radius;
    sigmas_subsampled[0] = sigmas_subsampled[1] = sigma_subsampled;
    centers[0] = centers[1] = static_cast<float>(sizes[0])/2.0f;
    centers_multiplied[0] = centers_multiplied[1] = static_cast<float>(sizes_multiplied[0])/2.0f;
    centers_subsampled[0] = centers_subsampled[1] = static_cast<float>(sizes_subsampled[0])/2.0f;
    scale_gaussians[l] = cedar::aux::math::gaussMatrix(2, sizes, 1.0, sigmas, centers, false);
    scale_gaussians_subsampled[l] = cedar::aux::math::gaussMatrix(2, sizes_subsampled, 1.0, sigmas_subsampled, centers_subsampled, false);
    keypoint_histogram_gaussians[l] = cedar::aux::math::gaussMatrix(2, sizes_multiplied, 1.0, sigmas_multiplied, centers_multiplied, false);

    auto norm = cv::norm(scale_gaussians_subsampled[l]);
    scale_gaussians_subsampled_normalized[l] = scale_gaussians_subsampled[l] / norm;
  }

  // == extract localized histograms ===================================================================================

  cv::Mat used_orientation_bins = orientation_bins;
  cv::Mat used_magnitudes = edge_locations;
  if (use_gradient)
  {
    CEDAR_DEBUG_ASSERT(orientation_bins.size[1] == gradient_orientation_bins.rows);
    CEDAR_DEBUG_ASSERT(orientation_bins.size[2] == gradient_orientation_bins.cols);
    used_orientation_bins = gradient_orientation_bins;
    used_magnitudes = gradient_magnitudes;
    edge_threshold = this->_mGradientStrengthThreshold->getValue();
  }

  int rows, cols;
  if (use_gradient)
  {
    rows = used_orientation_bins.rows;
    cols = used_orientation_bins.cols;
  }
  else
  {
    rows = used_orientation_bins.size[1];
    cols = used_orientation_bins.size[2];
  }

  int index[4], output_index[4];
  // iterate through all keypoints, form histograms
  for (const cv::KeyPoint& keypoint : keypoint_list_data)
  {
    float lambda = keypoint.size;
    int lambda_index = scale_map[lambda];
    index[0] = output_index[0] = lambda_index;
    index[2] = static_cast<int>(keypoint.pt.y);
    index[3] = static_cast<int>(keypoint.pt.x);
    output_index[2] = static_cast<int>(keypoint.pt.y / static_cast<float>(subsampling_factor));
    output_index[3] = static_cast<int>(keypoint.pt.x / static_cast<float>(subsampling_factor));

    // get the gaussian weight matrix
    const cv::Mat& gaussian = keypoint_histogram_gaussians[lambda_index];

    // iterate over small local neighborhood, enter (weighted) values into histogram
    int neighborhood_size = gaussian.rows;
    CEDAR_DEBUG_ASSERT(gaussian.cols == gaussian.rows);

    const int c_col = index[3]; // static_cast<int>(keypoint.pt.x);
    const int c_row = index[2]; // static_cast<int>(keypoint.pt.y);

    int start_offset = neighborhood_size / 2;
    int end_offset = neighborhood_size - start_offset;

    int row_start = std::max(0, c_row - start_offset);
    int row_end = std::min(c_row + end_offset, rows);
    int col_start = std::max(0, c_col - start_offset);
    int col_end = std::min(c_col + end_offset, cols);
    CEDAR_DEBUG_ASSERT(edge_locations.type() == CV_64F);
    for (int r = row_start; r < row_end; ++r)
    {
      int dr = r - row_start;
      for (int c = col_start; c < col_end; ++c)
      {
        int dc = c - col_start;
        unsigned short orientation_bin;
        if (use_gradient)
        {
          orientation_bin = used_orientation_bins.at<unsigned short>(r, c);
        }
        else
        {
          orientation_bin = used_orientation_bins.at<unsigned short>(lambda_index, r, c);
        }

        index[1] = output_index[1] = static_cast<int>(orientation_bin);
        // weight the contribution based on the distance from the keypoint
        float weight = gaussian.at<float>(dr, dc);

        float strength;
        if (use_gradient)
        {
          strength = used_magnitudes.at<float>(r, c);
        }
        else
        {
          strength = static_cast<float>(used_magnitudes.at<double>(lambda_index, r, c));
        }

        // modify weight by entry on edge location map
        if (strength >= edge_threshold)
        {
          if (use_gradient)
          {
            float acuity = gradient_orientation_acuities.at<float>(r, c);
            histograms.at<float>(output_index) += acuity * weight;
            output_index[1] = (orientation_bin + 1) % norientations;
            histograms.at<float>(output_index) += (1.0f - acuity) * weight;
          }
          else
          {
            histograms.at<float>(output_index) += weight;
          }
        }
      } // for col
    } // for row
  }

  // -- normalize and spread localized histograms --
  std::vector<cv::Range> hist_range;
  hist_range.assign(4, cv::Range::all());
  for (const cv::KeyPoint& keypoint : keypoint_list_data)
  {
    // determine keypoint information
    float lambda = keypoint.size;
    int lambda_index = scale_map[lambda];
    float kp_x = keypoint.pt.x;
    float kp_y = keypoint.pt.y;
    int kp_res_x = static_cast<int>(kp_x / static_cast<float>(subsampling_factor));
    int kp_res_y = static_cast<int>(kp_y / static_cast<float>(subsampling_factor));

    // normalize the histogram

    if (this->_mNormalize->getValue())
    {
      hist_range[0] = cv::Range(lambda_index, lambda_index + 1);
      hist_range[2] = cv::Range(kp_res_y, kp_res_y + 1);
      hist_range[3] = cv::Range(kp_res_x, kp_res_x + 1);

      cv::Mat hist_4d = histograms(&hist_range.front());

      int hist_index[4] = {0, 0, 0, 0};
      double sum = 0.0;
      for (hist_index[1] = 0; hist_index[1] < norientations; ++hist_index[1])
      {
        sum += hist_4d.at<float>(hist_index);
      }

      if (std::abs(sum) > std::numeric_limits<float>::epsilon())
      {
        hist_4d /= sum;
      }
    }

    // enter the keypoint into the distance buffer
    index[0] = output_index[0] = lambda_index;
    index[2] = kp_res_y;
    index[3] = kp_res_x;

    // get the Gaussian weight matrix
    const cv::Mat& gaussian = scale_gaussians_subsampled[lambda_index];

    int neighborhood_size = gaussian.rows;
    CEDAR_DEBUG_ASSERT(gaussian.cols == gaussian.rows);

    const int c_col = index[3];
    const int c_row = index[2];

    int start_offset = neighborhood_size / 2;

    // subtract the keypoint location from the distance buffer
    cv::Range distance_cutout[3];
    distance_cutout[0] = cv::Range(lambda_index, lambda_index + 1);
    distance_cutout[1] = cv::Range(c_row - start_offset, c_row - start_offset + neighborhood_size);
    distance_cutout[2] = cv::Range(c_col - start_offset, c_col - start_offset + neighborhood_size);

    cv::Range gaussian_range[3];
    gaussian_range[0] = cv::Range::all();
    gaussian_range[1] = cv::Range::all();
    gaussian_range[2] = cv::Range::all();

    int gaussian_3d_sizes[3];
    gaussian_3d_sizes[0] = 1;
    gaussian_3d_sizes[1] = gaussian.rows;
    gaussian_3d_sizes[2] = gaussian.cols;

    cv::Mat gaussian_3d(3, gaussian_3d_sizes, gaussian.type(), gaussian.data);

    for (int i = 1; i <= 2; ++i)
    {
      if (distance_cutout[i].start < 0)
      {
        gaussian_range[i] = cv::Range(-distance_cutout[i].start, neighborhood_size);
        distance_cutout[i].start = 0;
      }

      if (distance_cutout[i].end >= keypoint_distance_buffer.size[i])
      {
        gaussian_range[i].start = 0;
        gaussian_range[i].end = neighborhood_size - (distance_cutout[i].end - keypoint_distance_buffer.size[i]);
        distance_cutout[i].end = keypoint_distance_buffer.size[i];
      }
    }

    keypoint_distance_buffer(distance_cutout) += gaussian_3d(gaussian_range);
  }

  // convolve all scales with an appropriately-sized Gaussian
  keypoint_histograms = histograms.clone();

  //!@todo This could also be done with a 4d-kernel which just has an appropriately-sized Gaussian at each position
  //!      First tests suggest that approach is much slower, but they were performed with a kernel that was
  //!      constructed every compute call. Maybe storing it (thus calculating the Fourier-transform of the kernel just
  //!      once) might save time.
  if (apply_blurring)
  {
    for (size_t scale = 0; scale < scale_map.size(); ++scale)
    {
      const cv::Mat& gaussian = scale_gaussians_subsampled[scale];

      std::vector<cv::Range> cutout;
      cutout.assign(4, cv::Range::all());
      cutout[0].start = scale;
      cutout[0].end = scale + 1;

      // construct a new header for the Gaussian
      int gauss_sizes[3] = {1, gaussian.rows, gaussian.cols};
      cv::Mat gaussian_3d(3, gauss_sizes, gaussian.type(), gaussian.data);

      int slice_sizes[3] = {keypoint_histograms.size[1], keypoint_histograms.size[2], keypoint_histograms.size[3]};
      cv::Mat slice = keypoint_histograms(&cutout.front());
      cv::Mat kp_slice = cv::Mat(3, slice_sizes, keypoint_histograms.type(), slice.data);

      cv::Mat result = this->_mConvolution->convolve(kp_slice, gaussian_3d);
      result.copyTo(kp_slice);
    }
  }

  // pass the location map through a sigmoid
  double sigmoid_beta = 50.0;
  keypoint_distance_buffer = cedar::aux::math::sigmoidAbs(keypoint_distance_buffer, sigmoid_beta, 0.5);

  // == build orientation map ==========================================================================================
  cv::Mat& orientation_map = this->mOrientationMap->getData();
  orientation_map = 0.0f;

  cv::Mat gradient_populationcode;
  if (use_gradient)
  {
    int sizes[3];
    sizes[0] = 1;
    sizes[1] = norientations;
    sizes[2] = resampled_orientation_bins.size[1];
    sizes[3] = resampled_orientation_bins.size[2];
    gradient_populationcode = cv::Mat(4, sizes, CV_32F, cv::Scalar(0));
    float m_threshold = this->_mGradientStrengthThreshold->getValue();

    int index[4];
    index[0] = 0;
    for (int r = 0; r < gradient_orientation_bins.rows; ++r)
    {
      index[2] = r / subsampling_factor;
      for (int c = 0; c < gradient_orientation_bins.cols; ++c)
      {
        index[3] = c / subsampling_factor;
        float m = gradient_magnitudes.at<float>(r, c);
        auto o = used_orientation_bins.at<unsigned short>(r, c);
        if (m > m_threshold)
        {
          float acuity = gradient_orientation_acuities.at<float>(r, c);
          index[1] = o;
          gradient_populationcode.at<float>(index) += acuity;
          index[1] = (o + 1) % norientations;
          gradient_populationcode.at<float>(index) += (1.0f - acuity);
        }
      }
    }
  }

  float orientation_threshold = this->_mEdgeStrengthThreshold->getValue();
  int ori_index[4];
  for (int lambda_index = 0; lambda_index < static_cast<int>(keypoint_data.size()); ++lambda_index)
  {
    ori_index[0] = lambda_index;

    if (use_gradient)
    {
      cv::Range ranges[4];
      ranges[0] = cv::Range(lambda_index, lambda_index + 1);
      ranges[1] = cv::Range::all();
      ranges[2] = cv::Range::all();
      ranges[3] = cv::Range::all();
      gradient_populationcode.copyTo(orientation_map(ranges));
    }
    else
    {
      // extract the local edge information from the
      for (int r = 0; r < orientation_bins.size[1]; ++r)
      {
        ori_index[2] = r / subsampling_factor;
        for (int c = 0; c < orientation_bins.size[2]; ++c)
        {
          double edge_strength = edge_locations.at<double>(lambda_index, r, c);
          unsigned short orientation_bin = orientation_bins.at<unsigned short>(lambda_index, r, c);
          ori_index[1] = orientation_bin;
          ori_index[3] = c / subsampling_factor;

          if (edge_strength >= orientation_threshold)
          {
            orientation_map.at<float>(ori_index) += 1.0f;
          }
        } // for c
      } // for r
    }

    // -- convolve the result with a scale-appropriate Gaussian --
    const cv::Mat& gaussian = scale_gaussians_subsampled_normalized[lambda_index];
    int gaussian_size[3];
    gaussian_size[0] = 1;
    gaussian_size[1] = gaussian.rows;
    gaussian_size[2] = gaussian.cols;
    cv::Mat gaussian_3d = cv::Mat(3, gaussian_size, gaussian.type(), gaussian.data);

    std::vector<cv::Range> ranges;
    ranges.assign(4, cv::Range::all());
    ranges[0] = cv::Range(lambda_index, lambda_index + 1);
    cv::Mat slice = orientation_map(&ranges.front());
    int slice_size[3];
    slice_size[0] = slice.size[1];
    slice_size[1] = slice.size[2];
    slice_size[2] = slice.size[3];
    cv::Mat slice_3d(3, slice_size, slice.type(), slice.data);

    if (apply_blurring)
    {
      cv::Mat res = this->_mConvolution->convolve(slice_3d, gaussian_3d);
      res.copyTo(slice_3d);
    }

    float max = 0.0f;
    // subtract the keypoint distance buffer
    if (this->_mRemoveKeypointsFromEdges->getValue())
    {
      for (int r = 0; r < orientation_bins.size[1]; ++r)
      {
        ori_index[2] = r / subsampling_factor;
        for (int c = 0; c < orientation_bins.size[2]; ++c)
        {
          ori_index[3] = c / subsampling_factor;
          for (int orientation = 0; orientation < norientations; ++orientation)
          {
            ori_index[1] = orientation;
            float& mat_elem = orientation_map.at<float>(ori_index);
            mat_elem -= keypoint_distance_buffer.at<float>(lambda_index, ori_index[2], ori_index[3]);
            max = std::max(max, mat_elem);
          }
        }
      }
    }
    else
    {
      for (int r = 0; r < orientation_bins.size[1]; ++r)
      {
        ori_index[2] = r / subsampling_factor;
        for (int c = 0; c < orientation_bins.size[2]; ++c)
        {
          ori_index[3] = c / subsampling_factor;
          for (int orientation = 0; orientation < norientations; ++orientation)
          {
            ori_index[1] = orientation;
            max = std::max(max, orientation_map.at<float>(ori_index));
          }
        }
      }
    }

    if (this->_mNormalize->getValue() && max > 0.0f)
    {
      std::vector<cv::Range> ranges;
      ranges.assign(4, cv::Range::all());
      ranges[0] = cv::Range(lambda_index, lambda_index + 1);
      orientation_map(&ranges.front()) /= max;
    }
  }

  orientation_map.setTo(0.0, orientation_map < 0.0);
}
