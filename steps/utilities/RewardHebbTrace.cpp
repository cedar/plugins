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

 File:        RewardHebbTrace.cpp

 Maintainer:  Jan Tekülve
 Email:       jan.tekuelve@ini.rub.de
 Date:        2016 10 28

 Description: Source file for the class cedar::proc::steps::RewardHebbTrace.

 Credits:

 ======================================================================================================================*/

// CEDAR CONFIGURATION
#include "cedar/configuration.h"

// CLASS HEADER
#include "steps/utilities/RewardHebbTrace.h"

#include "cedar/processing/typecheck/IsMatrix.h"
#include "cedar/processing/DataSlot.h"
#include "cedar/processing/ElementDeclaration.h"
#include "cedar/processing/DeclarationRegistry.h"
#include "cedar/auxiliaries/assert.h"
#include "cedar/auxiliaries/exceptions.h"
#include "cedar/units/Time.h"
#include "cedar/units/prefixes.h"
#include "cedar/processing/steps/Sum.h"
#include <cedar/auxiliaries/math/TransferFunction.h>
#include "cedar/auxiliaries/math/transferFunctions/HeavisideSigmoid.h"

// SYSTEM INCLUDES
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------
cedar::proc::steps::RewardHebbTrace::RewardHebbTrace()
        :
// parameters
        mAssociationDimension(new cedar::aux::UIntParameter(this, "association dimension", 2, 0, 2)),
        mAssociationSizes(new cedar::aux::UIntVectorParameter(this, "association sizes", 2, 50)),
        mLearnRatePositive(new cedar::aux::DoubleParameter(this, "learning rate", 0.001)),
        mUseRewardDuration(new cedar::aux::BoolParameter(this, "fixed reward duration", false)),
        mRewardDuration(new cedar::aux::DoubleParameter(this, "reward duration", 200)),
        mSetWeights(new cedar::aux::BoolParameter(this, "manual weights", false)),
        mWeightCenters(
                new cedar::aux::DoubleVectorParameter(this, "weight centers", mAssociationDimension->getValue(), 3)),
        mWeightSigmas(
                new cedar::aux::DoubleVectorParameter(this, "weight sigmas", mAssociationDimension->getValue(), 3)),
        mWeightAmplitude(new cedar::aux::DoubleParameter(this, "weight amplitude", 6)),
        mRewardThreshold(new cedar::aux::DoubleParameter(this, "reward threshold", 0.5)),
        mLearnedThreshold(new cedar::aux::DoubleParameter(this, "rec. weight threshold", 0.5,0,1)),
        // outputs
        mConnectionWeights(new cedar::aux::MatData(cv::Mat::zeros(100, 100, CV_32F))),
        mWeightOutput((new cedar::aux::MatData(cv::Mat::zeros(100, 100, CV_32F)))),
        mRecipocralOutput((new cedar::aux::MatData(cv::Mat::zeros(1, 1, CV_32F)))),
        mReadOutTrigger(new cedar::aux::MatData(cv::Mat::zeros(1, 1, CV_32F))),
