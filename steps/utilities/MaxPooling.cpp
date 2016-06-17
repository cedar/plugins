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

    File:        MaxPooling.cpp

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2011 10 24

    Description:

    Credits:

======================================================================================================================*/

// LOCAL INCLUDES
#include "steps/utilities/MaxPooling.h"

// PROJECT INCLUDES

// SYSTEM INCLUDES
#include <cedar/processing/typecheck/Matrix.h>
#include <cedar/auxiliaries/assert.h>
#include <cedar/auxiliaries/exceptions.h>

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::proc::steps::MaxPooling::MaxPooling()
:
mOutput(new cedar::aux::MatData(cv::Mat())),
mPoolSize(new cedar::aux::UIntParameter(this, "poolSizes", 4, 2, 100000)),
_mPooledDimensions(new cedar::aux::BoolVectorParameter(this, "pooled dimensions", 0, true))
{
  auto input_slot = this->declareInput("input");
  cedar::proc::typecheck::Matrix input_check;
  input_check.addAcceptedDimensionalityRange(1, 3);
  input_slot->setCheck(input_check);

  this->declareOutput("output", mOutput);

  QObject::connect(this->mPoolSize.get(), SIGNAL(valueChanged()), this, SLOT(poolSizeChanged()));
  QObject::connect(this->_mPooledDimensions.get(), SIGNAL(valueChanged()), this, SLOT(poolSizeChanged()));
  this->updateInternals();
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

void cedar::proc::steps::MaxPooling::updateInternals()
{
  if (!this->mInput)
  {
    this->mOutput->setData(cv::Mat());
    return;
  }

  int pool_size = static_cast<int>(this->mPoolSize->getValue());

  cv::Mat input = this->mInput->getData();

  if (input.empty())
  {
    return;
  }

  std::vector<int> dest_size(static_cast<size_t>(input.dims));
  for (int d = 0; d < input.dims; ++d)
  {
    if (input.size[d] != 1)
    {
      if (this->_mPooledDimensions->at(d))
      {
        dest_size[static_cast<size_t>(d)] = input.size[d] / pool_size;
      }
      else
      {
        dest_size[static_cast<size_t>(d)] = input.size[d];
      }
    }
    else
    {
      dest_size[static_cast<size_t>(d)] = 1;
    }
  }

  this->mOutput->setData(cv::Mat(input.dims, &dest_size.front(), CV_32F, cv::Scalar(0.0)));
  this->mRanges.resize(this->mInput->getDimensionality());
  this->mIndices.resize(this->mInput->getDimensionality());

  if (this->mInput->getDimensionality() == 1)
  {
    this->mRanges.push_back(cv::Range::all());
    this->mIndices.push_back(0);
  }
}

void cedar::proc::steps::MaxPooling::poolSizeChanged()
{
  this->updateInternals();

  this->onTrigger();
}

void cedar::proc::steps::MaxPooling::inputConnectionChanged(const std::string& inputName)
{
  QReadLocker l(&this->mOutput->getLock());
  cv::Mat old_output = this->mOutput->getData().clone();
  l.unlock();

  cv::Mat mat;
  if (inputName == "input")
  {
    // need to block the signals so onTrigger doesn't get called (which should not happen in the inputConnectionChanged function)
    bool blocked = this->_mPooledDimensions->blockSignals(true);

    this->mInput = boost::dynamic_pointer_cast<const cedar::aux::MatData>(this->getInput(inputName));
    if (this->mInput)
    {
      this->mOutput->copyAnnotationsFrom(this->mInput);
      this->_mPooledDimensions->resize(this->mInput->getDimensionality());
    }
    this->_mPooledDimensions->setConstant(!this->mInput || this->mInput->getDimensionality() <= 2);

    this->_mPooledDimensions->blockSignals(blocked);
  }

  CEDAR_ASSERT(mat.channels() == 1);
  this->updateInternals();

  l.relock();
  const cv::Mat& output = this->mOutput->getData();
  bool changed = old_output.type() != output.type() || old_output.size != output.size;
  l.unlock();

  if (changed)
  {
    this->emitOutputPropertiesChangedSignal("output");
  }
}

void cedar::proc::steps::MaxPooling::compute(const cedar::proc::Arguments&)
{
  const cv::Mat& input = mInput->getData();
  cv::Mat& output = mOutput->getData();
  int pool_size = static_cast<int>(this->mPoolSize->getValue());

  auto dimensionality = this->mInput->getDimensionality();

  for (size_t d = 0; d < dimensionality; ++d)
  {
    mIndices.at(d) = 0;
  }

  int max_index;
  if (this->mInput->getDimensionality() == 1)
  {
    max_index = cedar::aux::math::get1DMatrixSize(output);
  }
  else
  {
    max_index = output.size[input.dims - 1];
  }

  while (mIndices.at(dimensionality - 1) < max_index)
  {
    for (size_t d = 0; d < dimensionality; ++d)
    {
      if (this->_mPooledDimensions->at(d))
      {
        mRanges.at(d).start = mIndices.at(d) * pool_size;
        mRanges.at(d).end = mRanges.at(d).start + pool_size;
      }
      else
      {
        mRanges.at(d).start = mIndices.at(d);
        mRanges.at(d).end = mIndices.at(d) + 1;
      }
      CEDAR_DEBUG_ASSERT(this->mRanges.at(d).start >= 0);
      CEDAR_DEBUG_ASSERT(this->mRanges.at(d).start < input.size[d]);
      CEDAR_DEBUG_ASSERT(this->mRanges.at(d).end >= 0);
      CEDAR_DEBUG_ASSERT(this->mRanges.at(d).end <= input.size[d]);
      CEDAR_DEBUG_ASSERT(this->mRanges.at(d).start < this->mRanges.at(d).end);
    }

    double max = -std::numeric_limits<double>::max();
    switch (input.dims)
    {
      // opencv doesn't do minmaxloc on dims > 2 ...
      default:
      {
        CEDAR_ASSERT(input.type() == CV_32F);
        cv::Mat slice = input(&mRanges.front());
        cv::MatConstIterator_<float> it = slice.begin<float>(), it_end = slice.end<float>();
        for(; it != it_end; ++it)
        {
          max = std::max(static_cast<double>(*it), max);
        }
        break;
      }

      case 1:
      case 2:
        CEDAR_DEBUG_ASSERT(mRanges.size() == static_cast<size_t>(input.dims));
        cv::minMaxLoc(input(&mRanges.front()), NULL, &max);
        break;
    }
    output.at<float>(&mIndices.front()) = max;

    // increment the last index
    mIndices.front() += 1;

    // overflow to the next dimension
    for (size_t d = 0; d < dimensionality - 1; ++d) // the last index doesn't overflow
    {
      if (mIndices.at(d) >= output.size[d])
      {
        mIndices.at(d) = 0;
        mIndices.at(d + 1) += 1;
      }
      else
      {
        break;
      }
    }
  }
}
