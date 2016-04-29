/*======================================================================================================================

    Copyright 2011 Institut fuer Neuroinformatik, Ruhr-Universitaet Bochum, Germany

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

    File:        CrossCorrelation.cpp

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2012 01 26

    Description:

    Credits:

======================================================================================================================*/

// LOCAL INCLUDES
#include "steps/utilities/CrossCorrelation.h"

// CEDAR INCLUDES
#include <cedar/processing/Arguments.h>
#include <cedar/auxiliaries/math/tools.h>
#include <cedar/auxiliaries/BoolVectorParameter.h>
#include <cedar/auxiliaries/assert.h>
#include <cedar/auxiliaries/exceptions.h>
#include <cedar/auxiliaries/convolution/Convolution.h>
#include <cedar/auxiliaries/convolution/FFTW.h>

// SYSTEM INCLUDES
#include <boost/make_shared.hpp>

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::proc::steps::CrossCorrelation::CrossCorrelation()
:
mNormalizedInput(new cedar::aux::MatData(cv::Mat::zeros(10, 10, CV_32F))),
mNormalizedPattern(new cedar::aux::MatData(cv::Mat())),
mFullResult(new cedar::aux::MatData(cv::Mat())),
mCorrelation(new cedar::aux::MatData(cv::Mat())),
_mPadInput(new cedar::aux::BoolParameter(this, "pad input", true)),
_mUnPadOutput(new cedar::aux::BoolParameter(this, "unpad output", true)),
_mNormalize(new cedar::aux::BoolParameter(this, "normalize", false)),
_mCorrelatedDimensions(new cedar::aux::BoolVectorParameter(this, "correlated dimensions", 1, true)),
_mBorderType
(
  new cedar::aux::EnumParameter
  (
    this,
    "border type",
    cedar::aux::conv::BorderType::typePtr(),
    cedar::aux::conv::BorderType::Zero
  )
),
_mAlternateKernelCenter(new cedar::aux::BoolParameter(this, "alternate kernel center", false)),
_mApplySpatialFiltering(new cedar::aux::BoolParameter(this, "apply spatial filtering", false)),
_mSpatialFilteringThreshold(new cedar::aux::DoubleParameter(this, "spatial filtering threshold", 0.01))
{
  this->declareInput("input");
  this->declareInput("pattern");

  this->declareBuffer("normalized input", mNormalizedInput);
  this->declareBuffer("normalized pattern", mNormalizedPattern);
  this->declareBuffer("full result", mFullResult);

  this->declareOutput("crossCorrelation", mCorrelation);
  QObject::connect(_mPadInput.get(), SIGNAL(valueChanged()), SLOT(recompute()), Qt::DirectConnection);
  QObject::connect(_mUnPadOutput.get(), SIGNAL(valueChanged()), SLOT(recompute()), Qt::DirectConnection);
  QObject::connect(_mCorrelatedDimensions.get(), SIGNAL(valueChanged()), SLOT(recompute()), Qt::DirectConnection);
  QObject::connect(_mBorderType.get(), SIGNAL(valueChanged()), SLOT(recompute()), Qt::DirectConnection);
  QObject::connect(_mNormalize.get(), SIGNAL(valueChanged()), SLOT(recompute()), Qt::DirectConnection);
  QObject::connect(_mAlternateKernelCenter.get(), SIGNAL(valueChanged()), SLOT(alternateKernelCenterChanged()), Qt::DirectConnection);
  QObject::connect(_mApplySpatialFiltering.get(), SIGNAL(valueChanged()), SLOT(applySpatialFilteringChanged()), Qt::DirectConnection);
  QObject::connect(_mSpatialFilteringThreshold.get(), SIGNAL(valueChanged()), SLOT(recompute()), Qt::DirectConnection);
}

