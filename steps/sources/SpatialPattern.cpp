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

    File:        SpatialPattern.cpp

    Maintainer:  Mathis Richter
    Email:       mathis.richter@ini.rub.de
    Date:        2013 10 30

    Description:

    Credits:

======================================================================================================================*/

// LOCAL INCLUDES
#include "steps/sources/SpatialPattern.h"

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

cedar::proc::steps::SpatialPattern::SpatialPattern()
:
mPattern(new cedar::aux::MatData(cv::Mat::zeros(10, 10, CV_32F))),
_mInvertSides(new cedar::aux::BoolParameter(this, "invert sides", false)),
_mHorizontalPattern(new cedar::aux::BoolParameter(this, "horizontal pattern", false)),
_mSizeX(new cedar::aux::UIntParameter(this, "size x", 10, cedar::aux::UIntParameter::LimitType::positive(1000))),
_mSizeY(new cedar::aux::UIntParameter(this, "size y", 10, cedar::aux::UIntParameter::LimitType::positive(1000))),
_mSigmaTh(new cedar::aux::DoubleParameter(this, "sigma th hor", 0.25, cedar::aux::DoubleParameter::LimitType::positiveZero(1.0))),
_mMuR(new cedar::aux::UIntParameter(this, "mu r", 15, cedar::aux::UIntParameter::LimitType::positiveZero(1000))),
_mSigmaR(new cedar::aux::DoubleParameter(this, "sigma r", 100.0, cedar::aux::DoubleParameter::LimitType::positiveZero(1000.0))),
_mScaleSigmoid(new cedar::aux::DoubleParameter(this, "sigma sigmoid fw", 0.475, cedar::aux::DoubleParameter::LimitType::positiveZero(1.0))),
_mSigmoid
(
  new cedar::proc::steps::SpatialPattern::SigmoidParameter
  (
    this,
    "sigmoid",
    cedar::aux::math::SigmoidPtr(new cedar::aux::math::AbsSigmoid(0.0, 100.0))
  )
)
{
  // output
  this->declareOutput("spatial pattern", mPattern);

  QObject::connect(_mInvertSides.get(), SIGNAL(valueChanged()), this, SLOT(recompute()));
  QObject::connect(_mHorizontalPattern.get(), SIGNAL(valueChanged()), this, SLOT(recompute()));
  QObject::connect(_mSizeX.get(), SIGNAL(valueChanged()), this, SLOT(recompute()));
  QObject::connect(_mSizeY.get(), SIGNAL(valueChanged()), this, SLOT(recompute()));
  QObject::connect(_mSigmaTh.get(), SIGNAL(valueChanged()), this, SLOT(recompute()));
  QObject::connect(_mMuR.get(), SIGNAL(valueChanged()), this, SLOT(recompute()));
  QObject::connect(_mSigmaR.get(), SIGNAL(valueChanged()), this, SLOT(recompute()));
  QObject::connect(_mScaleSigmoid.get(), SIGNAL(valueChanged()), this, SLOT(recompute()));
  QObject::connect(_mSigmoid.get(), SIGNAL(valueChanged()), this, SLOT(recompute()));

  this->recompute();
}

cedar::proc::steps::SpatialPattern::~SpatialPattern()
{
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

void cedar::proc::steps::SpatialPattern::recompute()
{
  unsigned int size_x = _mSizeX->getValue();
  unsigned int size_y = _mSizeY->getValue();
  double sigma_th = _mSigmaTh->getValue();
  double mu_r = _mMuR->getValue();
  double sigma_r = _mSigmaR->getValue();
  double scale_sigmoid = _mScaleSigmoid->getValue();

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
      // shift the indices so that the pattern is centered in the output matrix
      double x_shifted = i - ((size_x - 1) / 2.0);
      double y_shifted = j - ((size_y - 1) / 2.0);

      double x = x_shifted;
      double y = y_shifted;

      // this will rotate the pattern by 90 degrees
      if (_mHorizontalPattern->getValue())
      {
        x = y_shifted;
        y = x_shifted;
      }

      // log polar transformation
      // http://docs.opencv.org/modules/imgproc/doc/geometric_transformations.html#logpolar
      double th = atan2(y, invert_sides * x);

      double r = log(sqrt(pow(x, 2.0) + pow(y, 2.0)));

      double gaussian = exp(
                           -0.5 * pow(th, 2.0)       / pow(sigma_th, 2.0)
                           -0.5 * pow(r - mu_r, 2.0) / pow(sigma_r, 2.0)
                           );

      if (cedar::aux::math::isZero(x) && cedar::aux::math::isZero(y))
      {
        gaussian = 0.0;
      }

      // generate a sigmoid to strengthen the relevant side of the pattern and attenuate the other one
      double sigmoid = invert_sides * _mSigmoid->getValue()->compute(x);

      // generate the pattern as a weighted sum of the gaussian and the sigmoid
      mPattern->getData().at<float>(i, j)
        = static_cast<float>((1 - scale_sigmoid) * gaussian + scale_sigmoid * sigmoid);
    }
  }

  // this triggers all connected steps.
  this->onTrigger();
}

void cedar::proc::steps::SpatialPattern::compute(const cedar::proc::Arguments&)
{
}
