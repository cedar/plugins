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

    File:        EquidistantMatrix.cpp

    Maintainer:  
    Email:       
    Date:        

    Description:

    Credits:

======================================================================================================================*/

// LOCAL INCLUDES
#include "steps/neuraloscillator/EquidistantMatrix.h"
#include "ApproximateInput.h"
#include "ApproximateCoupling.h"
#include "ApproximateCouplingVector.h"

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
#include <boost/pointer_cast.hpp>
#include <iostream>

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::proc::steps::EquidistantMatrix::EquidistantMatrix()
:
//mDistance(new cedar::aux::MatData(cv::Mat::zeros(50, 1, CV_32F))),
//mDuration(new cedar::aux::MatData(cv::Mat::zeros(50, 1, CV_32F))),
mMatrix(new cedar::aux::MatData(cv::Mat::zeros(50, 50, CV_32F))),
_mSizeX(new cedar::aux::UIntParameter(this, "size x", 50)),
_mSizeY(new cedar::aux::UIntParameter(this, "size y", 50)),
_mMinimalDuration(new cedar::aux::DoubleParameter(this, "duration min", 1.0)),
_mMaximalDuration(new cedar::aux::DoubleParameter(this, "duration max", 3.0)),
_mDurationOffset(new cedar::aux::DoubleParameter(this, "duration offset", 0.0)),
_mMinimalDistance(new cedar::aux::DoubleParameter(this, "distance min", 0.0)),
_mMaximalDistance(new cedar::aux::DoubleParameter(this, "distance max", 1.0)),
_mTau(new cedar::aux::DoubleParameter(this, "tau", 1.0)),
_mH(new cedar::aux::DoubleParameter(this, "h", 1.0))
{
  // output
  this->declareOutput("equidistant matrix", mMatrix);

  QObject::connect(_mMinimalDuration.get(), SIGNAL(valueChanged()), this, SLOT(recompute()));
  QObject::connect(_mMaximalDuration.get(), SIGNAL(valueChanged()), this, SLOT(recompute()));
  QObject::connect(_mMinimalDistance.get(), SIGNAL(valueChanged()), this, SLOT(recompute()));
  QObject::connect(_mMaximalDistance.get(), SIGNAL(valueChanged()), this, SLOT(recompute()));
  QObject::connect(_mTau.get(), SIGNAL(valueChanged()), this, SLOT(recompute()));
  QObject::connect(_mH.get(), SIGNAL(valueChanged()), this, SLOT(recompute()));

  this->recompute();
}

cedar::proc::steps::EquidistantMatrix::~EquidistantMatrix()
{
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

float cedar::proc::steps::EquidistantMatrix::calculateDistanceFromIndex(unsigned int i, unsigned int siz, float min, float max)
{
  return min + ( max-min ) * static_cast<float>(i) / static_cast<float>(siz);
}

float cedar::proc::steps::EquidistantMatrix::calculateVelocityFromIndex(unsigned int i, unsigned int siz, float min, float max)
{
  return min + ( max-min ) * static_cast<float>(i) / static_cast<float>(siz);
}

void cedar::proc::steps::EquidistantMatrix::internal_recompute()
{
  unsigned int tf_index= 0;
  unsigned int tf_index_max= _mSizeX->getValue();

  unsigned int distance_index_max= _mSizeY->getValue();

  mMatrix->getData()= cv::Mat::zeros(distance_index_max,
                          tf_index_max, 
                          CV_32F);

  auto tf_min = _mMinimalDuration->getValue();
  auto tf_max = _mMaximalDuration->getValue();
  auto dist_min = _mMinimalDistance->getValue();
  auto dist_max = _mMaximalDistance->getValue();
  auto tau = _mTau->getValue();
  auto h = _mH->getValue();

  for (; tf_index < tf_index_max; tf_index++)
  {
    // for each duration
    auto oneTF = cedar::proc::steps::ApproximateCouplingVector::calculateDurationFromIndex(tf_index, tf_index_max, tf_min, tf_max);
    auto oneC= cedar::proc::steps::ApproximateCoupling::calculateCouplingFromTF(oneTF - _mDurationOffset->getValue() );

    unsigned int distance_index = 0;
    for (; distance_index <distance_index_max; distance_index++)
    {
      float distance;
      distance= calculateDistanceFromIndex(distance_index, distance_index_max, dist_min, dist_max);

      float velmax;
      velmax= cedar::proc::steps::ApproximateInput::calculateMaxVelocity(distance, oneC, oneTF, tau, h);
      if (velmax < 0)
      {
        velmax= 0.0;
      }

      float W;
      float s = 1.0;
      W = cedar::proc::steps::ApproximateInput::calculateDistance(s, oneC, oneTF, tau, h);

//std::cout << "W: " << W << "  dist: " << distance << "  res: " << distance / W << std::endl;
      if (W == 0.0)
      {
std::cout << "W ist 0!   dist: " << distance << " s: " << s << " tf: " << oneTF << " C: " << oneC << " tau " << tau << " h: " << h << std::endl;        
        mMatrix->getData().at<float>(distance_index, tf_index)
                  = 0.0f;
      }
      else
      {
        mMatrix->getData().at<float>(distance_index, tf_index)
                  = distance / W;
      }
    }

  }

}

void cedar::proc::steps::EquidistantMatrix::recompute()
{
  internal_recompute();
  // this triggers all connected steps.
  this->onTrigger();
}

void cedar::proc::steps::EquidistantMatrix::compute(const cedar::proc::Arguments&)
{
  internal_recompute();
}
