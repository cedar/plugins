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

    File:        ApproximateCouplingVector.cpp

    Maintainer:  
    Email:       
    Date:        

    Description:

    Credits:

======================================================================================================================*/

// LOCAL INCLUDES
#include "steps/neuraloscillator/ApproximateCouplingVector.h"

// PROJECT INCLUDES

// SYSTEM INCLUDES
#include <sstream>
#include <iostream>
#include <algorithm>

#include "ApproximateCoupling.h"

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::proc::steps::ApproximateCouplingVector::ApproximateCouplingVector()
:
Step(true), //is looped, since a large number of independent inputs leads to spam of "compute canceled" messaged
mOutput(new cedar::aux::MatData(cv::Mat::zeros(50,1, CV_32F))),
_mSize(new cedar::aux::UIntParameter(this, "Size", 50,1, 1000)),
_mMinimalDuration(new cedar::aux::DoubleParameter(this, "duration min", 1.0)),
_mMaximalDuration(new cedar::aux::DoubleParameter(this, "duration max", 3.0))
{
  declareOutput("coupling", mOutput);

  QObject::connect(_mSize.get(), SIGNAL(valueChanged()), this, SLOT(vectorDimensionChanged()));
}



//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------


float cedar::proc::steps::ApproximateCouplingVector::calculateDurationFromIndex(unsigned int i, unsigned int siz, unsigned int min, unsigned int max)
{
  return min + ( max-min ) * static_cast<float>(i) / static_cast<float>(siz);
}

void cedar::proc::steps::ApproximateCouplingVector::compute(const cedar::proc::Arguments&)
{
  int i= 0;
  int siz = _mSize->getValue();

  for( ; i < siz; i++ )
  {
    float newC= cedar::proc::steps::ApproximateCoupling::calculateCouplingFromTF( calculateDurationFromIndex( i, siz, _mMinimalDuration->getValue(), _mMaximalDuration->getValue()  ) );

    mOutput->getData().at<float>(i) = newC;
  }
}

void cedar::proc::steps::ApproximateCouplingVector::vectorDimensionChanged()
{
  mOutput->getData()= cv::Mat::zeros( _mSize->getValue(), 
                                       1,
                                       CV_32F);
}

void cedar::proc::steps::ApproximateCouplingVector::inputConnectionChanged(const std::string& )
{
}

cedar::proc::DataSlot::VALIDITY cedar::proc::steps::ApproximateCouplingVector::determineInputValidity
(
  cedar::proc::ConstDataSlotPtr,
  cedar::aux::ConstDataPtr
)
const
{
  return cedar::proc::DataSlot::VALIDITY_VALID;
}

