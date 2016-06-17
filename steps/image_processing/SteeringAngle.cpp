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

    File:        Normalization.cpp

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2011 10 18

    Description:

    Credits:

======================================================================================================================*/

// LOCAL INCLUDES
#include "steps/image_processing/SteeringAngle.h"

// PROJECT INCLUDES

// SYSTEM INCLUDES
#include <cedar/processing/typecheck/And.h>
#include <cedar/processing/typecheck/Matrix.h>
#include <cedar/processing/typecheck/SameSize.h>
#include <cedar/processing/Arguments.h>
#include <cedar/auxiliaries/assert.h>
#include <cedar/auxiliaries/exceptions.h>

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::proc::steps::SteeringAngle::SteeringAngle()
:
mDominantAngle(new cedar::aux::MatData(cv::Mat()))
{
  auto c2_slot = this->declareInput("C2");
  auto c3_slot = this->declareInput("C3");

  cedar::proc::typecheck::SameSize size_check;
  size_check.addSlot(c2_slot);
  size_check.addSlot(c3_slot);

  cedar::proc::typecheck::Matrix input_check;
  input_check.addAcceptedType(CV_32F);
  input_check.addAcceptedDimensionality(2);

  cedar::proc::typecheck::And combined_check;
  combined_check.addCheck(input_check);
  combined_check.addCheck(size_check);

  c2_slot->setCheck(combined_check);
  c3_slot->setCheck(combined_check);

  this->declareOutput("dominantAngle", mDominantAngle);
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

void cedar::proc::steps::SteeringAngle::inputConnectionChanged(const std::string& inputName)
{
  cv::Mat mat;
  if (inputName == "C2")
  {
    mat = this->connectData(inputName, this->mC2);
  }
  else if (inputName == "C3")
  {
    mat = this->connectData(inputName, this->mC3);
  }

  if (mat.empty())
  {
    return;
  }

  CEDAR_ASSERT(mat.channels() == 1);
  bool changed = false;
  if (this->mDominantAngle->getData().cols != mat.cols || this->mDominantAngle->getData().rows != mat.rows)
  {
    changed = true;
    this->mDominantAngle->getData() = cv::Mat::zeros(mat.rows, mat.cols, CV_32F);
  }

  this->callComputeWithoutTriggering();

  if (changed)
  {
    this->emitOutputPropertiesChangedSignal("dominantAngle");
  }
}

cv::Mat cedar::proc::steps::SteeringAngle::connectData(const std::string& name, cedar::aux::ConstMatDataPtr& member)
{
  member = boost::dynamic_pointer_cast<const cedar::aux::MatData>(this->getInput(name));
  if(!member)
  {
    return cv::Mat();
  }
  return member->getData();
}

void cedar::proc::steps::SteeringAngle::compute(const cedar::proc::Arguments&)
{
  const cv::Mat& c2 = mC2->getData();
  const cv::Mat& c3 = mC3->getData();
  cv::Mat& dominant_angle = mDominantAngle->getData();

  cv::phase(c2, c3, dominant_angle, true);
  dominant_angle = dominant_angle / 2.0;
}