//        mReciprocalInput(new cedar::aux::MatData(cv::mat::zeros(100,100,CV_32F))),
        mInputSum(new cedar::aux::MatData(cv::Mat::zeros(1, 1, CV_32F))),
        mWeightSizeX(mAssociationDimension->getValue() > 0 ? mAssociationSizes->getValue().at(0) : 1),
        mWeightSizeY(mAssociationDimension->getValue() > 1 ? mAssociationSizes->getValue().at(1) : 1)
{

  // declare all data
  cedar::proc::DataSlotPtr assoInput = this->declareInput(mAssoInputName, false);
  this->declareInputCollection(mRewardInputName);
  cedar::proc::DataSlotPtr readOutInput = this->declareInput(mReadOutInputName, false);
  auto weightOutput = this->declareOutput(mOutputName, mConnectionWeights);
  weightOutput->setSerializable(true);
  auto weightTriggerOutput = this->declareOutput(mTriggerOutputName, mWeightOutput);

//  cedar::proc::DataSlotPtr reciprocInput = this->declareInput(mReciprocalInputName, true);
  auto reciprocOutput = this->declareOutput(mReciprocalOutputName,mRecipocralOutput);

  this->mConnectionWeights->getData() = initializeWeightMatrix();
  mWeightOutput->setData(mConnectionWeights->getData());

  mWeightAmplitude->setConstant(!mSetWeights->getValue());
  mWeightSigmas->setConstant(!mSetWeights->getValue());
  mWeightCenters->setConstant(!mSetWeights->getValue());

  this->registerFunction("reset Weights", boost::bind(&RewardHebbTrace::resetWeights, this), false);
//  this->registerFunction("create OutputField", boost::bind(&RewardHebbTrace::createFieldRecruit, this), false);

  QObject::connect(mAssociationDimension.get(), SIGNAL(valueChanged()), this, SLOT(updateAssociationDimension()));
  QObject::connect(mAssociationSizes.get(), SIGNAL(valueChanged()), this, SLOT(resetWeights()));
  QObject::connect(mUseRewardDuration.get(), SIGNAL(valueChanged()), this, SLOT(toggleUseReward()));
  QObject::connect(mSetWeights.get(), SIGNAL(valueChanged()), this, SLOT(toggleUseManualWeights()));
  QObject::connect(mWeightCenters.get(), SIGNAL(valueChanged()), this, SLOT(resetWeights()));
  QObject::connect(mWeightSigmas.get(), SIGNAL(valueChanged()), this, SLOT(resetWeights()));
  QObject::connect(mWeightAmplitude.get(), SIGNAL(valueChanged()), this, SLOT(resetWeights()));
}
//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

void cedar::proc::steps::RewardHebbTrace::updateAssociationDimension()
{
  int new_dim = static_cast<int>(mAssociationDimension->getValue());
  mAssociationSizes->resize(new_dim, mAssociationSizes->getDefaultValue());
  mWeightCenters->resize(new_dim, mWeightCenters->getDefaultValue());
  mWeightSigmas->resize(new_dim, mWeightCenters->getDefaultValue());
  mWeightSizeX = mAssociationDimension->getValue() > 0 ? mAssociationSizes->getValue().at(0) : 1;
  mWeightSizeY = mAssociationDimension->getValue() > 1 ? mAssociationSizes->getValue().at(1) : 1;
  this->resetWeights();
}

cv::Mat cedar::proc::steps::RewardHebbTrace::initializeWeightMatrix()
{
  mWeightSizeX = mAssociationDimension->getValue() > 0 ? mAssociationSizes->getValue().at(0) : 1;
  mWeightSizeY = mAssociationDimension->getValue() > 1 ? mAssociationSizes->getValue().at(1) : 1;
  cv::Mat myWeightMat = cv::Mat::zeros(mWeightSizeX, mWeightSizeY, CV_32F);
  if (!mSetWeights->getValue())
  {
//    std::cout<<"InitWeights: RANDOM!"<<std::endl;
    srand(static_cast<unsigned>(time(0)));
    float HIGH = 0.01;
    float LOW = 0;
    for (unsigned int x = 0; x < mWeightSizeX; x++)
    {
      for (unsigned int y = 0; y < mWeightSizeY; y++)
      {
        float random = LOW + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (HIGH - LOW)));
        myWeightMat.at<float>(x, y) = random;
      }
    }
  } else
  {
//    std::cout<<"InitWeights: MANUAL"<<std::endl;
    std::vector<unsigned int> curSizes;
    curSizes.push_back(mWeightSizeX);
    curSizes.push_back(mWeightSizeY);
    myWeightMat = cedar::aux::math::gaussMatrix(mAssociationDimension->getValue(), curSizes,
                                                mWeightAmplitude->getValue(), mWeightSigmas->getValue(),
                                                mWeightCenters->getValue(), true);
  }
  return myWeightMat;
}

