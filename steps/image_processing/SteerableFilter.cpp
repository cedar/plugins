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

    File:        SteeringFilter.cpp

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2011 10 18

    Description:

    Credits:

======================================================================================================================*/

// LOCAL INCLUDES
#include "steps/image_processing/SteerableFilter.h"
#include "kernels/SteerableKernel.h"

// SYSTEM INCLUDES
#include <cedar/processing/typecheck/Matrix.h>
#include <cedar/processing/Arguments.h>
#include <cedar/auxiliaries/assert.h>
#include <cedar/auxiliaries/exceptions.h>

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::proc::steps::SteerableFilter::SteerableFilter()
:
mFilteredImage(new cedar::aux::MatData(cv::Mat())),
mSteeringKernel(new cedar::aux::kernel::SteerableKernel()),
mConvolution(new cedar::aux::conv::Convolution())
{
  // configuration
  this->addConfigurableChild("kernel", this->mSteeringKernel);
  this->addConfigurableChild("convolution", this->mConvolution);
  // input
  auto input_slot = this->declareInput("input");
  cedar::proc::typecheck::Matrix input_check;
  input_check.addAcceptedDimensionality(2);
  input_slot->setCheck(input_check);

  // buffers
  this->declareBuffer("kernel", mSteeringKernel->getKernelRaw());

  // output
  this->declareOutput("filteredInput", mFilteredImage);

  this->mConvolution->getKernelList()->append(this->mSteeringKernel);

  QObject::connect(mSteeringKernel.get(), SIGNAL(kernelUpdated()), this, SLOT(recompute()));
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

void cedar::proc::steps::SteerableFilter::inputConnectionChanged(const std::string& inputName)
{
  if (inputName == "input")
  {
    // if we don't have a nullptr, but get one afterwards, a change occurred
    bool changed = this->mImage.get() != nullptr;

    this->mImage = boost::dynamic_pointer_cast<const cedar::aux::MatData>(this->getInput("input"));

    if (this->mImage)
    {
      QReadLocker input_l(&this->mImage->getLock());
      const cv::Mat& input = this->mImage->getData();
      int input_rows = input.rows;
      int input_cols = input.cols;
      int input_type = input.type();
      int input_channels = input.channels();
      input_l.unlock();

      QWriteLocker output_l(&this->mFilteredImage->getLock());
      // if we didn't get a nullptr, check for change
      changed = input_rows != this->mFilteredImage->getData().rows || input_cols != this->mFilteredImage->getData().cols
                || input_type != this->mFilteredImage->getData().type();
      this->mFilteredImage->getData() = cv::Mat::zeros(input_rows, input_cols, input_type);
      output_l.unlock();
      this->mFilteredImage->copyAnnotationsFrom(this->mImage);

      CEDAR_ASSERT(input_channels == 1);

      cedar::proc::Step::ReadLocker locker(this);
      if (this->allInputsValid())
      {
        this->compute(cedar::proc::Arguments());
      }
      locker.unlock();
    }

    if (changed)
    {
      this->emitOutputPropertiesChangedSignal("filteredInput");
    }
  }
}

void cedar::proc::steps::SteerableFilter::recompute()
{
  this->onTrigger();
}

void cedar::proc::steps::SteerableFilter::compute(const cedar::proc::Arguments&)
{
  this->mFilteredImage->setData(mConvolution->convolve(this->mImage->getData()));
}
