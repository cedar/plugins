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

    File:        RewardHebbTrace.h

    Maintainer:  Jan Tekülve
    Email:       jan.tekuelve@ini.rub.de
    Date:        2016 10 28

    Description: Header file for the class promoplugin::ImprintHebb.

    Credits:

======================================================================================================================*/

#ifndef PROMOPLUGIN_REWARD_HEBB_TRACE_H
#define PROMOPLUGIN_REWARD_HEBB_TRACE_H

// CEDAR INCLUDES
#include "cedar/dynamics/Dynamics.h"
#include "cedar/auxiliaries/MatData.h"
#include "cedar/auxiliaries/DoubleParameter.h"
#include "cedar/auxiliaries/DoubleVectorParameter.h"
#include "cedar/auxiliaries/UIntParameter.h"
#include "cedar/auxiliaries/UIntVectorParameter.h"
#include "cedar/auxiliaries/BoolParameter.h"
#include "cedar/auxiliaries/math/functions.h"

#include "steps/utilities/RewardHebbTrace.fwd.h"

// SYSTEM INCLUDES

class cedar::proc::steps::RewardHebbTrace : public cedar::dyn::Dynamics
{
  //--------------------------------------------------------------------------------------------------------------------
  // macros
  //--------------------------------------------------------------------------------------------------------------------
  Q_OBJECT
  //--------------------------------------------------------------------------------------------------------------------
  // constructors and destructor
  //--------------------------------------------------------------------------------------------------------------------
public:
  //!@brief The standard constructor.
  RewardHebbTrace();


  //--------------------------------------------------------------------------------------------------------------------
  // public methods
  //--------------------------------------------------------------------------------------------------------------------
public:
  unsigned int getDimensionality();
  cv::Mat getSizes();
  std::string getOutputName();
  std::string getAssoInputName();
  std::string getRewardInputName();
  std::string getReadOutInputName();
  std::string getTriggerOutputName();
  void setDimensionality(unsigned int dim);
  void setSize(unsigned int dim, unsigned int size);
  void setWeights(cv::Mat newWeights);

public slots:
 void updateAssociationDimension();
 void resetWeights();
 void toggleUseReward();
 void toggleUseManualWeights();

// signals:
//   void evokeFieldRecruitment(cedar::proc::steps::RewardHebbTracePtr);
  //--------------------------------------------------------------------------------------------------------------------
  // protected methods
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

  //--------------------------------------------------------------------------------------------------------------------
  // private methods
  //--------------------------------------------------------------------------------------------------------------------
private:
  //!@brief Reacts to a change in the input connection.
  void inputConnectionChanged(const std::string& inputName);

  cv::Mat initializeWeightMatrix();

  //!@brief Updates the output matrix.
  void eulerStep(const cedar::unit::Time& time);

  cedar::proc::DataSlot::VALIDITY determineInputValidity(
      cedar::proc::ConstDataSlotPtr slot,
      cedar::aux::ConstDataPtr data) const;

  void reset();

//  void createFieldRecruit();

  //--------------------------------------------------------------------------------------------------------------------
  // parameters
  //--------------------------------------------------------------------------------------------------------------------
protected:
  //!@brief The factor by which the input is multiplied.
  cedar::aux::UIntParameterPtr mAssociationDimension;
  cedar::aux::UIntVectorParameterPtr mAssociationSizes;
  cedar::aux::DoubleParameterPtr mLearnRatePositive;
  cedar::aux::BoolParameterPtr mUseRewardDuration;
  cedar::aux::DoubleParameterPtr mRewardDuration;
  cedar::aux::BoolParameterPtr mSetWeights;
  cedar::aux::DoubleVectorParameterPtr mWeightCenters;
  cedar::aux::DoubleVectorParameterPtr mWeightSigmas;
  cedar::aux::DoubleParameterPtr mWeightAmplitude;
  cedar::aux::DoubleParameterPtr mRewardThreshold;
  cedar::aux::DoubleParameterPtr mLearnedThreshold;
  cedar::aux::BoolParameterPtr mRequireAssoAndTrigger;

private:

  //--------------------------------------------------------------------------------------------------------------------
  // members
  //--------------------------------------------------------------------------------------------------------------------
protected:
  //!@brief The data containing the output.
  cedar::aux::MatDataPtr mConnectionWeights;
  cedar::aux::MatDataPtr mWeightOutput;
  cedar::aux::MatDataPtr mRecipocralOutput;
private:
  std::string mAssoInputName = "association input";
  std::string mOutputName = "weights";
  std::string mTriggerOutputName = "trigger output";
  std::string mRewardInputName = "reward signal";
  std::string mReadOutInputName= "read out trigger";
//  std::string mReciprocalInputName = "reciprocal input";
  std::string mReciprocalOutputName = "reciprocal output";
  bool mIsRewarded = false;
  int mElapsedTime = 0;

  cedar::aux::ConstMatDataPtr mAssoInput;
  cedar::aux::ConstMatDataPtr mReadOutTrigger;
//  cedar::aux::ConstMatDataPtr mReciprocalInput;
  cedar::aux::MatDataPtr mInputSum;
  unsigned int mWeightSizeX;
  unsigned int mWeightSizeY;
};// class cedar::proc::steps::RewardHebbTrace



#endif // PROMOPLUGIN_REWARD_HEBB_TRACE_H

