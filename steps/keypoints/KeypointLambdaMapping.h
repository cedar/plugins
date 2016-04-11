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

    File:        KeypointLambdaMapping.h

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2016 04 11

    Description: Header file for the class cedar::proc::steps::KeypointLambdaMapping.

    Credits:

======================================================================================================================*/

#ifndef CEDAR_PROC_STEPS_KEYPOINT_LAMBDA_MAPPING_H
#define CEDAR_PROC_STEPS_KEYPOINT_LAMBDA_MAPPING_H

// CEDAR CONFIGURATION
#include "cedar/configuration.h"

// CEDAR INCLUDES
#include <cedar/processing/Step.h>
#include <cedar/processing/InputSlotHelper.h>
#include <cedar/auxiliaries/EnumParameter.h>
#include <cedar/auxiliaries/DoubleParameter.h>
#include <cedar/auxiliaries/UIntParameter.h>
#include <cedar/auxiliaries/MatData.h>

// FORWARD DECLARATIONS
#include "steps/keypoints/KeypointLambdaMapping.fwd.h"

// SYSTEM INCLUDES
#include <vector>


/*!@brief Maps activation defined over a lambda dimension to a log polar dimension and back.
 */
class cedar::proc::steps::KeypointLambdaMapping : public cedar::proc::Step
{
  Q_OBJECT

  //--------------------------------------------------------------------------------------------------------------------
  // nested types
  //--------------------------------------------------------------------------------------------------------------------
  class DimensionType
  {
    public:
      //! Type of the enum.
      typedef cedar::aux::EnumId Id;
      //! Pointer to the enumeration type.
      typedef boost::shared_ptr<cedar::aux::EnumBase> TypePtr;

      //! Constructs the enumeration values.
      static void construct()
      {
        mType.type()->def(cedar::aux::Enum(LogPolarShift, "LogPolarShift", "Log-Polar Shift"));
        mType.type()->def(cedar::aux::Enum(KeypointLambdaLinear, "KeypointLambdaLinear", "Linear Keypoint Lambdas"));
      }

      //! Returns the enum base class.
      static const cedar::aux::EnumBase& type()
      {
        return *mType.type();
      }

      //! Returns a pointer to the enum base class.
      static const TypePtr& typePtr()
      {
        return mType.type();
      }

    public:
      static constexpr int LogPolarShift = 0;
      static constexpr int KeypointLambdaLinear = 1;

    private:
      static cedar::aux::EnumType<cedar::proc::steps::KeypointLambdaMapping::DimensionType> mType;
  };

  //--------------------------------------------------------------------------------------------------------------------
  // constructors and destructor
  //--------------------------------------------------------------------------------------------------------------------
public:
  //!@brief The standard constructor.
  KeypointLambdaMapping();

  //--------------------------------------------------------------------------------------------------------------------
  // public methods
  //--------------------------------------------------------------------------------------------------------------------
public:
  // none yet

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

  void compute(const cedar::proc::Arguments& arguments);

  void updateMapping();

  double getSourceScaleFactorForIndex(unsigned int index) const;

  double getTargetScaleFactorForIndex(unsigned int index) const;

  double getScaleFactorForIndex(DimensionType::Id dimension, unsigned int index, unsigned int arraySize) const;

  double getLambdaScaleForIndex(unsigned int index) const;

  void allocateOutput();

private slots:
  void sourceDimensionChanged();
  void targetDimensionChanged();

  //--------------------------------------------------------------------------------------------------------------------
  // members
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet
private:
  //!@brief MatrixData representing the input. Storing it like this saves time during computation.
  cedar::proc::InputSlotHelper<cedar::aux::MatData> mInput;

  //!@brief The output data.
  cedar::aux::MatDataPtr mOutput;

  std::vector<int> mCoordinateMapping;

  std::vector<float> mMappingAcuity;

  //--------------------------------------------------------------------------------------------------------------------
  // parameters
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

private:
  cedar::aux::EnumParameterPtr _mSourceDimension;

  cedar::aux::EnumParameterPtr _mTargetDimension;

  cedar::aux::DoubleParameterPtr _mLambdasStart;

  cedar::aux::DoubleParameterPtr _mLambdasEnd;

  cedar::aux::UIntParameterPtr _mNumberOfLambdas;

  cedar::aux::UIntParameterPtr _mTrainingLambdaIndex;

  cedar::aux::DoubleParameterPtr _mMagnitudeForward;

  cedar::aux::UIntParameterPtr _mLogPolarShiftFieldSize;

}; // class cedar::proc::steps::KeypointLambdaMapping

#endif // CEDAR_PROC_STEPS_KEYPOINT_LAMBDA_MAPPING_H

