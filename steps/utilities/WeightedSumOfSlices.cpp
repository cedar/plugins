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

    File:        WeightedSumOfSlices.cpp

    Maintainer:  christian faubel
    Email:       christian.faubel@ini.ruhr-uni-bochum.de
    Date:        2012 01 09

    Description:

    Credits:

======================================================================================================================*/

// LOCAL INCLUDES
#include "steps/utilities/WeightedSumOfSlices.h"

// PROJECT INCLUDES

// CEDAR INCLUDES
#include <cedar/processing/Arguments.h>
#include <cedar/auxiliaries/math/tools.h>
#include <cedar/auxiliaries/MatData.h>
//#include <cedar/auxiliaries/assert.h>

// SYSTEM INCLUDES
#include <vector>

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::proc::steps::WeightedSumOfSlices::WeightedSumOfSlices()
:
mOutput( new cedar::aux::MatData(cv::Mat::zeros(1, 1, CV_32F)) ),
_mWeightedDimension(new cedar::aux::UIntParameter(this, "dropped dimension", 0))
{
  // declare input
  this->declareInput("input");
  this->declareInput("weight vector");

  // declare output
  this->declareOutput("weighted sum", mOutput);

  // parameter connections
  this->connect(this->_mWeightedDimension.get(), SIGNAL(valueChanged()), SLOT(allocateOutput()));
}

