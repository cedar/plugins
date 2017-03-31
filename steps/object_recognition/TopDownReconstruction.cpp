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

    File:        TopDownReconstruction.cpp

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2016 02 16

    Description: Source file for the class cedar::proc::steps::TopDownReconstruction.

    Credits:

======================================================================================================================*/

// CEDAR CONFIGURATION
#include "cedar/configuration.h"

// CLASS HEADER
#include "steps/object_recognition/TopDownReconstruction.h"

// CEDAR INCLUDES
#include "cedar/processing/typecheck/Matrix.h"
#include "cedar/auxiliaries/math/constants.h"

// SYSTEM INCLUDES

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::proc::steps::TopDownReconstruction::TopDownReconstruction()
:
// input slot helpers
mImage(this, "image"),
mTopDownPrediction(this, "top-down shape"),
mGradientOrientationImage(this, "gradient orientations"),
mGradientMagnitudeImage(this, "gradient magnitudes"),
// buffers
mReconstructedOrientations(new cedar::aux::MatData(cv::Mat())),
mReconstructedMagnitudes(new cedar::aux::MatData(cv::Mat())),
mReconstructionError(new cedar::aux::MatData(cv::Mat())),
// outputs
mReconstructedEdges(new cedar::aux::MatData(cv::Mat())),
// parameters
_mReconstructionMagnitudeThreshold(new cedar::aux::DoubleParameter(this, "reconstruction magnitude threshold", 1.0)),
_mGradientMagnitudeThreshold(new cedar::aux::DoubleParameter(this, "gradient magnitude threshold", 50.0))
{
  // declare buffers
  this->declareBuffer("reconstructed orientations", mReconstructedOrientations);
  this->declareBuffer("reconstructed magnitudes", mReconstructedMagnitudes);
  this->declareBuffer("reconstruction error", mReconstructionError);

  // declare the output data
  auto output_slot = this->declareOutput("output", mReconstructedEdges);

  cedar::proc::CopyMatrixPropertiesPtr copier(new cedar::proc::CopyMatrixProperties(output_slot));
  copier->setTypeOverride(CV_8UC3);
  mImage.addOutputRelation(copier);
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

void cedar::proc::steps::TopDownReconstruction::computeTopDownGradients()
{
  const auto& top_down_shape = this->mTopDownPrediction.getData();

  int num_orientations = top_down_shape.size[0];
  int rows = top_down_shape.size[1];
  int cols = top_down_shape.size[2];

  auto& magnitudes = this->mReconstructedMagnitudes->getData();
  auto& orientations = this->mReconstructedOrientations->getData();

  std::vector<float> orientation_angles(static_cast<size_t>(num_orientations));
  for (int o = 0; o < num_orientations; ++o)
  {
    orientation_angles[static_cast<size_t>(o)] = static_cast<float>(o) + cedar::aux::math::pi / static_cast<float>(num_orientations);
  }

  magnitudes.create(rows, cols, CV_32F);
  magnitudes.setTo(0.0);

  orientations.create(rows, cols, CV_32F);

  cv::Range slice_range[3];
  slice_range[1] = cv::Range::all();
  slice_range[2] = cv::Range::all();
  // calculate average magnitude
  for (int o = 0; o < num_orientations; ++o)
  {
    slice_range[0] = cv::Range(o, o + 1);
    cv::Mat slice_3d = top_down_shape(slice_range);
    cv::Mat slice_2d = cv::Mat(rows, cols, slice_3d.type(), slice_3d.data);
    magnitudes += slice_2d;
  }
  magnitudes /= static_cast<float>(num_orientations);

  slice_range[0] = cv::Range::all();
  for (int r = 0; r < rows; ++r)
  {
    slice_range[1] = cv::Range(r, r + 1);
    for (int c = 0; c < cols; ++c)
    {
      slice_range[2] = cv::Range(c, c + 1);
      cv::Mat orientation_distribution_3d = top_down_shape(slice_range);
      cv::Mat orientation_distribution_2d(num_orientations, 1, orientation_distribution_3d.type(), orientation_distribution_3d.data);

      float sin_sum = 0.0f, cos_sum = 0.0f;
      for (int o = 0; o < num_orientations; ++o)
      {
        float v = orientation_distribution_2d.at<float>(o);
        float angle = orientation_angles[o];
        sin_sum += v * std::sin(angle);
        cos_sum += v * std::cos(angle);
      }

      float mean_angle = std::atan2(sin_sum, cos_sum);
      orientations.at<float>(r, c) = mean_angle;
    }
  }
}

void cedar::proc::steps::TopDownReconstruction::calculateReconstructionError()
{
  float threshold = this->_mReconstructionMagnitudeThreshold->getValue();
  float gradient_threshold = this->_mGradientMagnitudeThreshold->getValue();

  auto& td_magnitudes = this->mReconstructedMagnitudes->getData();
  auto& td_orientations = this->mReconstructedOrientations->getData();
  const auto& bu_magnitudes = this->mGradientMagnitudeImage.getData();
  const auto& bu_orientations = this->mGradientOrientationImage.getData();
  auto& errors = this->mReconstructionError->getData();

  int rows = bu_magnitudes.rows;
  int cols = bu_magnitudes.cols;
  int scaling_factor = bu_magnitudes.rows / td_magnitudes.rows;

  errors.create(rows, cols, CV_32F);
  errors.setTo(0.0);

  float half_pi = cedar::aux::math::pi / 2.0f;

  for (int r = 0; r < rows; ++r)
  {
    int r_scaled = r / scaling_factor;
    for (int c = 0; c < cols; ++c)
    {
      int c_scaled = c / scaling_factor;
      float top_down_magnitude = td_magnitudes.at<float>(r_scaled, c_scaled);
      float bottom_up_magnitude = bu_magnitudes.at<float>(r, c);
      if (top_down_magnitude < threshold && bottom_up_magnitude < gradient_threshold)
      {
        continue;
      }

      float bottom_up_angle = bu_orientations.at<float>(r, c);
      float top_down_angle = td_orientations.at<float>(r_scaled, c_scaled);

      float error = std::fmod(std::abs(top_down_angle - bottom_up_angle), cedar::aux::math::pi);
      if (error > half_pi)
      {
        error = cedar::aux::math::pi - error;
      }

      errors.at<float>(r, c) = error / half_pi;
    }
  }
}

void cedar::proc::steps::TopDownReconstruction::compute(const cedar::proc::Arguments&)
{
  if
  (
    this->mGradientMagnitudeImage.getData().empty()
    || this->mGradientOrientationImage.getData().empty()
    || this->mTopDownPrediction.getData().dims < 3
  )
  {
    return;
  }

  auto& reconstructed_edges = this->mReconstructedEdges->getData();
  const auto& input = this->mImage.getData();
  auto& errors = this->mReconstructionError->getData();



  // as a basis, create a three-channel grayscale image
  cv::Mat channels[3];
  channels[0] = input.clone();
  channels[1] = input.clone();
  channels[2] = input.clone();

  // next, project the top-down shape to a single dimension
  this->computeTopDownGradients();

  // calculate the error
  this->calculateReconstructionError();

  // display the results
  cv::Mat converted_errors;
  converted_errors.create(errors.rows, errors.cols, CV_8U);
  cv::convertScaleAbs(errors, converted_errors, 255.0, 0.0);
  channels[0] = 1.0 - converted_errors;
  channels[2] = converted_errors;

  // finally, merge the three channels into one
  cv::merge(channels, 3, reconstructed_edges);
}
