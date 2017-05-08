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

 File:        2DTo2DHebbMap.cpp

 Maintainer:  Jan Tekülve
 Email:       jan.tekuelve@ini.rub.de
 Date:        2016 10 28

 Description: Source file for the class cedar::proc::steps::2DTo2DHebbMap.

 Credits:

 ======================================================================================================================*/

// CEDAR CONFIGURATION
#include "cedar/configuration.h"

// CLASS HEADER
#include "steps/utilities/FourDimHebbMap.h"

#include "cedar/processing/typecheck/IsMatrix.h"
#include "cedar/processing/DataSlot.h"
#include "cedar/processing/ElementDeclaration.h"
#include "cedar/processing/DeclarationRegistry.h"
#include "cedar/auxiliaries/assert.h"
#include "cedar/auxiliaries/exceptions.h"
#include "cedar/units/Time.h"
#include "cedar/units/prefixes.h"
#include "cedar/processing/steps/Sum.h"

// SYSTEM INCLUDES
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------
cedar::proc::steps::FourDimHebbMap::FourDimHebbMap()
        :
// parameters
//        mAssociationDimension(new cedar::aux::UIntParameter(this, "association dimension", 2, 0, 2)),
        mAssociationSizes(new cedar::aux::UIntVectorParameter(this, "association sizes", 2, 50)),
        mLearnRatePositive(new cedar::aux::DoubleParameter(this, "learning rate", 0.001)),
        mUseRewardDuration(new cedar::aux::BoolParameter(this, "fixed reward duration", false)),
        mRewardDuration(new cedar::aux::DoubleParameter(this, "reward duration", 200)),
//        mSetWeights(new cedar::aux::BoolParameter(this, "manual weights", false)),
//        mWeightCenters(
//                new cedar::aux::DoubleVectorParameter(this, "weight centers", mAssociationDimension->getValue(), 3)),
//        mWeightSigmas(
//                new cedar::aux::DoubleVectorParameter(this, "weight sigmas", mAssociationDimension->getValue(), 3)),
//        mWeightAmplitude(new cedar::aux::DoubleParameter(this, "weight amplitude", 6)),
        mRewardThreshold(new cedar::aux::DoubleParameter(this, "reward threshold", 0.5)),
        mCueThreshold(new cedar::aux::DoubleParameter(this, "cue overlap threshold", 0.1)),
        // outputs
//        mConnectionWeights(new cedar::aux::MatData(cv::Mat::zeros(100, 100, CV_32F))),
        mWeightOutput((new cedar::aux::MatData(
                cv::Mat::zeros(mAssociationSizes->getValue().at(0), mAssociationSizes->getValue().at(1), CV_32F)))),
        mCueOutput((new cedar::aux::MatData(
                cv::Mat::zeros(mAssociationSizes->getValue().at(0), mAssociationSizes->getValue().at(1), CV_32F)))),
        mReadOutTrigger(new cedar::aux::MatData(
                cv::Mat::zeros(mAssociationSizes->getValue().at(0), mAssociationSizes->getValue().at(1), CV_32F))),
        mCueInput(new cedar::aux::MatData(
                cv::Mat::zeros(mAssociationSizes->getValue().at(0), mAssociationSizes->getValue().at(1), CV_32F))),
        mInputSum(new cedar::aux::MatData(cv::Mat::zeros(1, 1, CV_32F))),
        mWeightSizeX(mAssociationSizes->getValue().at(0)),
        mWeightSizeY(mAssociationSizes->getValue().at(1)),
        mMatrixOfOnes(cv::Mat(mWeightSizeX, mWeightSizeY, CV_32F, 1.0f))
{

  // declare all data
  cedar::proc::DataSlotPtr assoInput = this->declareInput(mAssoInputName, false);
  this->declareInputCollection(mRewardInputName);
  cedar::proc::DataSlotPtr readOutInput = this->declareInput(mReadOutInputName, false);
  cedar::proc::DataSlotPtr cueInput = this->declareInput(mCueInputName, false);


  auto weightTriggerOutput = this->declareOutput(mTriggerOutputName, mWeightOutput);
  auto cueOutput = this->declareOutput(mCueOutputName, mCueOutput);
//  auto weightOutput = this->declareOutput(mOutputName, mConnectionWeights);
//  weightOutput->setSerializable(true);

//  this->mConnectionWeights->getData() = initializeWeightMatrix();
//  mWeightOutput->setData(mConnectionWeights->getData());

//  mWeightAmplitude->setConstant(!mSetWeights->getValue());
//  mWeightSigmas->setConstant(!mSetWeights->getValue());
//  mWeightCenters->setConstant(!mSetWeights->getValue());

  this->registerFunction("reset Weights", boost::bind(&FourDimHebbMap::resetWeights, this), false);
//  this->registerFunction("create OutputField", boost::bind(&FourDimHebbMap::createFieldRecruit, this), false);

//  QObject::connect(mAssociationDimension.get(), SIGNAL(valueChanged()), this, SLOT(updateAssociationDimension()));
  QObject::connect(mAssociationSizes.get(), SIGNAL(valueChanged()), this, SLOT(resetWeights()));
  QObject::connect(mUseRewardDuration.get(), SIGNAL(valueChanged()), this, SLOT(toggleUseReward()));
//  QObject::connect(mSetWeights.get(), SIGNAL(valueChanged()), this, SLOT(toggleUseManualWeights()));
//  QObject::connect(mWeightCenters.get(), SIGNAL(valueChanged()), this, SLOT(resetWeights()));
//  QObject::connect(mWeightSigmas.get(), SIGNAL(valueChanged()), this, SLOT(resetWeights()));
//  QObject::connect(mWeightAmplitude.get(), SIGNAL(valueChanged()), this, SLOT(resetWeights()));
}
//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