cedar::proc::steps::CrossCorrelation::~CrossCorrelation()
{
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

void cedar::proc::steps::CrossCorrelation::applySpatialFilteringChanged()
{
  this->_mSpatialFilteringThreshold->setConstant(!this->_mApplySpatialFiltering->getValue());
  this->recompute();
}

void cedar::proc::steps::CrossCorrelation::alternateKernelCenterChanged()
{
  if (this->mConvolution)
  {
    auto parameter = this->mConvolution->getParameter("alternate even kernel center");
    if (!parameter)
    {
      cedar::aux::LogSingleton::getInstance()->warning("Correlation could not find alternate kernel center parameter.", CEDAR_CURRENT_FUNCTION_NAME);
      return;
    }
    auto bool_parameter = boost::dynamic_pointer_cast<cedar::aux::BoolParameter>(parameter);
    bool_parameter->setValue(this->_mAlternateKernelCenter->getValue());
  }
}

cedar::proc::DataSlot::VALIDITY cedar::proc::steps::CrossCorrelation::determineInputValidity
                                (
                                  cedar::proc::ConstDataSlotPtr slot,
                                  cedar::aux::ConstDataPtr data
                                )
const
{
  auto input_slot = this->getInputSlot("input");
  auto pattern_slot = this->getInputSlot("pattern");
  if (slot == input_slot || slot == pattern_slot)
  {
    if (auto mat_data = boost::dynamic_pointer_cast<const cedar::aux::MatData>(data))
    {
      // empty data is never accepted
      if (mat_data->isEmpty())
      {
        return cedar::proc::DataSlot::VALIDITY_ERROR;
      }

      // if the opposite slot has not been connected, we accept
      if ((slot == pattern_slot && !this->mInput) || (slot == input_slot && !this->mPattern))
      {
        return cedar::proc::DataSlot::VALIDITY_VALID;
      }

      cv::Mat input, pattern;

      if (slot == input_slot)
      {
        input = mat_data->getData();
      }
      else if (slot == pattern_slot)
      {
        pattern = mat_data->getData();
      }

      if (input.empty() && this->mInput)
      {
        input = this->mInput->getData();
      }
      if (pattern.empty() && this->mPattern)
      {
        pattern = this->mPattern->getData();
      }

      if (!pattern.empty() && !input.empty())
      {
        if (cedar::aux::math::getDimensionalityOf(pattern) == 1)
        {
          if (cedar::aux::math::getDimensionalityOf(input) == 1)
          {
            if (cedar::aux::math::get1DMatrixSize(input) >= cedar::aux::math::get1DMatrixSize(pattern))
            {
              return cedar::proc::DataSlot::VALIDITY_VALID;
            }
            else
            {
              return cedar::proc::DataSlot::VALIDITY_ERROR;
            }
          }
          else
          {
            if (static_cast<unsigned int>(input.rows) >= cedar::aux::math::get1DMatrixSize(pattern))
            {
              return cedar::proc::DataSlot::VALIDITY_VALID;
            }
            else
            {
              return cedar::proc::DataSlot::VALIDITY_ERROR;
            }
          }
        }
        else // pattern is ND
        {
          auto input_dim = cedar::aux::math::getDimensionalityOf(input);
          auto pattern_dim = cedar::aux::math::getDimensionalityOf(pattern);
          switch (input_dim)
          {
            case 4:
            case 3:
              if (input_dim == pattern_dim)
              {
                if (cedar::aux::math::matrixSizesEqual(input, pattern) && input.type() == pattern.type())
                {
                  return cedar::proc::DataSlot::VALIDITY_VALID;
                }
                else
                {
                  return cedar::proc::DataSlot::VALIDITY_ERROR;
                }
              }
              else if (input_dim - 1 == pattern_dim)
              {
                //!@todo Check that sizes in the correlated dimensions are equal
                return cedar::proc::DataSlot::VALIDITY_VALID;
              }
              else
              {
                return cedar::proc::DataSlot::VALIDITY_ERROR;
              }
            case 2:
            {
              switch (pattern_dim)
              {
                case 1:
                  if (input.rows >= pattern.cols)
                  {
                    return cedar::proc::DataSlot::VALIDITY_VALID;
                  }
                case 2:
                  if (input.rows >= pattern.rows && input.cols >= pattern.cols)
                  {
                    return cedar::proc::DataSlot::VALIDITY_VALID;
                  }
                default:
                  return cedar::proc::DataSlot::VALIDITY_ERROR;
              }
            }
            case 1:
              // can't get here: pattern must also be 1d, but that is handled above
            default:
              return cedar::proc::DataSlot::VALIDITY_ERROR;
          }
        }
      }

      return cedar::proc::DataSlot::VALIDITY_ERROR;
    }
  }
  return cedar::proc::DataSlot::VALIDITY_ERROR;
}

void cedar::proc::steps::CrossCorrelation::inputConnectionChanged(const std::string& inputName)
{
  if (inputName == "input")
  {
    this->mInput = boost::dynamic_pointer_cast<cedar::aux::ConstMatData>(this->getInput(inputName));

    if (!this->mInput || this->mInput->isEmpty() || this->mInput->getDimensionality() > 4)
    {
      return;
    }

    if (this->mInput->getDimensionality() > 2)
    {
      this->_mAlternateKernelCenter->setConstant(false);
      if (!mConvolution)
      {
        this->createConvolutionObject();
      }
      this->_mCorrelatedDimensions->setConstant(this->mInput->getDimensionality() > 3);
    }
    else
    {
      this->_mAlternateKernelCenter->setConstant(true);
      this->_mCorrelatedDimensions->setConstant(false);
    }

    this->_mApplySpatialFiltering->setConstant(this->mInput->getDimensionality() < 3);

    // update the dimensionality (this step's dimensionality is only determined by the input, not the pattern
    this->_mCorrelatedDimensions->resize(this->mInput->getDimensionality(), false);

    this->redetermineInputValidity("pattern");
  }
  else if (inputName == "pattern")
  {
    if (auto mat_data = boost::dynamic_pointer_cast<cedar::aux::ConstMatData>(this->getInput(inputName)))
    {
      if (!mat_data->isEmpty())
      {
        this->mPattern = mat_data;
      }
    }

    this->redetermineInputValidity("input");
  }
  else
  {
    // there should not be any other inputs
    CEDAR_ASSERT(false);
  }

  if (this->mInput && this->mPattern)
  {
    this->callComputeWithoutTriggering();

    this->emitOutputPropertiesChangedSignal("crossCorrelation");
  }
}

void cedar::proc::steps::CrossCorrelation::createConvolutionObject()
{
  this->mConvolution = boost::make_shared<cedar::aux::conv::Convolution>();
  this->mConvolution->setEngine(boost::make_shared<cedar::aux::conv::FFTW>());

  this->alternateKernelCenterChanged();
}

void cedar::proc::steps::CrossCorrelation::recompute()
{
  this->onTrigger();
}

void cedar::proc::steps::CrossCorrelation::compute(const cedar::proc::Arguments&)
{
  switch (this->mInput->getDimensionality())
  {
    case 4:
      this->compute4D();
      break;

    case 3:
      this->compute3D();
      break;

    case 2:
    case 1:
      this->compute1D2D();
      break;

    default:
      CEDAR_THROW(cedar::aux::UnhandledValueException, "Unhandled matrix dimensionality in cedar::proc::steps::CrossCorrelation::compute.");
  }
}

void cedar::proc::steps::CrossCorrelation::compute4D()
{
  CEDAR_ASSERT(this->mPattern->getDimensionality() == 3);
  CEDAR_ASSERT(this->mInput->getDimensionality() == 4);

  bool normalize = this->_mNormalize->getValue();
  bool filter_spatially = this->_mApplySpatialFiltering->getValue();

  cv::Mat pattern_in = this->mPattern->getData();
  cv::Mat input = this->mInput->getData();

  CEDAR_ASSERT(pattern_in.size[0] == input.size[1]);
  CEDAR_ASSERT(pattern_in.size[1] == input.size[2]);
  CEDAR_ASSERT(pattern_in.size[2] == input.size[3]);

  CEDAR_ASSERT(pattern_in.type() == CV_32F);

  cv::Mat output = this->mCorrelation->getData();
  if (output.size[0] != input.size[0] || output.size[1] != 1)
  {
    output = cv::Mat(input.size[0], 1, CV_32F);
    this->mCorrelation->setData(output);
  }

  std::vector<cv::Range> slice_index(4, cv::Range::all());

  cv::Mat pattern, slice;

  if (normalize && !filter_spatially)
  {
    double pattern_norm = 0.0;
    pattern = normalizeMatrix(pattern_in, pattern_norm);
    this->mNormalizedPattern->setData(pattern);
  }
  else
  {
    pattern = pattern_in;
  }

  int pattern_4d_sizes[] = {1, pattern.size[0], pattern.size[1], pattern.size[2]};
  cv::Mat pattern_4d(4, pattern_4d_sizes, pattern.type(), pattern.data);
  cv::Mat pattern_spatially_normalized;
  double slice_norm;
  for (int l = 0; l < input.size[0]; ++l)
  {
    slice_index[0] = cv::Range(l, l + 1);
    cv::Mat slice_in = input(&slice_index.front());
    cv::Mat pattern_spatially_normalized;

    if (normalize)
    {
      cv::Mat mask_slice, mask_pattern;
      if (filter_spatially)
      {
        this->computeMasks(slice_in, pattern, mask_slice, mask_pattern);

        double pattern_norm;
        pattern_spatially_normalized = normalizeMatrix(pattern, pattern_norm, mask_pattern);
        pattern_4d = cv::Mat(4, pattern_4d_sizes, pattern_spatially_normalized.type(), pattern_spatially_normalized.data);
      }

      slice = normalizeMatrix(slice_in, slice_norm, mask_slice);
    }
    else
    {
      slice = slice_in;
    }

    cv::Mat product = slice.mul(pattern_4d);
    cv::Scalar sum = cv::sum(product);
    output.at<float>(l, 0) = sum[0];
  }
}

void cedar::proc::steps::CrossCorrelation::compute3D()
{
  cv::Mat input_data_in = this->mInput->getData();
  cv::Mat pattern_in;

  int dim_0, dim_1, dim_sliced;
  dim_0 = 0;
  dim_1 = 1;
  dim_sliced = 2;

  for (size_t d = 0; d < 3; ++d)
  {
    if (this->_mCorrelatedDimensions->at(d))
    {
      dim_sliced = d;
      break;
    }
  }

  switch (dim_sliced)
  {
    case 0:
      dim_0 = 1;
      dim_1 = 2;
      break;

    case 1:
      dim_0 = 0;
      dim_1 = 2;
      break;

    case 2:
      dim_0 = 0;
      dim_1 = 1;
      break;
  }

  if (cedar::aux::math::getDimensionalityOf(input_data_in) != this->mPattern->getDimensionality())
  {
    cv::Mat low_d_pattern = this->mPattern->getData();
    int size[3];
    switch (this->mPattern->getDimensionality())
    {
      case 1:
      {
        size[dim_0] = 1;
        size[dim_1] = 1;
        size[dim_sliced] = cedar::aux::math::get1DMatrixSize(low_d_pattern);
        break;
      }

      case 2:
      {
        size[dim_0] = low_d_pattern.rows;
        size[dim_1] = low_d_pattern.cols;
        size[dim_sliced] = 1;
        break;
      }

      default:
      {
        CEDAR_ASSERT(false && "This pattern dimensionality does not work with 3D cross correlation.");
      }
    }
    CEDAR_ASSERT(low_d_pattern.type() == CV_32FC1);
    cv::Mat pattern_3d(3, size, CV_32F, low_d_pattern.ptr<float>());
    pattern_in = pattern_3d;
  }
  else
  {
    pattern_in = this->mPattern->getData();
  }

  bool filter_spatially = this->_mApplySpatialFiltering->getValue();
  bool normalize = this->_mNormalize->getValue();
  cv::Mat pattern, input_data, pattern_mask, input_mask;

  if (filter_spatially)
  {
    this->computeMasks(input_data_in, pattern_in, input_mask, pattern_mask);
  }

  if (normalize)
  {
    double pattern_norm, input_data_norm;
    switch (this->mPattern->getDimensionality())
    {
      case 1:
      {
        // normalize each 1d vector in the input matrix
        std::vector<cv::Range> sub_range(3, cv::Range::all());
        this->mNormalizedInput->setData(input_data_in.clone());
        input_data = this->mNormalizedInput->getData();
        for (int d0 = 0; d0 < input_data_in.size[dim_0]; ++d0)
        {
          for (int d1 = 0; d1 < input_data_in.size[dim_1]; ++d1)
          {
            sub_range.at(dim_0) = cv::Range(d0, d0 + 1);
            sub_range.at(dim_1) = cv::Range(d1, d1 + 1);
            cv::Mat normalized = normalizeMatrix(input_data(&sub_range.front()), pattern_norm);
            normalized.copyTo(input_data(&sub_range.front()));
          }
        }
        break;
      }

      case 2:
      {
        // normalize each 2d slice in the input matrix
        std::vector<cv::Range> sub_range(3, cv::Range::all());
        this->mNormalizedInput->setData(input_data_in.clone());
        input_data = this->mNormalizedInput->getData();
        for (int d = 0; d < input_data_in.size[dim_sliced]; ++d)
        {
          sub_range.at(dim_sliced) = cv::Range(d, d + 1);
          cv::Mat normalized = normalizeMatrix(input_data(&sub_range.front()), pattern_norm);
          normalized.copyTo(input_data(&sub_range.front()));
        }
        break;
      }

      default:
        this->mNormalizedInput->setData(normalizeMatrix(input_data_in, input_data_norm, input_mask));
        input_data = this->mNormalizedInput->getData();
    }
    pattern = normalizeMatrix(pattern_in, pattern_norm, pattern_mask);
    this->mNormalizedPattern->setData(pattern.clone());
  }
  else
  {
    pattern = pattern_in;
    input_data = input_data_in;
  }

  CEDAR_DEBUG_ASSERT(cedar::aux::math::getDimensionalityOf(input_data) == 3);

  std::vector<bool> flip(3, true);
  cv::Mat flipped = cedar::aux::math::flip(pattern, flip);

  if (!this->mConvolution)
  {
    this->createConvolutionObject();
  }

  this->mFullResult->setData(this->mConvolution->convolve(input_data, flipped));
  cv::Mat& result = this->mFullResult->getData();

  cv::Mat result_low_d;
  std::vector<cv::Range> ranges(3, cv::Range::all());
  switch (this->mPattern->getDimensionality())
  {
    case 1:
    case 3:
    {
      int start = (result.size[dim_sliced] - 1) / 2;
      ranges.at(dim_sliced) = cv::Range(start, start + 1);
      result_low_d = result(&ranges.front());
      break;
    }

    case 2:
    {
      int start_0 = (result.size[dim_0] - 1) / 2;
      int start_1 = (result.size[dim_1] - 1) / 2;
      ranges.at(dim_0) = cv::Range(start_0, start_0 + 1);
      ranges.at(dim_1) = cv::Range(start_1, start_1 + 1);
      mTempResultMatrix = result(&ranges.front()).clone();
      result_low_d = cv::Mat(mTempResultMatrix.size[dim_sliced], 1, mTempResultMatrix.type(), mTempResultMatrix.data);
      break;
    }

    default:
      CEDAR_ASSERT(false && "Cannot handle this dimensionality.");
  }

  this->mCorrelation->setData(result_low_d);
}

void cedar::proc::steps::CrossCorrelation::computeMasks(const cv::Mat& input, const cv::Mat& pattern, cv::Mat& inputMask, cv::Mat& patternMask) const
{
  if (this->_mApplySpatialFiltering->getValue())
  {
    CEDAR_DEBUG_ASSERT(input.dims == 3 || (input.dims == 4 && input.size[0] == 1));
    CEDAR_DEBUG_ASSERT(input.type() == CV_32F);
    float threshold = static_cast<float>(this->_mSpatialFilteringThreshold->getValue());

    int spatial_size_rows, spatial_size_cols;
    int feature_dimension;
    if (input.dims == 4)
    {
      feature_dimension = 1;
      spatial_size_rows = input.size[2];
      spatial_size_cols = input.size[3];
    }
    else
    {
      feature_dimension = 0;
      spatial_size_rows = input.size[1];
      spatial_size_cols = input.size[2];
    }
    cv::Mat input_spatial_average(spatial_size_rows, spatial_size_cols, CV_32F, cv::Scalar(0));
    cv::Mat pattern_spatial_average(spatial_size_rows, spatial_size_cols, CV_32F, cv::Scalar(0));

    // reduce the input to its spatial dimensions
    std::vector<cv::Range> input_slice_ranges;
    input_slice_ranges.assign(static_cast<size_t>(input.dims), cv::Range::all());
    for (int i = 0; i < input.size[feature_dimension]; ++i)
    {
      input_slice_ranges[feature_dimension] = cv::Range(i, i + 1);
      cv::Mat slice = input(&input_slice_ranges.front());
      cv::Mat slice_2d(spatial_size_rows, spatial_size_cols, slice.type(), slice.data);
      input_spatial_average += slice_2d;
    }
    input_spatial_average /= static_cast<float>(input.size[feature_dimension]);
//    double min, max;
//    cv::minMaxLoc(input_spatial_average, &min, &max, nullptr, nullptr);
//    cv::imwrite("input_spatial_average.png", 255.0 * (input_spatial_average - min) / (max - min));
    cv::Mat input_thresholded = input_spatial_average > threshold;
//    cv::imwrite("input_thresholded.png", input_thresholded);

    // reduce the pattern to its spatial dimensions
    // TODO this probably doesn't have to be done all the time; make it possible to pass this as an argument
    std::vector<cv::Range> pattern_slice_ranges;
    pattern_slice_ranges.assign(static_cast<size_t>(pattern.dims), cv::Range::all());
    for (int i = 0; i < pattern.size[0]; ++i)
    {
      pattern_slice_ranges[0] = cv::Range(i, i + 1);
      cv::Mat slice = pattern(&pattern_slice_ranges.front());
      cv::Mat slice_2d(spatial_size_rows, spatial_size_cols, slice.type(), slice.data);
      pattern_spatial_average += slice_2d;
    }
//    cv::minMaxLoc(pattern_spatial_average, &min, &max, nullptr, nullptr);
//    cv::imwrite("pattern_spatial_average.png", 255.0 * (pattern_spatial_average - min) / (max - min));
    pattern_spatial_average /= static_cast<float>(pattern.size[0]);
    cv::Mat pattern_thresholded = pattern_spatial_average > threshold;
//    cv::imwrite("pattern_thresholded.png", pattern_thresholded);

    // combine the two masks to a single one
    cv::Mat spatial_mask = input_thresholded | pattern_thresholded;
//    cv::imwrite("spatial_mask.png", spatial_mask);

    // repeat the masks to make proper masks for the input and pattern matrices
    CEDAR_DEBUG_ASSERT(pattern.size[0] == input.size[feature_dimension]);
    inputMask.create(input.dims, input.size, spatial_mask.type());
    patternMask.create(pattern.dims, pattern.size, spatial_mask.type());
    for (int i = 0; i < pattern.size[0]; ++i)
    {
      input_slice_ranges[feature_dimension] = cv::Range(i, i + 1);
      pattern_slice_ranges[0] = cv::Range(i, i + 1);

      cv::Mat input_slice = inputMask(&input_slice_ranges.front());
      cv::Mat input_slice_2d(spatial_size_rows, spatial_size_cols, input_slice.type(), input_slice.data);
      spatial_mask.copyTo(input_slice_2d);

      cv::Mat pattern_slice = patternMask(&pattern_slice_ranges.front());
      cv::Mat pattern_slice_2d(spatial_size_rows, spatial_size_cols, pattern_slice.type(), pattern_slice.data);
      spatial_mask.copyTo(pattern_slice_2d);
    }
  }
}

cv::Mat cedar::proc::steps::CrossCorrelation::normalizeMatrix(const cv::Mat& in, double& norm, cv::Mat mask)
{
  double mean = cv::mean(in, mask).val[0];
  cv::Mat mean_free = in - mean;
  if (!mask.empty())
  {
    cv::Mat inv_mask;
    cv::bitwise_not(mask, inv_mask);
    mean_free.setTo(cv::Scalar(0), inv_mask);
  }
  norm = cv::norm(mean_free, cv::NORM_L2, mask);

  if (static_cast<float>(norm) < std::numeric_limits<float>::epsilon()) // casting to float because the matrices involved are float matrices
  {
    return 0.0 * in;
  }
  else
  {
    return mean_free / norm;
  }
}

void cedar::proc::steps::CrossCorrelation::compute1D2D()
{
  const cv::Mat& input_data = this->mInput->getData();
  const cv::Mat& pattern = this->mPattern->getData();
  cv::Mat pattern_buffer;
  int top = 0;
  int bottom = 0;
  int left = 0;
  int right = 0;
  pattern_buffer = pattern;
  cv::Mat input;

  int method = cv::TM_CCORR;
  if (this->_mNormalize->getValue())
  {
    method = cv::TM_CCORR_NORMED;
  }

  unsigned int dim = cedar::aux::math::getDimensionalityOf(input_data);
  unsigned int pattern_dim = cedar::aux::math::getDimensionalityOf(pattern);

  if (this->padInput())
  {
    switch (dim)
    {
      case 2:
        if (this->correlateAlong(0))
        {
          if (pattern_dim==1)
          {
            pattern_buffer = pattern.t();
          }
          left = right = std::max(0, pattern_buffer.cols/2);
        }
        if (this->correlateAlong(1))
        {
           top = bottom = std::max(0, pattern_buffer.rows/2);
        }
        break;
      case 1:
        top = bottom = std::max(0, pattern_buffer.rows/2 );
        break;
      default:
        CEDAR_THROW(cedar::aux::UnhandledValueException, "Unhandled matrix dimensionality in cedar::proc::steps::CrossCorrelation::compute1D2D.");
    }
    cv::copyMakeBorder(input_data, input, top, bottom, left, right, this->getCvBorderType(), cv::Scalar(0));
  }
  else
  {
    input = input_data;
  }

  cv::Mat& cross_correlation = this->mCorrelation->getData();
  if (pattern_dim != dim - 1)
  {
    cv::matchTemplate(input, pattern_buffer, cross_correlation, method);
  }
  else
  {
    int correlated_dim = 0;
    int other_dim = 1;
    if (this->correlateAlong(1))
    {
      correlated_dim = 1;
      other_dim = 0;
    }

    if (this->_mNormalize->getValue())
    {
      double norm;
      this->mNormalizedPattern->setData(this->normalizeMatrix(pattern_buffer, norm));
      pattern_buffer = this->mNormalizedPattern->getData();
    }

    // since we correlate along one dimension, the other must have the same size as the pattern we are matching
    CEDAR_ASSERT(input.size[other_dim] == static_cast<int>(cedar::aux::math::get1DMatrixSize(pattern_buffer)));

    if (cross_correlation.rows != input.size[correlated_dim] || cross_correlation.cols != 1)
    {
      cross_correlation = cv::Mat::zeros(input.size[correlated_dim], 1, CV_32F);
    }

    for (int slice_index = 0; slice_index < input.size[correlated_dim]; ++slice_index)
    {
      cv::Range row_range = cv::Range::all();
      cv::Range col_range = cv::Range::all();

      if (correlated_dim == 0)
      {
        row_range.start = slice_index;
        row_range.end = slice_index + 1;
      }
      else
      {
        col_range.start = slice_index;
        col_range.end = slice_index + 1;
      }

      cv::Mat slice = input(row_range, col_range);
      if (this->_mNormalize->getValue())
      {
        double norm;
        slice = this->normalizeMatrix(slice, norm);
      }

      // make sure both 1d-vectors are oriented in the same way
      if ((pattern_buffer.rows == 1 && slice.rows != 1) || (pattern_buffer.cols == 1 && slice.cols != 1))
      {
        slice = slice.t();
      }

      cv::Mat product = pattern_buffer.mul(slice);
      auto correlation = cv::sum(product)[0];
      cross_correlation.at<float>(slice_index, 0) = static_cast<float>(correlation);
    }
  }

  if (this->_mPadInput->getValue() && this->_mUnPadOutput->getValue())
  {
    CEDAR_DEBUG_ASSERT(cedar::aux::math::getDimensionalityOf(cross_correlation) <= 2);
    cv::Range row_range;
    row_range.start = top;
    row_range.end = cross_correlation.rows - bottom;
    cv::Range col_range;
    if (cedar::aux::math::getDimensionalityOf(cross_correlation) == 2)
    {
      col_range.start = left;
      col_range.end = std::max(cross_correlation.cols - right, left + 1);
    }
    else
    {
      col_range = cv::Range::all();
    }
    cross_correlation = cross_correlation(row_range, col_range);
  }
}


