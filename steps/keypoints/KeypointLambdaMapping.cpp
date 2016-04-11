/*======================================================================================================================

    Copyright 2011, 2012, 2013, 2014, 2015 Institut fuer Neuroinformatik, Ruhr-Universitaet Bochum, Germany
 
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

    File:        KeypointLambdaMapping.cpp

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2016 04 11

    Description: Source file for the class cedar::proc::steps::KeypointLambdaMapping.

    Credits:

======================================================================================================================*/

// CEDAR CONFIGURATION
#include "cedar/configuration.h"

// CLASS HEADER
#include "steps/keypoints/KeypointLambdaMapping.h"

// CEDAR INCLUDES
#include "cedar/processing/typecheck/Matrix.h"

// SYSTEM INCLUDES
#include <keypoints/keypoints.h>


//----------------------------------------------------------------------------------------------------------------------
// static members
//----------------------------------------------------------------------------------------------------------------------

cedar::aux::EnumType<cedar::proc::steps::KeypointLambdaMapping::DimensionType>
  cedar::proc::steps::KeypointLambdaMapping::DimensionType::mType("cedar::proc::steps::KeypointLambdaMapping::DimensionType::");

//#ifndef CEDAR_COMPILER_MSVC
//const cedar::proc::steps::KeypointLambdaMapping::DimensionType::Id cedar::proc::steps::KeypointLambdaMapping::DimensionType::LogPolarShift;
//const cedar::proc::steps::KeypointLambdaMapping::DimensionType::Id cedar::proc::steps::KeypointLambdaMapping::DimensionType::KeypointLambdaLinear;
//#endif // CEDAR_COMPILER_MSVC