//void cedar::proc::steps::FourDimHebbMap::updateAssociationDimension()
//{
//  int new_dim = static_cast<int>(mAssociationDimension->getValue());
//  mAssociationSizes->resize(new_dim, mAssociationSizes->getDefaultValue());
//  mWeightCenters->resize(new_dim, mWeightCenters->getDefaultValue());
//  mWeightSigmas->resize(new_dim, mWeightCenters->getDefaultValue());
//  mWeightSizeX = mAssociationDimension->getValue() > 0 ? mAssociationSizes->getValue().at(0) : 1;
//  mWeightSizeY = mAssociationDimension->getValue() > 1 ? mAssociationSizes->getValue().at(1) : 1;
//  this->resetWeights();
//}

void cedar::proc::steps::FourDimHebbMap::initializeWeightMatrix()
{
  mWeightSizeX = mAssociationSizes->getValue().at(0);
  mWeightSizeY = mAssociationSizes->getValue().at(1);
  mMatrixOfOnes = cv::Mat(mWeightSizeX, mWeightSizeY, CV_32F, 1.0f);

  //Add Weights only dynamically!
//  mWeights.clear();
  mWeights.resize(mWeightSizeX);
  for (unsigned int i = 0; i < mWeightSizeX; i++)
  {
    mWeights.at(i).resize(mWeightSizeY);
    for (unsigned int j = 0; j < mWeightSizeY; j++)
    {
      cv::Mat myWeightMat = cv::Mat::zeros(mWeightSizeX, mWeightSizeY, CV_32F);
//      srand(static_cast<unsigned>(time(0)));
//      float HIGH = 0.1;
//      float LOW = 0;
//      for (unsigned int x = 0; x < mWeightSizeX; x++)
//      {
//        for (unsigned int y = 0; y < mWeightSizeY; y++)
//        {
//          float random = LOW + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (HIGH - LOW)));
//          myWeightMat.at<float>(x, y) = random;
//        }
//      }
      mWeights.at(i).at(j) = myWeightMat;
    }
  }
}

cv::Mat cedar::proc::steps::FourDimHebbMap::retrieveWeightedOutput()
{
  cv::Mat output = cv::Mat::zeros(mWeightSizeX, mWeightSizeY, CV_32F);
  if (mReadOutTrigger)
  {
    const cv::Mat &inputMap = mReadOutTrigger->getData();
    for (int x = 0; x < inputMap.rows; x++)
    {
      for (int y = 0; y < inputMap.cols; y++)
      {
        if (inputMap.at<float>(x, y) > 0.5)
        {
          output += mWeights.at(x).at(y);
        }
      }
    }
  }
  //Todo:: Maybe Normalize here!
  return output;
}

