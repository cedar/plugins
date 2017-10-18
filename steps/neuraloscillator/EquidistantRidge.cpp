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

cedar::proc::steps::EquidistantRidge::EquidistantRidge()
:
//mDistance(new cedar::aux::MatData(cv::Mat::zeros(50, 1, CV_32F))),
//mDuration(new cedar::aux::MatData(cv::Mat::zeros(50, 1, CV_32F))),
mRidge(new cedar::aux::MatData(cv::Mat::zeros(50, 50, CV_32F))),
mRidgeDuration(new cedar::aux::MatData(cv::Mat::zeros(50, 50, CV_32F))),
mRidgeVelocity(new cedar::aux::MatData(cv::Mat::zeros(50, 50, CV_32F))),
_mMinimalDuration(new cedar::aux::DoubleParameter(this, "duration min", 1.0)),
_mMaximalDuration(new cedar::aux::DoubleParameter(this, "duration max", 3.0)),
_mMinimalDistance(new cedar::aux::DoubleParameter(this, "distance min", 0.0)),
_mMaximalDistance(new cedar::aux::DoubleParameter(this, "distance max", 1.0)),
_mMinimalVelocity(new cedar::aux::DoubleParameter(this, "velocity min", 0.0)),
_mMaximalVelocity(new cedar::aux::DoubleParameter(this, "velocity max", 7.5)),
_mVelocitySize(new cedar::aux::UIntParameter(this, "velocity size", 50)),
_mTau(new cedar::aux::DoubleParameter(this, "tau", 1.0)),
_mH(new cedar::aux::DoubleParameter(this, "h", 1.0))
{
  // inputs
  this->declareInput("distance");
  this->declareInput("duration");

  // output
  this->declareOutput("equidistant ridge", mRidge);

  QObject::connect(_mMinimalDuration.get(), SIGNAL(valueChanged()), this, SLOT(recompute()));
  QObject::connect(_mMaximalDuration.get(), SIGNAL(valueChanged()), this, SLOT(recompute()));
  QObject::connect(_mMinimalDistance.get(), SIGNAL(valueChanged()), this, SLOT(recompute()));
  QObject::connect(_mMaximalDistance.get(), SIGNAL(valueChanged()), this, SLOT(recompute()));
  QObject::connect(_mMinimalVelocity.get(), SIGNAL(valueChanged()), this, SLOT(recompute()));
  QObject::connect(_mMaximalVelocity.get(), SIGNAL(valueChanged()), this, SLOT(recompute()));
  QObject::connect(_mVelocitySize.get(), SIGNAL(valueChanged()), this, SLOT(recompute()));
  QObject::connect(_mTau.get(), SIGNAL(valueChanged()), this, SLOT(recompute()));
  QObject::connect(_mH.get(), SIGNAL(valueChanged()), this, SLOT(recompute()));

  this->recompute();
}

cedar::proc::steps::EquidistantRidge::~EquidistantRidge()
{
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

float cedar::proc::steps::EquidistantRidge::calculateDistanceFromIndex(unsigned int i, unsigned int siz, unsigned int min, unsigned int max)
{
  return min + ( max-min ) * static_cast<float>(i) / static_cast<float>(siz);
}

float cedar::proc::steps::EquidistantRidge::calculateVelocityFromIndex(unsigned int i, unsigned int siz, unsigned int min, unsigned int max)
{
  return min + ( max-min ) * static_cast<float>(i) / static_cast<float>(siz);
}

void cedar::proc::steps::EquidistantRidge::internal_recompute()
{
  auto mDistanceInput= getInput("distance");
  
  if (!mDistanceInput)
    return;
  
  auto mDistance= mDistanceInput->getData<cv::Mat>();

  auto mDurationInput= getInput("duration");
  
  if (!mDurationInput)
    return;

  auto mDuration= mDurationInput->getData<cv::Mat>();

  if (mDistance.empty()
      || mDuration.empty())
    return;

  unsigned int tf_index= 0;
  unsigned int tf_index_max= mDuration.rows;

  unsigned int distance_index_max= mDistance.rows - 1;
  unsigned int velocity_index_max= _mVelocitySize->getValue() - 1;

  mRidge->getData()= cv::Mat::zeros(velocity_index_max + 1,
                          tf_index_max, 
                          CV_32F);
  mRidgeDuration->getData()= cv::Mat::zeros(velocity_index_max + 1,
                          tf_index_max, 
                          CV_32F);
  mRidgeVelocity->getData()= cv::Mat::zeros(velocity_index_max + 1,
                          tf_index_max, 
                          CV_32F);

  auto tf_min = _mMinimalDuration->getValue();
  auto tf_max = _mMaximalDuration->getValue();
  auto dist_min = _mMinimalDistance->getValue();
  auto dist_max = _mMaximalDistance->getValue();
  auto vel_min  = _mMinimalVelocity->getValue();
  auto vel_max  = _mMaximalVelocity->getValue();
  auto tau = _mTau->getValue();
  auto h = _mH->getValue();

  for (; tf_index < tf_index_max; tf_index++)
  {
    // for each duration
    auto oneTF = cedar::proc::steps::ApproximateCouplingVector::calculateDurationFromIndex(tf_index, tf_index_max, tf_min, tf_max);
    auto oneC= cedar::proc::steps::ApproximateCoupling::calculateCouplingFromTF(oneTF);

    // fill straigth ridge:
    unsigned int velocity_index = 0;
    for (; velocity_index <= velocity_index_max; velocity_index++)
    {
      mRidgeDuration->getData().at<float>(velocity_index, tf_index)
            = static_cast<float>( mDuration.at<float>(tf_index) );


      float velmax;
      velmax= calculateVelocityFromIndex(velocity_index, velocity_index_max, vel_min, vel_max);
   
      float fordistance;
      fordistance= cedar::proc::steps::ApproximateInput::calculateDistance( 
        velmax, oneC, oneTF, tau, h);

      unsigned int distance_index;
      distance_index= floor( ( fordistance - dist_min)
                                          / (dist_max - dist_min ) 
                                          * distance_index_max );

      if (distance_index > distance_index_max)
      {
        // todo: error
        std::cout << "Equidistant Ridge max distance too small" << std::endl;
        continue;
      }

      mRidgeVelocity->getData().at<float>(velocity_index, tf_index)
              = static_cast<float>( mDistance.at<float>( distance_index ) );
    }

  }

  mRidge->getData()= mRidgeVelocity->getData() + mRidgeDuration->getData();
}

void cedar::proc::steps::EquidistantRidge::recompute()
{
  internal_recompute();
  // this triggers all connected steps.
  this->onTrigger();
}

void cedar::proc::steps::EquidistantRidge::compute(const cedar::proc::Arguments&)
{
  internal_recompute();
}