void cedar::proc::steps::RewardHebbTrace::eulerStep(const cedar::unit::Time &time)
{
  if (mAssoInput && mInputSum)
  {
    cv::Mat &trigger = this->mInputSum->getData();
    cedar::proc::steps::Sum::sumSlot(this->getInputSlot(mRewardInputName), this->mInputSum->getData(), false);
    if (trigger.at<float>(0, 0) > mRewardThreshold->getValue())
    {
//      std::cout << "Surpass the trigger!" <<std::endl;
//      this->createFieldRecruit();
      if (!mIsRewarded && mUseRewardDuration->getValue())
      {
        mElapsedTime = 0;
        mIsRewarded = true;
      }
      float curTime = time / cedar::unit::Time(1 * cedar::unit::milli * cedar::unit::seconds);
      mElapsedTime += curTime;
      //The Learning Rule is only applied for a fixed time TODO:THINK About This
      if (mElapsedTime < mRewardDuration->getValue() || !mUseRewardDuration->getValue())
      {
        //Apply Learning Rule
        double learnRate = mLearnRatePositive->getValue();
        cv::Mat currentWeights = mConnectionWeights->getData();
        cv::Mat currentAssoMat = mAssoInput->getData();
        for (unsigned int x = 0; x < mWeightSizeX; x++)
        {
          for (unsigned int y = 0; y < mWeightSizeY; y++)
          {
            float weightChange = learnRate * (currentAssoMat.at<float>(x, y) - currentWeights.at<float>(x, y));
            currentWeights.at<float>(x, y) = currentWeights.at<float>(x, y) + weightChange;
          }
        }
        mConnectionWeights->setData(currentWeights);
      }
    }
    else if (mUseRewardDuration->getValue())
    {
      mIsRewarded = false;
    }
  }

  if(mAssoInput)
  {
    //Calculate the reciprocal Output
    //Somehow match the current Input with the learned Weights
    cv::Mat currentAssoMat = mAssoInput->getData();
    cv::Mat currentWeights = mConnectionWeights->getData();

    cv::Mat compMultiplication = currentAssoMat.mul(currentWeights);
//    Because right now the data will always be in 0s or 1s, we need to also threshold the learned weights here. Just to make tuning easier for now. Other way would be to make sure that learned weights
//    stay at a value of 1.
    cedar::aux::math::TransferFunctionPtr transferFunctionPtr = cedar::aux::math::HeavisideSigmoidPtr(new cedar::aux::math::HeavisideSigmoid(mLearnedThreshold->getValue()));
    cv::Mat threshHoldedMultiplication = transferFunctionPtr->compute(compMultiplication);



    float summedMatrix = cv::sum(threshHoldedMultiplication)[0];
    cv::Mat recOutput = cv::Mat::zeros(1,1,CV_32F);
    recOutput.at<float>(0,0) = summedMatrix;

    mRecipocralOutput->setData(recOutput);
  }

  if (mReadOutTrigger)
  {
    const cv::Mat& trigger = mReadOutTrigger->getData();
    if (trigger.at<float>(0, 0) > 0.5) //TODO: This should actually just be calculated using a multiplication wit a sigmoid
    {
      mWeightOutput->setData(mConnectionWeights->getData());
    }
    else
    {
      mWeightOutput->setData(cv::Mat::zeros(mWeightSizeX, mWeightSizeY, CV_32F));
    }
  } else
    mWeightOutput->setData(cv::Mat::zeros(mWeightSizeX, mWeightSizeY, CV_32F));
}

void cedar::proc::steps::RewardHebbTrace::inputConnectionChanged(const std::string &inputName)
{
// Assign the input to the member. This saves us from casting in every computation step.
//  std::cout << "InputConnectionChanged! Inputname: " << inputName << std::endl;
  if (inputName == mAssoInputName)
  {
    this->mAssoInput = boost::dynamic_pointer_cast<const cedar::aux::MatData>(this->getInput(inputName));
  }
  if (inputName == mReadOutInputName)
  {
    this->mReadOutTrigger = boost::dynamic_pointer_cast<const cedar::aux::MatData>(this->getInput(inputName));
  }
//  if (inputName == mReciprocalInputName)
//  {
//    this->mReciprocalInput = boost::dynamic_pointer_cast<const cedar::aux::MatData>(this->getInput(inputName));
//  }
}

