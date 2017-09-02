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

    File:        EquidistantRidge.cpp

    Maintainer:  
    Email:       
    Date:        

    Description:

    Credits:

======================================================================================================================*/

// LOCAL INCLUDES
#include "steps/neuraloscillator/EquidistantRidge.h"

// SYSTEM INCLUDES
#include <cedar/auxiliaries/assert.h>
#include <cedar/auxiliaries/exceptions.h>
#include <cedar/auxiliaries/DataTemplate.h>
#include <cedar/auxiliaries/MatData.h>
#include <cedar/auxiliaries/BoolParameter.h>
#include <cedar/auxiliaries/UIntParameter.h>
#include <cedar/auxiliaries/DoubleParameter.h>
#include <cedar/auxiliaries/ObjectParameterTemplate.h>
#include <cedar/auxiliaries/math/TransferFunction.h>
#include <cedar/auxiliaries/math/transferFunctions/AbsSigmoid.h>
#include <cedar/auxiliaries/math/tools.h>
#include <cmath>

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::proc::steps::EquidistantRidge::EquidistantRidge()
:
mPattern(new cedar::aux::MatData(cv::Mat::zeros(10, 10, CV_32F))),
_mInvertSides(new cedar::aux::BoolParameter(this, "invert sides", false)),
_mHorizontalPattern(new cedar::aux::BoolParameter(this, "horizontal pattern", false)),
_mSizeX(new cedar::aux::UIntParameter(this, "size x", 10, cedar::aux::UIntParameter::LimitType::positive(1000))),
_mSizeY(new cedar::aux::UIntParameter(this, "size y", 10, cedar::aux::UIntParameter::LimitType::positive(1000)))
{
  // output
  this->declareOutput("spatial pattern", mPattern);

  QObject::connect(_mInvertSides.get(), SIGNAL(valueChanged()), this, SLOT(recompute()));
  QObject::connect(_mHorizontalPattern.get(), SIGNAL(valueChanged()), this, SLOT(recompute()));
  QObject::connect(_mSizeX.get(), SIGNAL(valueChanged()), this, SLOT(recompute()));
  QObject::connect(_mSizeY.get(), SIGNAL(valueChanged()), this, SLOT(recompute()));

  this->recompute();
}

cedar::proc::steps::EquidistantRidge::~EquidistantRidge()
{
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

void cedar::proc::steps::EquidistantRidge::recompute()
{
  unsigned int size_x = _mSizeX->getValue();
  unsigned int size_y = _mSizeY->getValue();

  double invert_sides = 1.0;

  // this will switch the sides on which the pattern strengthens and attenuates
  // to be able to generate patterns for left/right (or top/bottom)
  if (_mInvertSides->getValue())
  {
    invert_sides = -1.0;
  }

  mPattern->getData() = cv::Mat(size_x, size_y, CV_32F);

  // go through all positions of the pattern
  for (unsigned int i = 0; i < size_x; ++i)
  {
    for (unsigned int j = 0; j < size_y; ++j)
    {
      double x = 0;
      double y = 0;

      // this will rotate the pattern by 90 degrees
      if (_mHorizontalPattern->getValue())
      {
        x = y;
        y = x;
      }

      double value;

      // generate the pattern as a weighted sum of the gaussian and the sigmoid
      mPattern->getData().at<float>(i, j)
        = static_cast<float>( value );
    }
  }

  // this triggers all connected steps.
  this->onTrigger();
}

void cedar::proc::steps::EquidistantRidge::compute(const cedar::proc::Arguments&)
{
}
