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

    File:        ApproximateCoupling.cpp

    Maintainer:  
    Email:       
    Date:        

    Description:

    Credits:

======================================================================================================================*/

// LOCAL INCLUDES
#include "steps/neuraloscillator/ApproximateCoupling.h"

// PROJECT INCLUDES
#include "steps/neuraloscillator/NeuralTimer.h"

// SYSTEM INCLUDES
#include <sstream>
#include <iostream>
#include <algorithm>

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::proc::steps::ApproximateCoupling::ApproximateCoupling()
:
Step(true), //is looped, since a large number of independent inputs leads to spam of "compute canceled" messaged
mOutput(new cedar::aux::MatData(cv::Mat::zeros(1,1, CV_32F))),
mInputs(1, cedar::aux::MatDataPtr()),
_mOutputDimension (new cedar::aux::UIntParameter(this, "VectorDimension", 1,1,255))
{
  declareOutput("coupling", mOutput);
  declareInput("duration", false);

  QObject::connect(_mOutputDimension.get(), SIGNAL(valueChanged()), this, SLOT(vectorDimensionChanged()));
}



//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

float cedar::proc::steps::ApproximateCoupling::calculateCouplingFromTF(float tf)
{
  return cedar::proc::steps::NeuralTimer::calculateCouplingFromTF(tf);
}


void cedar::proc::steps::ApproximateCoupling::compute(const cedar::proc::Arguments&)
{
  auto data = getInput("duration");
  if (!data)
    return;
  auto mat = data->getData<cv::Mat>();
  if (mat.empty())
    return;

  float newC= calculateCouplingFromTF( mat.at<float>(0,0)  );

  mOutput->getData().create(1, 1, CV_32F);
  mOutput->getData().at<float>(0,0) = newC;
}

void cedar::proc::steps::ApproximateCoupling::vectorDimensionChanged()
{
}

void cedar::proc::steps::ApproximateCoupling::inputConnectionChanged(const std::string& )
{
}

cedar::proc::DataSlot::VALIDITY cedar::proc::steps::ApproximateCoupling::determineInputValidity
(
  cedar::proc::ConstDataSlotPtr,
  cedar::aux::ConstDataPtr
)
const
{
  return cedar::proc::DataSlot::VALIDITY_VALID;
}