cedar::proc::DataSlot::VALIDITY
cedar::proc::steps::RewardHebbTrace::determineInputValidity(cedar::proc::ConstDataSlotPtr slot,
                                                            cedar::aux::ConstDataPtr data) const
{
  if (cedar::aux::ConstMatDataPtr input = boost::dynamic_pointer_cast<const cedar::aux::MatData>(data))
  {
    if (input && input->getDimensionality() == mAssociationDimension->getValue() && slot->getName() == mAssoInputName &&
        (unsigned int) input->getData().rows == mWeightSizeX
        && (unsigned int) input->getData().cols == mWeightSizeY)
    {
      return cedar::proc::DataSlot::VALIDITY_VALID;
    }
    if (input && input->getDimensionality() == 0 &&
        (slot->getName() == mRewardInputName || slot->getName() == mReadOutInputName))
    {
      return cedar::proc::DataSlot::VALIDITY_VALID;
    }

  }
  return cedar::proc::DataSlot::VALIDITY_ERROR;
}

void cedar::proc::steps::RewardHebbTrace::reset()
{
}

void cedar::proc::steps::RewardHebbTrace::resetWeights()
{
  this->mConnectionWeights->setData(initializeWeightMatrix());
  this->mWeightOutput->setData(mConnectionWeights->getData());
  this->emitOutputPropertiesChangedSignal(mOutputName);
  this->emitOutputPropertiesChangedSignal(mTriggerOutputName);
  this->revalidateInputSlot(mAssoInputName);
}

void cedar::proc::steps::RewardHebbTrace::toggleUseReward()
{
//  this->createFieldRecruit();
  mRewardDuration->setConstant(!mUseRewardDuration->getValue());
}

void cedar::proc::steps::RewardHebbTrace::toggleUseManualWeights()
{
  if (mSetWeights->getValue())
  {
    this->resetWeights();
  }
  mWeightAmplitude->setConstant(!mSetWeights->getValue());
  mWeightSigmas->setConstant(!mSetWeights->getValue());
  mWeightCenters->setConstant(!mSetWeights->getValue());
}

//void cedar::proc::steps::RewardHebbTrace::createFieldRecruit()
//{
//  std::cout << "Emit the Recruitment Signal: " << this->getName() << std::endl;
//  emit evokeFieldRecruitment(boost::dynamic_pointer_cast<cedar::proc::steps::RewardHebbTrace>(shared_from_this()));
//}

unsigned int cedar::proc::steps::RewardHebbTrace::getDimensionality()
{
  return mAssociationDimension->getValue();
}

cv::Mat cedar::proc::steps::RewardHebbTrace::getSizes()
{
  cv::Mat sizes = cv::Mat::zeros(2, 1, CV_32F);
  sizes.at<float>(0, 0) = mWeightSizeX;
  sizes.at<float>(1, 0) = mWeightSizeY;
  return sizes;
}

std::string cedar::proc::steps::RewardHebbTrace::getOutputName()
{
  return mOutputName;
}

std::string cedar::proc::steps::RewardHebbTrace::getAssoInputName()
{
  return mAssoInputName;
}

std::string cedar::proc::steps::RewardHebbTrace::getRewardInputName()
{
  return mRewardInputName;
}

std::string cedar::proc::steps::RewardHebbTrace::getReadOutInputName()
{
  return mReadOutInputName;
}

std::string cedar::proc::steps::RewardHebbTrace::getTriggerOutputName()
{
  return mTriggerOutputName;
}

void cedar::proc::steps::RewardHebbTrace::setDimensionality(unsigned int dim)
{
  mAssociationDimension->setValue(dim);
}

void cedar::proc::steps::RewardHebbTrace::setSize(unsigned int dim, unsigned int size)
{
  if (dim < mAssociationSizes->size())
    mAssociationSizes->setValue(dim, size);
}

void cedar::proc::steps::RewardHebbTrace::setWeights(cv::Mat newWeights)
{
  this->mConnectionWeights->setData(newWeights);
  this->mWeightOutput->setData(newWeights);
}
