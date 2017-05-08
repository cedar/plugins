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

    File:        Rescaler.cpp

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2015 02 13

    Description: Source file for the class keypoints::Rescaler.

    Credits:

======================================================================================================================*/

// CEDAR CONFIGURATION
#include "cedar/configuration.h"

// CLASS HEADER
#include "steps/keypoints/KeypointPatchRescaler.h"

// CEDAR INCLUDES
#include "cedar/processing/typecheck/Matrix.h"

// PROJECT INCLUDES
#include "steps/keypoints/KeypointExtractor.h"

// SYSTEM INCLUDES

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::proc::steps::KeypointPatchRescaler::KeypointPatchRescaler()
:
// inputs
mImage(this, "image"),
mScaleVector(this, "scale selection"),
// outputs
mOutput(new cedar::aux::MatData(cv::Mat())),
// parameters
_mLambdasStart(new cedar::aux::DoubleParameter(this, "lambdas start", 5.0)),
_mLambdasEnd(new cedar::aux::DoubleParameter(this, "lambdas end", 16.0)),
_mNumLambdas(new cedar::aux::UIntParameter(this, "number of lambdas", 8)),
_mScaleType(new cedar::aux::EnumParameter(this, "scale type", cedar::proc::steps::KeypointExtractor::ScaleType::typePtr(), cedar::proc::steps::KeypointExtractor::ScaleType::Linear)),
_mOutputSize(new cedar::aux::UIntParameter(this, "output size", 13)),
_mPadding(new cedar::aux::UIntParameter(this, "padding", 0))
{
  // declare all data
  this->declareOutput("rescaled patch", mOutput);

  cedar::proc::typecheck::Matrix image_check;
  image_check.addAcceptedDimensionality(2);
  image_check.addAcceptedType(CV_32F);
  this->mImage.getSlot()->setCheck(image_check);

  cedar::proc::typecheck::Matrix scale_vector_check;
  scale_vector_check.addAcceptedDimensionality(1);
  scale_vector_check.addAcceptedType(CV_32F);
  this->mScaleVector.getSlot()->setCheck(scale_vector_check);
  
  QObject::connect(this->_mLambdasStart.get(), SIGNAL(valueChanged()), this, SLOT(updateLambdas()));
  QObject::connect(this->_mLambdasEnd.get(), SIGNAL(valueChanged()), this, SLOT(updateLambdas()));
  QObject::connect(this->_mNumLambdas.get(), SIGNAL(valueChanged()), this, SLOT(updateLambdas()));
  QObject::connect(this->_mScaleType.get(), SIGNAL(valueChanged()), this, SLOT(updateLambdas()));
  
  QObject::connect(this->_mOutputSize.get(), SIGNAL(valueChanged()), this, SLOT(updateOutputSize()));
  QObject::connect(this->_mPadding.get(), SIGNAL(valueChanged()), this, SLOT(updateOutputSize()));
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

void cedar::proc::steps::KeypointPatchRescaler::updateLambdas()
{
  //!@todo This and all code related to the lambdas is a copy from keypoints::KeypointExtractor; unify!
  cedar::proc::Step::ReadLocker locker(this);

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
  locker.unlock();

  this->onTrigger();
}

void cedar::proc::steps::KeypointPatchRescaler::updateOutputSize()
{
  int padding = static_cast<int>(this->getPadding());
  int rows = 2 * padding + static_cast<int>(this->getOutputSize());
  int columns = rows;
  this->mOutput->setData(cv::Mat::zeros(rows, columns, CV_32F));

  this->emitOutputPropertiesChangedSignal("rescaled patch");
}

void cedar::proc::steps::KeypointPatchRescaler::compute(const cedar::proc::Arguments& /* arguments */)
{
  // -- determine the lambda/scale of the current keypoint --
  // find index of maximum in the scale input
  const cv::Mat& scale_vector = this->mScaleVector.getData();
  cv::Point max_loc;
  cv::minMaxLoc(scale_vector, nullptr, nullptr, nullptr, &max_loc);
  size_t max_idx;
  if (max_loc.x == 0)
  {
    max_idx = static_cast<size_t>(max_loc.y);
  }
  else
  {
    max_idx = static_cast<size_t>(max_loc.x);
  }
  
  int output_size = static_cast<int>(this->getOutputSize());
  int padding = static_cast<int>(this->getPadding());

  // get lambda corresponding to the index of the maximum
  CEDAR_ASSERT(max_idx < this->mLambdas.size());
  double lambda = this->mLambdas.at(max_idx);
  double factor = lambda / static_cast<double>(output_size);
  double lambda_padding = static_cast<double>(this->getPadding()) * factor;
  int lambda_px = static_cast<int>(std::ceil(lambda + 2.0 * lambda_padding));
  
  // -- cut out the center region with the size of the current keypoint --
  const cv::Mat& input = this->mImage.getData();
  int lower = lambda_px / 2;
  int upper = lambda_px - lower;
  cv::Mat cutout = input(cv::Range(input.rows/2 - lower, input.rows/2 + upper), cv::Range(input.cols/2 - lower, input.cols/2 + upper));
  
  // -- rescale the cut-out to the output size --
  cv::Mat& scaled_patch = this->mOutput->getData();
  cv::resize(cutout, scaled_patch, cv::Size(output_size + 2 * padding, output_size + 2 * padding), 0, 0, cv::INTER_LINEAR);
}