//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::proc::steps::KeypointLambdaMapping::KeypointLambdaMapping()
:
// inputs
mInput(this, "input"),
// outputs
mOutput(new cedar::aux::MatData(cv::Mat())),
// parameters
_mSourceDimension
(
  new cedar::aux::EnumParameter
  (
    this,
    "source dimension",
    cedar::proc::steps::KeypointLambdaMapping::DimensionType::typePtr(),
    cedar::proc::steps::KeypointLambdaMapping::DimensionType::LogPolarShift
  )
),
_mTargetDimension
(
  new cedar::aux::EnumParameter
  (
    this,
    "target dimension",
    cedar::proc::steps::KeypointLambdaMapping::DimensionType::typePtr(),
    cedar::proc::steps::KeypointLambdaMapping::DimensionType::KeypointLambdaLinear
  )
),
_mLambdasStart(new cedar::aux::DoubleParameter(this, "lambdas start", 2.0)),
_mLambdasEnd(new cedar::aux::DoubleParameter(this, "lambdas end", 10.0)),
_mNumberOfLambdas(new cedar::aux::UIntParameter(this, "number of lambdas", 10)),
_mTrainingLambdaIndex(new cedar::aux::UIntParameter(this, "index of training lambda", 4)),
_mMagnitudeForward(new cedar::aux::DoubleParameter(this, "magnitude forward", 23.0)),
_mLogPolarShiftFieldSize(new cedar::aux::UIntParameter(this, "log polar shift field size", 50))
{
  // declare all data
  this->declareOutput("output", mOutput);

  cedar::proc::typecheck::Matrix input_check;
  input_check.addAcceptedDimensionality(1);
  input_check.addAcceptedType(CV_32F);
  this->mInput.getSlot()->setCheck(input_check);

  QObject::connect(this->_mSourceDimension.get(), SIGNAL(valueChanged()), this, SLOT(sourceDimensionChanged()));
  QObject::connect(this->_mTargetDimension.get(), SIGNAL(valueChanged()), this, SLOT(targetDimensionChanged()));
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

void cedar::proc::steps::KeypointLambdaMapping::sourceDimensionChanged()
{
  this->updateMapping();
}

void cedar::proc::steps::KeypointLambdaMapping::targetDimensionChanged()
{
  this->updateMapping();
}

void cedar::proc::steps::KeypointLambdaMapping::allocateOutput()
{
  unsigned int size = 1;
  switch (this->_mTargetDimension->getValue())
  {
    case DimensionType::LogPolarShift:
      size = this->_mLogPolarShiftFieldSize->getValue();
      break;

    case DimensionType::KeypointLambdaLinear:
      size = this->_mNumberOfLambdas->getValue();
      break;

    default:
      break;
  }
  this->mOutput->setData(cv::Mat::zeros(static_cast<int>(size), 1, CV_32F));
}

void cedar::proc::steps::KeypointLambdaMapping::updateMapping()
{
  this->mCoordinateMapping.clear();
  this->mMappingAcuity.clear();

  if (!this->mInput.isSet())
    return;

  this->allocateOutput();

  size_t source_size = cedar::aux::math::get1DMatrixSize(this->mInput.getData());
  size_t target_size = cedar::aux::math::get1DMatrixSize(this->mOutput->getData());
  this->mCoordinateMapping.resize(source_size);
  this->mMappingAcuity.resize(source_size);
  for (size_t s = 0; s < source_size; ++s)
  {
    // determine the scale factor for the source coordinate
    double source_scale = this->getSourceScaleFactorForIndex(s);

    // determine the closest index in the target dimension that is still below the source scale
    unsigned int closest_target_index = 0;
    for (size_t t = 0; t < target_size; ++t)
    {
      double scale = this->getTargetScaleFactorForIndex(t);
      if (source_scale < scale)
      {
        if (t > 0)
          closest_target_index = t - 1;
        else
          closest_target_index = 0;
        break;
      }
    }

    if (source_scale >= this->getTargetScaleFactorForIndex(target_size - 1))
    {
      closest_target_index = target_size - 1;
    }

    double closest_scale = this->getTargetScaleFactorForIndex(closest_target_index);

//    std::cout << "Mapping: " << s << " -> " << closest_target_index << std::endl;
//    std::cout << "       = " << source_scale << " -> " << closest_scale << std::endl;

    // set the source to be mapped to the closest target index
    this->mCoordinateMapping[s] = closest_target_index;

    // determine the acuity of the mapping
    // TODO fix for nonlinear mappings
    if (closest_target_index < target_size - 1)
    {
      double upper = this->getTargetScaleFactorForIndex(closest_target_index + 1);
      double width = upper - closest_scale;
//      std::cout << "   upper & width: " << upper << " & " << width << std::endl;
      this->mMappingAcuity[s] = 1.0f - static_cast<float>((source_scale - closest_scale) / width);
    }
    else
    {
      this->mMappingAcuity[s] = 1.0f;
    }
//    std::cout << "       @ " << this->mMappingAcuity[s] << std::endl;
  }
}

double cedar::proc::steps::KeypointLambdaMapping::getTargetScaleFactorForIndex(unsigned int index) const
{
  size_t size = cedar::aux::math::get1DMatrixSize(this->mOutput->getData());
  return this->getScaleFactorForIndex(this->_mTargetDimension->getValue(), index, size);
}

double cedar::proc::steps::KeypointLambdaMapping::getSourceScaleFactorForIndex(unsigned int index) const
{
  size_t size = cedar::aux::math::get1DMatrixSize(this->mInput.getData());
  return this->getScaleFactorForIndex(this->_mSourceDimension->getValue(), index, size);
}

double cedar::proc::steps::KeypointLambdaMapping::getScaleFactorForIndex(DimensionType::Id dimension, unsigned int index, unsigned int arraySize) const
{
  switch (dimension)
  {
    case DimensionType::LogPolarShift:
    {
      int middle = static_cast<int>(arraySize/2);
      int delta = middle - static_cast<int>(index);
      return std::exp(-static_cast<double>(delta) / this->_mMagnitudeForward->getValue());
    }

    case DimensionType::KeypointLambdaLinear:
    {
      double lambda = this->getLambdaScaleForIndex(index);
      double training_lambda = this->getLambdaScaleForIndex(this->_mTrainingLambdaIndex->getValue());
//      std::cout << "   > index: " << index << std::endl;
//      std::cout << "   > lambda: " << lambda << std::endl;
//      std::cout << "   > training lambda: " << training_lambda << std::endl;
      return lambda / training_lambda;
    }

    default:
      return 0.0f;
  }
}

double cedar::proc::steps::KeypointLambdaMapping::getLambdaScaleForIndex(unsigned int index) const
{
  auto lambdas = vislab::keypoints::makeLambdasLin(this->_mLambdasStart->getValue(), this->_mLambdasEnd->getValue(), this->_mNumberOfLambdas->getValue());
  return lambdas[index];
}

void cedar::proc::steps::KeypointLambdaMapping::inputConnectionChanged(const std::string&)
{
  this->updateMapping();
}

void cedar::proc::steps::KeypointLambdaMapping::compute(const cedar::proc::Arguments&)
{
  if (this->mCoordinateMapping.empty())
  {
    this->updateMapping();
  }

  cv::Mat& output = this->mOutput->getData();
  output.setTo(cv::Scalar(0.0));
  size_t source_size = cedar::aux::math::get1DMatrixSize(this->mInput.getData());
  size_t target_size = cedar::aux::math::get1DMatrixSize(this->mOutput->getData());
  for (size_t s = 0; s < source_size; ++s)
  {
    float acuity = this->mMappingAcuity[s];
    float v = this->mInput.getData().at<float>(static_cast<int>(s), 0);
    int t = this->mCoordinateMapping[s];
    output.at<float>(t, 0) += acuity * v;
    if (static_cast<size_t>(t) < target_size)
    {
      output.at<float>(t + 1, 0) += (1.0f - acuity) * v;
    }
  }
}
