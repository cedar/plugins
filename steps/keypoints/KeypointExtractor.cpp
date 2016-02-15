/*======================================================================================================================

    Copyright 2011, 2012, 2013, 2014 Institut fuer Neuroinformatik, Ruhr-Universitaet Bochum, Germany
 
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

    File:        KeypointExtractor.cpp

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2014 12 08

    Description: Source file for the class keypoints::KeypointExtractor.

    Credits:

======================================================================================================================*/

// CEDAR CONFIGURATION
#include "cedar/configuration.h"

// CLASS HEADER
#include "steps/keypoints/KeypointExtractor.h"

// CEDAR INCLUDES
#include "cedar/processing/typecheck/Matrix.h"
#include "cedar/processing/Arguments.h"

// SYSTEM INCLUDES


//----------------------------------------------------------------------------------------------------------------------
// static members
//----------------------------------------------------------------------------------------------------------------------

cedar::aux::EnumType<cedar::proc::steps::KeypointExtractor::ScaleType>
cedar::proc::steps::KeypointExtractor::ScaleType::mType("cedar::proc::steps::KeypointExtractor::ScaleType::");

#ifndef CEDAR_COMPILER_MSVC
const cedar::proc::steps::KeypointExtractor::ScaleType::Id cedar::proc::steps::KeypointExtractor::ScaleType::Linear;
const cedar::proc::steps::KeypointExtractor::ScaleType::Id cedar::proc::steps::KeypointExtractor::ScaleType::Logarithmic;
#endif // CEDAR_COMPILER_MSVC

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::proc::steps::KeypointExtractor::KeypointExtractor()
:
// outputs
mInput(this, "image"),
mKeypointData(new cedar::aux::KeypointData()),
mKeypoints(new cedar::aux::KeypointListData()),
_mOrientations(new cedar::aux::UIntParameter(this, "number of orientations", 8)),
_mScale(new cedar::aux::BoolParameter(this, "apply scaling", true)),
_mLambdasStart(new cedar::aux::DoubleParameter(this, "lambdas start", 5.0)),
_mLambdasEnd(new cedar::aux::DoubleParameter(this, "lambdas end", 16.0)),
_mNumLambdas(new cedar::aux::UIntParameter(this, "number of lambdas", 8)),
_mScaleType(new cedar::aux::EnumParameter(this, "scale type", cedar::proc::steps::KeypointExtractor::ScaleType::typePtr(), cedar::proc::steps::KeypointExtractor::ScaleType::Linear))
{
  // declare all data
  this->declareOutput("keypoint data", this->mKeypointData);
  this->declareOutput("keypoints", this->mKeypoints);

  cedar::proc::typecheck::Matrix input_check;
  input_check.addAcceptedDimensionality(2);
  input_check.addAcceptedType(CV_8UC1);
  mInput.getSlot()->setCheck(input_check);

  QObject::connect(this->_mOrientations.get(), SIGNAL(valueChanged()), this, SLOT(recompute()));
  QObject::connect(this->_mScale.get(), SIGNAL(valueChanged()), this, SLOT(recompute()));
  QObject::connect(this->_mLambdasStart.get(), SIGNAL(valueChanged()), this, SLOT(updateLambdas()));
  QObject::connect(this->_mLambdasEnd.get(), SIGNAL(valueChanged()), this, SLOT(updateLambdas()));
  QObject::connect(this->_mNumLambdas.get(), SIGNAL(valueChanged()), this, SLOT(updateLambdas()));
  QObject::connect(this->_mScaleType.get(), SIGNAL(valueChanged()), this, SLOT(updateLambdas()));

  this->updateLambdas();
}

void cedar::proc::steps::KeypointExtractor::ScaleType::construct()
{
  mType.type()->def(cedar::aux::Enum(Linear, "Linear"));
  mType.type()->def(cedar::aux::Enum(Logarithmic, "Logarithmic"));
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------
  
const cedar::aux::EnumBase& cedar::proc::steps::KeypointExtractor::ScaleType::type()
{
  return *cedar::proc::steps::KeypointExtractor::ScaleType::mType.type();
}

const cedar::proc::steps::KeypointExtractor::ScaleType::TypePtr& cedar::proc::steps::KeypointExtractor::ScaleType::typePtr()
{
  return cedar::proc::steps::KeypointExtractor::ScaleType::mType.type();
}

size_t cedar::proc::steps::KeypointExtractor::getScaleIndex(double scale) const
{
  size_t index = 0;
  double distance = std::numeric_limits<double>::max();
  // since we are dealing with floating point values, find the index of the one closest to the target scale
  for (size_t i = 0; i < this->mLambdas.size(); ++i)
  {
    double d = std::abs(this->mLambdas.at(i) - scale);
    if (d < distance)
    {
      distance = d;
      index = i;
    }
  }
  
  CEDAR_NON_CRITICAL_ASSERT(distance < 0.01);
  
  return index;
}

void cedar::proc::steps::KeypointExtractor::inputConnectionChanged(const std::string&)
{
  this->updateAndRecompute();
}

void cedar::proc::steps::KeypointExtractor::recompute()
{
  this->onTrigger();
}

void cedar::proc::steps::KeypointExtractor::updateAndRecompute()
{
  cedar::proc::Step::ReadLocker locker(this);
  if (this->allInputsValid())
  {
    this->compute(cedar::proc::Arguments());
  }
  locker.unlock();

  this->emitOutputPropertiesChangedSignal("keypoint data");

  this->onTrigger();
}

void cedar::proc::steps::KeypointExtractor::updateLambdas()
{
  cedar::proc::Step::ReadLocker locker(this);

  size_t size_pre = this->mLambdas.size();
  double lambda_start = this->_mLambdasStart->getValue();
  double lambda_end = this->_mLambdasEnd->getValue();
  int num_lambdas = static_cast<int>(this->_mNumLambdas->getValue());

  switch (this->_mScaleType->getValue())
  {
    case cedar::proc::steps::KeypointExtractor::ScaleType::Linear:
      this->mLambdas = vislab::keypoints::makeLambdasLin(lambda_start, lambda_end, num_lambdas);
      break;

    case cedar::proc::steps::KeypointExtractor::ScaleType::Logarithmic:
      this->mLambdas = vislab::keypoints::makeLambdasLog(lambda_start, lambda_end, num_lambdas);
      break;

    default:
      CEDAR_THROW(cedar::aux::UnhandledTypeException, "Unhandled enum value.");
  }

  if (this->allInputsValid())
  {
    this->compute(cedar::proc::Arguments());
  }
  size_t size_post = this->mLambdas.size();
  locker.unlock();

  if (size_pre != size_post)
  {
    this->emitOutputPropertiesChangedSignal("keypoint data");
  }

  this->onTrigger();
}

void cedar::proc::steps::KeypointExtractor::compute(const cedar::proc::Arguments&)
{
  if (!this->mInput.isSet())
  {
    return;
  }
  const cv::Mat& input = this->mInput.getData();
  auto& keypoint_data = this->mKeypointData->getData();
  int orientations = static_cast<int>(this->_mOrientations->getValue());
  bool scaling = this->_mScale->getValue();

  auto keypoints = vislab::keypoints::keypoints(input, this->mLambdas, keypoint_data, orientations, scaling);
  this->mKeypoints->getData().assign(keypoints.begin(), keypoints.end());
}