cedar::proc::steps::WeightedSumOfSlices::~WeightedSumOfSlices()
{
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

cedar::proc::DataSlot::VALIDITY cedar::proc::steps::WeightedSumOfSlices::determineInputValidity
                                                            (
                                                              cedar::proc::ConstDataSlotPtr slot,
                                                              cedar::aux::ConstDataPtr data
                                                            ) const
{
  // check whether data is MatData
  if(cedar::aux::ConstMatDataPtr mat_data = boost::dynamic_pointer_cast<cedar::aux::ConstMatData>(data))
  {
    if (slot->getName() == "input")
    {
      // check whether input has a dimensionality between 2 and 4
      if (mat_data->getDimensionality() < 2 || mat_data->getDimensionality() > 4)
      {
        return cedar::proc::DataSlot::VALIDITY_ERROR;
      }

      // check if the sizes of matrix and weight vector fit
      if (this->mWeightVector && !this->checkIfMaticesFit(mat_data->getData(), this->mWeightVector->getData()))
      {
        return cedar::proc::DataSlot::VALIDITY_ERROR;
      }
    }

    if (slot->getName() == "weight vector")
    {
      // check whether weightVector has a dimensionality of 1
      if (mat_data->getDimensionality() != 1)
      {
        return cedar::proc::DataSlot::VALIDITY_ERROR;
      }

      // check if the sizes of matrix and weight vector fit
      if (this->mInput && !this->checkIfMaticesFit(this->mInput->getData(), mat_data->getData()))
      {
        return cedar::proc::DataSlot::VALIDITY_ERROR;
      }
    }

    //input is valid
    return cedar::proc::DataSlot::VALIDITY_VALID;
  }

  return cedar::proc::DataSlot::VALIDITY_ERROR;
}

bool cedar::proc::steps::WeightedSumOfSlices::checkIfMaticesFit(const cv::Mat& input, const cv::Mat& weights) const
{
  if (cedar::aux::math::getDimensionalityOf(weights) != 1)
  {
    return false;
  }

  switch (cedar::aux::math::getDimensionalityOf(input))
  {
    case 2:
      if (static_cast<unsigned int>(input.cols) != cedar::aux::math::get1DMatrixSize(weights))
      {
        return false;
      }
      break;

    default:
    case 3:
      if (static_cast<unsigned int>(input.size[this->_mWeightedDimension->getValue()]) != cedar::aux::math::get1DMatrixSize(weights))
      {
        return false;
      }
      break;
  }

  return true;
}

void cedar::proc::steps::WeightedSumOfSlices::inputConnectionChanged(const std::string& inputName)
{
  // set input and output data, trigger afterwards
  if (inputName == "input")
  {
   this->mInput = boost::dynamic_pointer_cast<cedar::aux::ConstMatData>(this->getInput(inputName));

   if (!this->mInput || this->mInput->isEmpty())
   {
     return;
   }

   this->allocateOutput();

   this->_mWeightedDimension->setConstant(this->mInput->getDimensionality() != 4);

   this->redetermineInputValidity("weight vector");

   if (this->allInputsValid())
   {
     cedar::proc::Step::ReadLocker locker(this);
     this->compute(cedar::proc::Arguments());
     locker.unlock();
   }

   this->emitOutputPropertiesChangedSignal("weighted sum");
  }

  if (inputName == "weight vector")
  {
    this->mWeightVector = boost::dynamic_pointer_cast<cedar::aux::ConstMatData>(this->getInput(inputName));

    this->redetermineInputValidity("input");

//    this->onTrigger();
  }
}

void cedar::proc::steps::WeightedSumOfSlices::allocateOutput()
{
  if (!this->mInput || this->mInput->isEmpty())
  {
    return;
  }

  switch (this->mInput->getDimensionality())
  {
    case 2:
      this->mOutput->setData(cv::Mat::zeros(this->mInput->getData().rows, 1, this->mInput->getCvType()));
      break;

    case 3:
      this->mOutput->getData() = cv::Mat::zeros(this->mInput->getData().size[1], this->mInput->getData().size[2], this->mInput->getData().type());
      break;

    default:
    {
      std::vector<int> sizes;
      for (int d = 0; d < static_cast<int>(this->mInput->getDimensionality()); ++d)
      {
        if (static_cast<unsigned int>(d) != this->_mWeightedDimension->getValue())
          sizes.push_back(this->mInput->getData().size[d]);
      }
      this->mOutput->getData() = 0.0 * cv::Mat(static_cast<int>(sizes.size()), &sizes.front(), this->mInput->getData().type());
    }
  }
}

void cedar::proc::steps::WeightedSumOfSlices::compute(const cedar::proc::Arguments&)
{
  // input
  const cv::Mat& input = this->mInput->getData();
  const cv::Mat& weight_vector = this->mWeightVector->getData();

  // output, set to zero
  cv::Mat& output = this->mOutput->getData();
  output.setTo(0.0);

  int dropped_dimension = this->_mWeightedDimension->getValue();

  // loop over slices
  switch (this->mInput->getDimensionality())
  {
    case 2:
    {
      for (int r = 0; r < input.cols; r++)
      {
        // add up all rows
        output += input.col(r) * weight_vector.at<float>(r);
      }
      break;
    }

    case 3:
      for ( int i=0; i<input.size[0]; i++ )
      {
        // input slice
        void* ptr_in = input.data +  input.step[0]*i;
        cv::Mat slice_in = cv::Mat( input.size[1], input.size[2], input.type(), ptr_in);

        // add the slices to the output
        output += slice_in * weight_vector.at<float>(i);
      }
      break;

    default:
    {
//      std::vector<int> sizes;
//      for (int d = 1; d < static_cast<int>(this->mInput->getDimensionality()); ++d)
//      {
//        sizes.push_back(input.size[d]);
//      }
      int out_dim = static_cast<int>(this->mInput->getDimensionality() - 1);

      CEDAR_ASSERT(cedar::aux::math::getDimensionalityOf(weight_vector) == 1);
      CEDAR_ASSERT(static_cast<size_t>(input.size[dropped_dimension]) <= cedar::aux::math::get1DMatrixSize(weight_vector));
      switch (dropped_dimension)
      {
        case 0:
          for (int i = 0; i < input.size[dropped_dimension]; i++)
          {
            // input slice
            void* ptr_in = input.data +  input.step[0]*i;
            cv::Mat slice_in = cv::Mat(out_dim, output.size, input.type(), ptr_in);

            // add the slices to the output
            output += slice_in * weight_vector.at<float>(i);
          }
          break;

        // generic implementation
        default:
        {
          std::vector<cv::Range> ranges(this->mInput->getDimensionality(), cv::Range::all());

          for (int i = 0; i < input.size[dropped_dimension]; ++i)
          {
            ranges[dropped_dimension].start = i;
            ranges[dropped_dimension].end = i + 1;
            cv::Mat slice = input(&ranges.front()) * weight_vector.at<float>(i);
            cv::Mat slice_reduced(output.dims, output.size, output.type(), slice.data);
            output += slice_reduced;
          }
          break;
        }
      }
    }
  }
}
