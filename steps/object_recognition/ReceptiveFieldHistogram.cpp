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

    File:        ReceptiveFieldHistogram.cpp

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2013 12 11

    Description: Source file for the class cedar::proc::steps::ReceptiveFieldHistogram.

    Credits:

======================================================================================================================*/

// CEDAR CONFIGURATION
#include "cedar/configuration.h"

// LOCAL INCLUDES
#include "steps/object_recognition/ReceptiveFieldHistogram.h"

// CEDAR INCLUDES
#include <cedar/processing/typecheck/And.h>
#include <cedar/processing/typecheck/Matrix.h>
#include <cedar/processing/typecheck/SameSize.h>
#include <cedar/auxiliaries/MatData.h>

// SYSTEM INCLUDES
#include <boost/make_shared.hpp>
#include <limits.h>
#include <iostream>

//#define DEBUG_OUTPUTS

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::proc::steps::ReceptiveFieldHistogram::ReceptiveFieldHistogram()
:
mBinImage(new cedar::aux::MatData(cv::Mat())),
mOutput(new cedar::aux::MatData(cv::Mat())),
_mFeatureBins(new cedar::aux::UIntParameter(this, "feature bins", 10)),
_mNumberOfReceptiveFields(new cedar::aux::UIntVectorParameter(this, "receptive field division", 2, 1)),
_mValueRangeLower(new cedar::aux::DoubleParameter(this, "lowest value", 0.0)),
_mValueRangeUpper(new cedar::aux::DoubleParameter(this, "highest value", 1.0)),
_mCyclicBins(new cedar::aux::BoolParameter(this, "cyclic bins", true))
{
  // input declarations

  auto image_slot = this->declareInput("image");
  auto mask_slot = this->declareInput("mask", false);

  cedar::proc::typecheck::And check;

  cedar::proc::typecheck::Matrix matrix_check;
  matrix_check.addAcceptedDimensionality(2);
  check.addCheck(matrix_check);

  cedar::proc::typecheck::SameSize size_check;
  size_check.addSlot(image_slot);
  size_check.addSlot(mask_slot);
  check.addCheck(size_check);

  image_slot->setCheck(check);
  mask_slot->setCheck(check);

  auto rf_slot = this->declareInput("receptive field weights", false);
  rf_slot->setCheck(matrix_check);

  // buffer declarations
  this->declareBuffer("bin image", this->mBinImage);

  // output declarations
  this->declareOutput("output", this->mOutput);

  // signal/slot connections
  this->connect(this->_mFeatureBins.get(), SIGNAL(valueChanged()), SLOT(numberOfBinsChanged()), Qt::DirectConnection);
  this->connect(this->_mValueRangeLower.get(), SIGNAL(valueChanged()), SLOT(rangeChanged()), Qt::DirectConnection);
  this->connect(this->_mValueRangeUpper.get(), SIGNAL(valueChanged()), SLOT(rangeChanged()), Qt::DirectConnection);
  this->connect(this->_mNumberOfReceptiveFields.get(), SIGNAL(valueChanged()), SLOT(numberOfRFsChanged()), Qt::DirectConnection);
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

void cedar::proc::steps::ReceptiveFieldHistogram::numberOfBinsChanged()
{
  this->refreshLookupTable();
  this->resizeOutput();
  this->recompute();
}

void cedar::proc::steps::ReceptiveFieldHistogram::rangeChanged()
{
  this->refreshLookupTable();
  this->recompute();
}

void cedar::proc::steps::ReceptiveFieldHistogram::numberOfRFsChanged()
{
  this->resizeOutput();
  this->recompute();
}

void cedar::proc::steps::ReceptiveFieldHistogram::calulateWeightedHistogram
     (
       const cv::Mat& binImage,
       const cv::Mat& weights,
       const cv::Mat& mask,
       cv::Mat histogram,
       int* center, // 2d array {row, col}
       int* weight_size // {rows, cols}; ignored if weights is not empty
     )
{
  int start[2];
  int end[2];
  int hist_index[3];

  if (weights.empty())
  {
    for (size_t i = 0; i < 2; ++i)
    {
      start[i] = std::max(0, center[i] - weight_size[i]/2);
      end[i]   = std::min(center[i] + weight_size[i]/2, binImage.size[i]);
    }
  }
  else
  {
    for (size_t i = 0; i < 2; ++i)
    {
      start[i] = std::max(0, center[i] - weights.size[i]/2);
      end[i]   = std::min(center[i] + weights.size[i]/2, binImage.size[i]);
    }
  }

  CEDAR_ASSERT(weights.empty() || weights.type() == CV_32F);
  CEDAR_ASSERT(mask.empty() || mask.type() == CV_32F);
  CEDAR_ASSERT(histogram.type() == CV_32F);

  for (int row = start[0]; row < end[0]; ++row)
  {
    for (int col = start[1]; col < end[1]; ++col)
    {
      float weight = 1.0;

      if (!weights.empty())
      {
        int rf_matrix_row = center[0] - row + weights.size[0]/2;
        int rf_matrix_col = center[1] - col + weights.size[1]/2;
        CEDAR_DEBUG_ASSERT(rf_matrix_row < weights.rows);
        CEDAR_DEBUG_ASSERT(rf_matrix_col < weights.cols);
        CEDAR_DEBUG_ASSERT(rf_matrix_row >= 0);
        CEDAR_DEBUG_ASSERT(rf_matrix_col >= 0);
        weight = weights.at<float>(rf_matrix_row, rf_matrix_col);
      }

      if (!mask.empty())
      {
        CEDAR_DEBUG_ASSERT(row < mask.rows);
        CEDAR_DEBUG_ASSERT(col < mask.cols);
        weight *= mask.at<float>(row, col);
      }

      int bin = static_cast<int>(binImage.at<short>(row, col));
      CEDAR_DEBUG_ASSERT(bin >= 0);
      CEDAR_DEBUG_ASSERT(bin < histogram.size[2]);

      hist_index[0] = 0;
      hist_index[1] = 0;
      hist_index[2] = bin;

      histogram.at<float>(hist_index) += weight;
    }
  }
}

template <typename InputType>
void cedar::proc::steps::ReceptiveFieldHistogram::updateBinImage
     (
        const cv::Mat& input,
        cv::Mat& output,
        int bins,
        float lower,
        float upper
     )
{
  CEDAR_ASSERT(output.type() == CV_16SC1);
  CEDAR_ASSERT(output.size == input.size);

  if (typeid(InputType) == typeid(unsigned char) && !this->mBinLookupTable.empty())
  {
    // use lookup table
    for (int row = 0; row < input.rows; ++row)
    {
      for (int col = 0; col < input.cols; ++col)
      {
        auto value = input.at<InputType>(row, col);
        output.at<short>(row, col) = this->mBinLookupTable[value];
      }
    }
  }
  else
  {
    // calculate bin image for each entry in the image
    for (int row = 0; row < input.rows; ++row)
    {
      for (int col = 0; col < input.cols; ++col)
      {
        auto value = static_cast<float>(input.at<InputType>(row, col));

        // determine percentage of feature value along interval
        float scaled = (value - lower) / (upper - lower);

        // clamp the value to the range [0, 1]
        scaled = std::min(1.0f, std::max(0.0f, scaled));

        // determine the bin
        short bin = static_cast<short>(scaled * static_cast<float>(bins));
        if (this->_mCyclicBins->getValue())
        {
          bin %= bins;
        }
        CEDAR_DEBUG_ASSERT(bin >= 0);
        CEDAR_DEBUG_ASSERT(bin < bins);

        output.at<short>(row, col) = bin;
      }
    }
  }
}

void cedar::proc::steps::ReceptiveFieldHistogram::compute(const cedar::proc::Arguments&)
{
  const cv::Mat& input = this->mInput->getData();
  cv::Mat& output = this->mOutput->getData();
  cv::Mat& bin_image = this->mBinImage->getData();

  // reset all histogram bins
  output.setTo(0.0);

  float lower = static_cast<float>(this->_mValueRangeLower->getValue());
  float upper = static_cast<float>(this->_mValueRangeUpper->getValue());

  // offset for each receptive field (by how much is it shifted based on the index)
  int rf_offset[2];

  // number of receptive fields in each direction
  int num_rfs[2];

  cv::Mat rf;
  if (this->mReceptiveField)
  {
    rf = this->mReceptiveField->getData();
  }

  for (int i = 0; i < 2; ++i)
  {
    num_rfs[i] = static_cast<int>(this->_mNumberOfReceptiveFields->at(i));
    rf_offset[i] = input.rows / num_rfs[i];
  }

  cv::Mat mask;
  if (this->mMask)
  {
    mask = this->mMask->getData();
  }

  int hist_bins = static_cast<int>(this->getNumberOfFeatureBins());
  int hist_size[] = {1, 1, hist_bins};
  cv::Mat histogram = 0.0 * cv::Mat(3, hist_size, CV_32F);
  int rf_idx[2];
  int rf_center[2];
  cv::Range hist_dst[3];
  hist_dst[2] = cv::Range::all();

  switch (input.type())
  {
    case CV_8U:
      updateBinImage<unsigned char>
      (
        input,
        bin_image,
        hist_bins,
        lower,
        upper
      );
      break;

    case CV_32F:
      updateBinImage<float>
      (
        input,
        bin_image,
        hist_bins,
        lower,
        upper
      );
      break;

    default:
      CEDAR_ASSERT(false && "Not implemented for this input type.");
  }

  for (rf_idx[0] = 0; rf_idx[0] < num_rfs[0]; ++rf_idx[0])
  {
    rf_center[0] = rf_idx[0] * rf_offset[0] + rf_offset[0]/2;
    hist_dst[0] = cv::Range(rf_idx[0], rf_idx[0] + 1);

    for (rf_idx[1] = 0; rf_idx[1] < num_rfs[1]; ++rf_idx[1])
    {
      rf_center[1] = rf_idx[1] * rf_offset[1] + rf_offset[1]/2;
      hist_dst[1] = cv::Range(rf_idx[1], rf_idx[1] + 1);

      calulateWeightedHistogram
      (
        bin_image,
        rf,
        mask,
        output(hist_dst),
        rf_center,
        rf_offset
      );
    }
  }
}

void cedar::proc::steps::ReceptiveFieldHistogram::refreshLookupTable()
{
  this->mBinLookupTable.clear();

  float lower = static_cast<float>(this->_mValueRangeLower->getValue());
  float upper = static_cast<float>(this->_mValueRangeUpper->getValue());

  short bins = static_cast<short>(this->getNumberOfFeatureBins());

  if (this->mInput && this->mInput->getData().type() == CV_8U)
  {
    this->mBinLookupTable.resize(256);

    for (short value_s = 0; value_s < static_cast<short>(this->mBinLookupTable.size()); ++value_s)
    {
      auto value = static_cast<float>(value_s);

      // determine percentage of feature value along interval
      float scaled = (value - lower) / (upper - lower);

      // clamp the value to the range [0, 1]
      scaled = std::min(1.0f, std::max(0.0f, scaled));

      // determine the bin
      short bin = static_cast<short>(scaled * static_cast<float>(bins));
      if (this->_mCyclicBins->getValue())
      {
        bin %= bins;
      }
      CEDAR_DEBUG_ASSERT(bin >= 0);
      CEDAR_DEBUG_ASSERT(bin < bins);

      this->mBinLookupTable[value_s] = bin;
    }
  }
}

void cedar::proc::steps::ReceptiveFieldHistogram::inputConnectionChanged(const std::string& inputName)
{
  auto slot = this->getInputSlot(inputName);
  auto data = slot->getData();
  if (inputName == "image")
  {
    this->mInput = boost::dynamic_pointer_cast<cedar::aux::ConstMatData>(data);

    if (this->mInput && this->mInput->getDimensionality() == 2)
    {
      this->_mNumberOfReceptiveFields->resize(this->mInput->getDimensionality());
    }
    this->resizeOutput();

    this->redetermineInputValidity("receptive field weights");
  }
  else if (inputName == "receptive field weights")
  {
    this->mReceptiveField = boost::dynamic_pointer_cast<cedar::aux::ConstMatData>(data);
  }
  else if (inputName == "mask")
  {
    mMask = boost::dynamic_pointer_cast<cedar::aux::ConstMatData>(data);
    this->redetermineInputValidity("image");
  }
  else
  {
    CEDAR_NON_CRITICAL_ASSERT(false && "Unknown input name.");
  }
}

void cedar::proc::steps::ReceptiveFieldHistogram::resizeOutput()
{
  unsigned int input_dim = 0;
  int input_rows = 0;
  int input_cols = 0;
  if (this->mInput)
  {
    QReadLocker input_l(&this->mInput->getLock());
    input_dim = this->mInput->getDimensionality();
    input_rows = this->mInput->getData().rows;
    input_cols = this->mInput->getData().cols;
    input_l.unlock();
  }

  bool changed = false;
  if (this->mInput && input_dim == 2)
  {
    cedar::proc::Step::ReadLocker l(this);
    int sizes [3];
    sizes[0] = static_cast<int>(this->_mNumberOfReceptiveFields->at(0));
    sizes[1] = static_cast<int>(this->_mNumberOfReceptiveFields->at(1));
    sizes[2] = static_cast<int>(this->getNumberOfFeatureBins());

    cv::Mat new_output = cv::Mat(3, sizes, CV_32F, 0.0);
    changed = new_output.type() != this->mOutput->getData().type() || new_output.size != this->mOutput->getData().size;
    this->mOutput->setData(new_output);
    this->mBinImage->setData(cv::Mat(input_rows, input_cols, CV_16SC1, 0.0));
    l.unlock();
  }
  else
  {
    cedar::proc::Step::ReadLocker l(this);
    changed = !this->mOutput->getData().empty();
    this->mOutput->setData(cv::Mat());
    this->mBinImage->setData(cv::Mat());
    l.unlock();
  }

  if (changed)
  {
    this->emitOutputPropertiesChangedSignal("output");
  }
}

void cedar::proc::steps::ReceptiveFieldHistogram::recompute()
{
  this->onTrigger();
}

unsigned int cedar::proc::steps::ReceptiveFieldHistogram::getNumberOfFeatureBins() const
{
  return this->_mFeatureBins->getValue();
}
