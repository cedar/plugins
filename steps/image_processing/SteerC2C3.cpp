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
#include "steps/image_processing/SteerC2C3.h"

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

cedar::proc::steps::SteerC2C3::SteerC2C3()
:
mC2(new cedar::aux::MatData(cv::Mat(1, 1, CV_32F))),
mC3(new cedar::aux::MatData(cv::Mat(1, 1, CV_32F)))
{
  std::vector<cedar::proc::DataSlotPtr> input_slots;

  input_slots.push_back(this->declareInput("G2a"));
  input_slots.push_back(this->declareInput("G2b"));
  input_slots.push_back(this->declareInput("G2c"));

  input_slots.push_back(this->declareInput("H2a"));
  input_slots.push_back(this->declareInput("H2b"));
  input_slots.push_back(this->declareInput("H2c"));
  input_slots.push_back(this->declareInput("H2d"));

  cedar::proc::typecheck::SameSize size_check;
  for (auto slot : input_slots)
  {
    size_check.addSlot(slot);
  }

  cedar::proc::typecheck::Matrix input_check;
  input_check.addAcceptedType(CV_32F);
  input_check.addAcceptedDimensionality(2);

  cedar::proc::typecheck::And combined_check;
  combined_check.addCheck(input_check);
  combined_check.addCheck(size_check);

  for (auto slot : input_slots)
  {
    slot->setCheck(combined_check);
  }

  this->declareOutput("C2", mC2);
  this->declareOutput("C3", mC3);
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

void cedar::proc::steps::SteerC2C3::inputConnectionChanged(const std::string& inputName)
{
  cv::Mat mat;
  if (inputName == "G2a")
  {
    mat = this->connectData(inputName, this->mG2a);
  }
  else if (inputName == "G2b")
  {
    mat = this->connectData(inputName, this->mG2b);
  }
  else if (inputName == "G2c")
  {
    mat = this->connectData(inputName, this->mG2c);
  }
  else if (inputName == "H2a")
  {
    mat = this->connectData(inputName, this->mH2a);
  }
  else if (inputName == "H2b")
  {
    mat = this->connectData(inputName, this->mH2b);
  }
  else if (inputName == "H2c")
  {
    mat = this->connectData(inputName, this->mH2c);
  }
  else if (inputName == "H2d")
  {
    mat = this->connectData(inputName, this->mH2d);
  }

  bool changed = false;
  if (!mat.empty())
  {
    CEDAR_ASSERT(mat.channels() == 1);
    if
    (
      this->mC2->getData().cols != mat.cols
      || this->mC2->getData().rows != mat.rows
      || this->mC3->getData().cols != mat.cols
      || this->mC3->getData().rows != mat.rows
    )
    {
      changed = true;
      this->mC2->getData() = cv::Mat(mat.rows, mat.cols, CV_32F);
      this->mC3->getData() = cv::Mat(mat.rows, mat.cols, CV_32F);
    }

    this->callComputeWithoutTriggering();
  }

  if (changed)
  {
    this->emitOutputPropertiesChangedSignal("C2");
    this->emitOutputPropertiesChangedSignal("C3");
  }

  std::vector<std::string> input_slots;
  input_slots.push_back("G2a");
  input_slots.push_back("G2b");
  input_slots.push_back("G2c");
  input_slots.push_back("H2a");
  input_slots.push_back("H2b");
  input_slots.push_back("H2c");
  input_slots.push_back("H2d");

  for (const auto& slot : input_slots)
  {
    if (slot != inputName)
    {
      this->redetermineInputValidity(slot);
    }
  }
}

cv::Mat cedar::proc::steps::SteerC2C3::connectData(const std::string& name, cedar::aux::ConstMatDataPtr& member)
{
  member = boost::dynamic_pointer_cast<const cedar::aux::MatData>(this->getInput(name));
  if (!member)
  {
    return cv::Mat();
  }
  return member->getData();
}

void cedar::proc::steps::SteerC2C3::compute(const cedar::proc::Arguments&)
{
  const cv::Mat& g2a = this->mG2a->getData();
  const cv::Mat& g2b = this->mG2b->getData();
  const cv::Mat& g2c = this->mG2c->getData();
  const cv::Mat& h2a = this->mH2a->getData();
  const cv::Mat& h2b = this->mH2b->getData();
  const cv::Mat& h2c = this->mH2c->getData();
  const cv::Mat& h2d = this->mH2d->getData();

  cv::Mat& c2 = mC2->getData();
  cv::Mat& c3 = mC3->getData();

  c2 = .5 * (g2a.mul(g2a) - g2c.mul(g2c)) + .46875 * (h2a.mul(h2a) - h2d.mul(h2d)) + .28125 * (h2b.mul(h2b)
      - h2c.mul(h2c)) + .1875 * (h2a.mul(h2c) - h2b.mul(h2d));
  c3 = - g2a.mul(g2b) - g2b.mul(g2c) - .9375 * (h2c.mul(h2d) + h2a.mul(h2b)) - 1.6875 * h2b.mul(h2c)
      - .1875 * h2a.mul(h2d);
}
