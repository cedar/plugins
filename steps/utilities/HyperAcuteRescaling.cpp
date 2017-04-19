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

    File:        HyperAcuteRescaling.cpp

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2016 02 19

    Description: Source file for the class cedar::proc::steps::HyperAcuteRescaling.

    Credits:

======================================================================================================================*/

// CEDAR CONFIGURATION
#include "cedar/configuration.h"

// CLASS HEADER
#include "steps/utilities/HyperAcuteRescaling.h"

// CEDAR INCLUDES
#include "cedar/processing/typecheck/Matrix.h"

// SYSTEM INCLUDES

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::proc::steps::HyperAcuteRescaling::HyperAcuteRescaling()
:
// inputs
mInput(this, "input matrix"),
// outputs
mOutput(new cedar::aux::MatData(cv::Mat(8, 1, CV_32F, cv::Scalar(0)))),
_mTargetSize(new cedar::aux::UIntParameter(this, "target size", 8))
{
  cedar::proc::typecheck::Matrix input_check;
  input_check.addAcceptedDimensionality(1);
  input_check.addAcceptedType(CV_32F);
  this->mInput.getSlot()->setCheck(input_check);

  // declare the output data
  this->declareOutput("rescaled matrix", mOutput);

  QObject::connect(this->_mTargetSize.get(), SIGNAL(valueChanged()), this, SLOT(outputSizeChanged()));
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

void cedar::proc::steps::HyperAcuteRescaling::compute(const cedar::proc::Arguments&)
{
  const cv::Mat& matrix = this->mInput.getData();

  int matrix_size = static_cast<int>(cedar::aux::math::get1DMatrixSize(matrix));
  int target_size = this->_mTargetSize->getValue();

  cv::Mat& output = this->mOutput->getData();
  output.create(target_size, 1, CV_32F);
  output.setTo(0.0);

  float progress_per_target_bin = 1.0f / static_cast<float>(target_size);
  for (int i = 0; i < matrix_size; ++i)
  {
    float progress = static_cast<float>(i) / static_cast<float>(matrix_size);
    int lower_target_bin = static_cast<int>(std::floor(progress * static_cast<float>(target_size)));

    float v = matrix.at<float>(i);
    float acuity = (progress - (static_cast<float>(lower_target_bin) * progress_per_target_bin)) / progress_per_target_bin;
    output.at<float>(lower_target_bin) += (1.0 - acuity) * v;
    output.at<float>((lower_target_bin + 1) % target_size) += acuity * v;
  }

  output /= static_cast<float>(static_cast<float>(matrix_size) / static_cast<float>(target_size));
}

void cedar::proc::steps::HyperAcuteRescaling::outputSizeChanged()
{
  this->onTrigger();
}
