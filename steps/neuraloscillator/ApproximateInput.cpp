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

    File:        ApproximateInput.cpp

    Maintainer:  Guido Knips
    Email:       guido.knips@ini.rub.de
    Date:        2013 12 04

    Description:

    Credits:

======================================================================================================================*/

// LOCAL INCLUDES
#include "steps/neuraloscillator/ApproximateInput.h"

// PROJECT INCLUDES
#include "steps/neuraloscillator/NeuralTimer.h"

// SYSTEM INCLUDES
#include <sstream>
#include <iostream>
#include <algorithm>

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::proc::steps::ApproximateInput::ApproximateInput()
:
Step(true), //is looped, since a large number of independent inputs leads to spam of "compute canceled" messaged
mOutput(new cedar::aux::MatData(cv::Mat::zeros(1,1, CV_32F))),
mInputs(1, cedar::aux::MatDataPtr()),
_mTau( new cedar::aux::DoubleParameter ( this, "tau", 100.0, 0.0, 100000.0 ) ),
_mH( new cedar::aux::DoubleParameter ( this, "h (negative)", -5.0, -100000.0, 0.0 ) )
{
  declareOutput("S", mOutput);
  declareInput("duration", false);
  declareInput("coupling", false);
  declareInput("distance", false);

}



//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

float cedar::proc::steps::ApproximateInput::calculateMaxVelocity(float D, float
c, float tf, float tau, float h)
{
  return cedar::proc::steps::NeuralTimer::calculateInputS(D, c, tf, tau, h);
}

float cedar::proc::steps::ApproximateInput::calculateDistance(float s, float
c, float tf, float tau, float h)
{
  return cedar::proc::steps::NeuralTimer::calculateBaseIntegral(s, c, tf, tau, h);
}

void cedar::proc::steps::ApproximateInput::compute(const cedar::proc::Arguments&)
{
  float h = _mH->getValue();
  float tau = _mTau->getValue() / 1000.0;

  auto couplingDataPtr = getInput("coupling");
  auto durationDataPtr = getInput("duration");
  auto distanceDataPtr = getInput("distance");

  if (!couplingDataPtr
      || !durationDataPtr
      || !distanceDataPtr)
    return;

  auto couplingData = couplingDataPtr->getData<cv::Mat>();
  auto durationData = durationDataPtr->getData<cv::Mat>();
  auto distanceData = distanceDataPtr->getData<cv::Mat>();

  if (couplingData.empty()
      || durationData.empty()
      || distanceData.empty())
    return;

  float c = couplingData.at<float>(0,0);
  float tf = durationData.at<float>(0,0);
  float D = distanceData.at<float>(0,0);

  float newS;
  newS= calculateMaxVelocity(D, c, tf, tau, h);

  mOutput->getData().create(1, 1, CV_32F);
  mOutput->getData().at<float>(0,0) = newS;
}


void cedar::proc::steps::ApproximateInput::inputConnectionChanged(const std::string&)
{

}

cedar::proc::DataSlot::VALIDITY cedar::proc::steps::ApproximateInput::determineInputValidity
(
  cedar::proc::ConstDataSlotPtr,
  cedar::aux::ConstDataPtr
)
const
{
  return cedar::proc::DataSlot::VALIDITY_VALID;
}