cv::Mat cedar::proc::steps::FourDimHebbMap::retrieveCuedOutput()
{
  cv::Mat output = cv::Mat::zeros(mWeightSizeX, mWeightSizeY, CV_32F);
  if (mCueInput)
  {
    const cv::Mat &inputCue = mCueInput->getData();
    if ((unsigned int)inputCue.rows ==  mWeightSizeX && (unsigned int)inputCue.cols ==  mWeightSizeY)
    {
      double maxCueVal;
      cv::minMaxLoc(inputCue, nullptr, &maxCueVal);
      //Brute Force Variant
//    //Store Cue Indices in a LookUpTable, what about cv::for_each_operator?
//    std::vector<QPoint> relevantIndices;
//    for (int x = 0; x < inputCue.rows; x++)
//    {
//      for (int y = 0; y < inputCue.cols; y++)
//      {
//        if (inputCue.at<float>(x, y) > 0.5)
//        {
//          relevantIndices.push_back(QPoint(x,y));
//        }
//      }
//    }
//    std::cout << "InputCue: "<< inputCue.rows << "," << inputCue.cols <<std::endl;
//    std::cout << "MatchMatrix0: "<< mWeights.at(0).at(0).rows << "," << mWeights.at(0).at(0).cols <<std::endl;

      for (unsigned int i = 0; i < mWeightSizeX; i++)
      {
        for (unsigned int j = 0; j < mWeightSizeY; j++)
        {
          cv::Mat matchMat = mWeights.at(i).at(j);
//          cv::Mat multMat = inputCue.mul(matchMat);
          cv::Mat addMat = inputCue+matchMat;
          double maxValue = std::numeric_limits<double>::min();
          double minVal;
          cv::minMaxLoc(addMat, &minVal, &maxValue);
//          std::cout<<"MaxValue = " << maxValue<<std::endl;
          if (maxValue > maxCueVal + mCueThreshold->getValue())
          {
            output.at<float>(i, j) = maxValue;
          }
          //Does any part overlap with the cue? Then we can add the current i,j Position to our OutputMatrix
//        for(unsigned int k = 0 ; k< relevantIndices.size();k++)
//        {
//
//          QPoint curPoint = relevantIndices.at(k);
//          std::cout<<"Indice k= " << k << " at " << curPoint.x() <<","<<curPoint.y()<<" Matrix Value: "<< MatchMat.at<float>(curPoint.x(),curPoint.y())<<std::endl;
//
//          if(MatchMat.at<float>(curPoint.x(),curPoint.y()) > mCueThreshold->getValue() )
//          {
//            output.at<float>(i,j) = 1;
//            break;
//          }
//        }
        }
      }
    }
  }
  //Todo:: Maybe Normalize here!
  return output;
}

void cedar::proc::steps::FourDimHebbMap::eulerStep(const cedar::unit::Time &time)
{
  if (mAssoInput && mInputSum && mReadOutTrigger)
  {
    cv::Mat &rewardTrigger = this->mInputSum->getData();
    cedar::proc::steps::Sum::sumSlot(this->getInputSlot(mRewardInputName), this->mInputSum->getData(), false);
    if (rewardTrigger.at<float>(0, 0) > mRewardThreshold->getValue())
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
      if (mElapsedTime < mRewardDuration->getValue() || !mUseRewardDuration->getValue())
      {
        //Apply Learning Rule
        double learnRate = mLearnRatePositive->getValue();
        cv::Mat currentAssoMat = mAssoInput->getData();
        cv::Mat currentInputMap = mReadOutTrigger->getData();

        for (unsigned int x = 0; x < mWeightSizeX; x++)
        {
          for (unsigned int y = 0; y < mWeightSizeY; y++)
          {
            //Hebbian Approach
//               mWeights.at(x).at(y) = mWeights.at(x).at(y) + currentInputMap.at<float>(x, y)* learnRate * (currentAssoMat -mWeights.at(x).at(y));
            //Preshape Approach without decay
            cv::Mat normalizeMat = mWeights.at(x).at(y) + currentInputMap.at<float>(x, y) * learnRate * currentAssoMat;
            cv::min(normalizeMat, mMatrixOfOnes, normalizeMat); // Ensure that no MatrixValue surpasses One
            mWeights.at(x).at(y) = normalizeMat;
//            mWeights.at(x).at(y) = mWeights.at(x).at(y) + currentInputMap.at<float>(x, y)* learnRate * currentAssoMat;
          }
        }
      }
    } else if (mUseRewardDuration->getValue())
    {
      mIsRewarded = false;
    }
  }
  this->mWeightOutput->setData(retrieveWeightedOutput());
  this->mCueOutput->setData(retrieveCuedOutput());
}

void cedar::proc::steps::FourDimHebbMap::inputConnectionChanged(const std::string &inputName)
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
  if (inputName == mCueInputName)
  {
    this->mCueInput = boost::dynamic_pointer_cast<const cedar::aux::MatData>(this->getInput(inputName));
  }
}

