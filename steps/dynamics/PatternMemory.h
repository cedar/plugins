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

    File:        PatternMemory.h

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2012 01 26

    Description:

    Credits:

======================================================================================================================*/

#ifndef CEDAR_DYN_STEPS_PATTERN_MEMORY_H
#define CEDAR_DYN_STEPS_PATTERN_MEMORY_H

// LOCAL INCLUDES
#include "steps/dynamics/PatternMemory.fwd.h"

// PROJECT INCLUDES

// SYSTEM INCLUDES
#include <cedar/dynamics/Dynamics.h>
#include <cedar/processing/Step.h>
#include <cedar/auxiliaries/UIntParameter.h>
#include <cedar/auxiliaries/DoubleParameter.h>
#include <cedar/auxiliaries/EnumParameter.h>
#include <cedar/auxiliaries/EnumType.h>
#include <cedar/auxiliaries/MatData.h>


/*!@brief Abstract description of the class.
 */
class cedar::dyn::steps::PatternMemory : public cedar::dyn::Dynamics
{
  //--------------------------------------------------------------------------------------------------------------------
  // macros
  //--------------------------------------------------------------------------------------------------------------------
  Q_OBJECT

  //--------------------------------------------------------------------------------------------------------------------
  // nested types
  //--------------------------------------------------------------------------------------------------------------------
  class WeightInitialization
  {
  public:
    typedef cedar::aux::EnumId Id;
    typedef boost::shared_ptr<cedar::aux::EnumBase> TypePtr;

    static void construct()
    {
      mType.type()->def(cedar::aux::Enum(Legacy, "Legacy", "Legacy"));
      mType.type()->def(cedar::aux::Enum(Gaussians, "Gaussians", "Gaussians"));
      mType.type()->def(cedar::aux::Enum(RandomNormal, "RandomNormal", "Random (normally distributed)"));
    }

    static const cedar::aux::EnumBase& type()
    {
      return *mType.type();
    }

    static const TypePtr& typePtr()
    {
      return mType.type();
    }

    static const Id Legacy = 0;
    static const Id Gaussians = 1;
    static const Id RandomNormal = 2;

  private:
    static cedar::aux::EnumType<WeightInitialization> mType;
  };

  //--------------------------------------------------------------------------------------------------------------------
  // constructors and destructor
  //--------------------------------------------------------------------------------------------------------------------
public:
  //!@brief The standard constructor.
  PatternMemory();

  //--------------------------------------------------------------------------------------------------------------------
  // public methods
  //--------------------------------------------------------------------------------------------------------------------
public:
  inline double getLearningFactor() const
  {
    return this->_mLearningFactor->getValue();
  }

  inline unsigned int getNumLabels() const
  {
    return this->_mNumLabels->getValue();
  }

  inline void setNumberOfLabels(unsigned int numLabels)
  {
    this->_mNumLabels->setValue(numLabels);
  }

  void learnCurrentPattern();

  void saveWeightMatrix(const std::string& path) const;

  void loadWeightMatrix(const std::string& path);

  inline cedar::aux::ConstMatDataPtr getInputPattern() const
  {
    return this->mPattern;
  }

  inline cedar::aux::ConstMatDataPtr getLabelInput() const
  {
    return this->mLabels;
  }

  void initializeWeights();

  //--------------------------------------------------------------------------------------------------------------------
  // protected methods
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

  //--------------------------------------------------------------------------------------------------------------------
  // private methods
  //--------------------------------------------------------------------------------------------------------------------
private:
  void inputConnectionChanged(const std::string& inputName);

  cedar::proc::DataSlot::VALIDITY determineInputValidity
  (
    cedar::proc::ConstDataSlotPtr slot,
    cedar::aux::ConstDataPtr data
  )
  const;

  void eulerStep(const cedar::unit::Time& time);

  int getPatternSize1D();

  void openSaveWeightMatrixDialog();

  void openLoadWeightMatrixDialog();

  unsigned int getDimensionality() const;

  void initializeWeightsLegacy();

  void initializeWeightsRandomNormal();

  void initializeWeightsGaussians();

private slots:
  void updateWeightMatrixSize();

  //--------------------------------------------------------------------------------------------------------------------
  // members
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

private:
  // inputs
  cedar::aux::ConstMatDataPtr mPattern;

  cedar::aux::ConstMatDataPtr mLabels;

  // outputs
  cedar::aux::MatDataPtr mWeights;

  //--------------------------------------------------------------------------------------------------------------------
  // parameters
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

private:
  cedar::aux::UIntParameterPtr _mNumLabels;

  cedar::aux::DoubleParameterPtr _mLearningFactor;

  cedar::aux::BoolParameterPtr _mDisableDecay;

  cedar::aux::EnumParameterPtr _mWeightInitializationMethod;

}; // class cedar::dyn::steps::PatternMemory

#endif // CEDAR_DYN_STEPS_PATTERN_MEMORY_H