cedar::proc::DataSlot::VALIDITY
cedar::proc::steps::FourDimHebbMap::determineInputValidity(cedar::proc::ConstDataSlotPtr slot,
                                                           cedar::aux::ConstDataPtr data) const
{
  if (cedar::aux::ConstMatDataPtr input = boost::dynamic_pointer_cast<const cedar::aux::MatData>(data))
  {
    if (input && input->getDimensionality() == 2 &&
        (slot->getName() == mAssoInputName || slot->getName() == mReadOutInputName ||
         slot->getName() == mCueInputName) &&
        (unsigned int) input->getData().rows == mWeightSizeX
        && (unsigned int) input->getData().cols == mWeightSizeY)
    {
      return cedar::proc::DataSlot::VALIDITY_VALID;
    }
    if (input && input->getDimensionality() == 0 &&
        slot->getName() == mRewardInputName)
    {
      return cedar::proc::DataSlot::VALIDITY_VALID;
    }

  }
  return cedar::proc::DataSlot::VALIDITY_ERROR;
}

void cedar::proc::steps::FourDimHebbMap::reset()
{
}

void cedar::proc::steps::FourDimHebbMap::resetWeights()
{
  initializeWeightMatrix();
  this->mWeightOutput->setData(retrieveWeightedOutput());
  this->mCueOutput->setData(retrieveCuedOutput());
  this->emitOutputPropertiesChangedSignal(mTriggerOutputName);
  this->revalidateInputSlot(mAssoInputName);
}

void cedar::proc::steps::FourDimHebbMap::toggleUseReward()
{
//  this->createFieldRecruit();
  mRewardDuration->setConstant(!mUseRewardDuration->getValue());
}

//void cedar::proc::steps::FourDimHebbMap::toggleUseManualWeights()
//{
//  if (mSetWeights->getValue())
//  {
//    this->resetWeights();
//  }
//  mWeightAmplitude->setConstant(!mSetWeights->getValue());
//  mWeightSigmas->setConstant(!mSetWeights->getValue());
//  mWeightCenters->setConstant(!mSetWeights->getValue());
//}

//void cedar::proc::steps::FourDimHebbMap::createFieldRecruit()
//{
//  std::cout << "Emit the Recruitment Signal: " << this->getName() << std::endl;
//  emit evokeFieldRecruitment(boost::dynamic_pointer_cast<cedar::proc::steps::FourDimHebbMap>(shared_from_this()));
//}

//unsigned int cedar::proc::steps::FourDimHebbMap::getDimensionality()
//{
//  return mAssociationDimension->getValue();
//}

cv::Mat cedar::proc::steps::FourDimHebbMap::getSizes()
{
  cv::Mat sizes = cv::Mat::zeros(2, 1, CV_32F);
  sizes.at<float>(0, 0) = mWeightSizeX;
  sizes.at<float>(1, 0) = mWeightSizeY;
  return sizes;
}

std::string cedar::proc::steps::FourDimHebbMap::getCueOutputName()
{
  return mCueOutputName;
}

std::string cedar::proc::steps::FourDimHebbMap::getAssoInputName()
{
  return mAssoInputName;
}

std::string cedar::proc::steps::FourDimHebbMap::getRewardInputName()
{
  return mRewardInputName;
}

std::string cedar::proc::steps::FourDimHebbMap::getReadOutInputName()
{
  return mReadOutInputName;
}

std::string cedar::proc::steps::FourDimHebbMap::getTriggerOutputName()
{
  return mTriggerOutputName;
}

//void cedar::proc::steps::FourDimHebbMap::setDimensionality(unsigned int dim)
//{
//  mAssociationDimension->setValue(dim);
//}

void cedar::proc::steps::FourDimHebbMap::setSize(unsigned int dim, unsigned int size)
{
  if (dim < mAssociationSizes->size())
    mAssociationSizes->setValue(dim, size);
}

//This is stolen from the NormalizationStep!
double cedar::proc::steps::FourDimHebbMap::safeNormInverse(double norm) const
{
  if (isinf(norm))
  {
    return 0.0;
  }

  if (isnan(norm))
  {
    return 1.0;
  }

  if (std::abs(norm) < std::numeric_limits<float>::min())
  {
    return 1.0;
  }

  CEDAR_DEBUG_NON_CRITICAL_ASSERT(!isnan(1.0 / norm) && !isinf(1.0 / norm));
  return 1.0 / norm;
}

//void cedar::proc::steps::FourDimHebbMap::setWeights(cv::Mat newWeights)
//{
//  this->mConnectionWeights->setData(newWeights);
//  this->mWeightOutput->setData(newWeights);
//}
