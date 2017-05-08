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

 File:        ExecuteFunctionOnSteps.h

 Maintainer:  Jan Tekülve
 Email:       jan.tekuelve@ini.rub.de
 Date:        2016 11 03

 Description: Header file for the class promoplugin::ExecuteFunctionOnSteps.

 Credits:

 ======================================================================================================================*/

#ifndef EXECUTE_FUNCTION_ON_STEPS_H
#define EXECUTE_FUNCTION_ON_STEPS_H

// CEDAR CONFIGURATION
#include "cedar/configuration.h"

// CEDAR INCLUDES

// FORWARD DECLARATIONS
#include "scripts/cpp/ExecuteFunctionOnSteps.fwd.h"
#include "cedar/processing/CppScript.h"
#include "cedar/processing/Element.h"
#include "cedar/processing/Group.h"
#include "cedar/processing/steps/StaticGain.h"
#include "cedar/dynamics/fields/NeuralField.h"
#include "steps/dynamics/SerialOrderRecruiting.fwd.h"
#include <cedar/auxiliaries/StringParameter.h>
#include <cedar/auxiliaries/IntParameter.h>
#include "cedar/auxiliaries/EnumType.h"
#include "cedar/auxiliaries/EnumParameter.h"
// SYSTEM INCLUDES

/*!This Script should be run once to connect Steps in the Architecture with a recruit function of this script.
 * Then during runtime, whenever the connected steps emit the specified signal, a new group, specified in the appropiate Scriptparameters, should be created.
 * For each recruit Routine you need to add a new Enum, and a new slot function that connects the newly created group in your desired way.
 *
 */
class cedar::proc::scripts::ExecuteFunctionOnSteps : public cedar::proc::CppScript
{
  //--------------------------------------------------------------------------------------------------------------------
  // macros
  //--------------------------------------------------------------------------------------------------------------------
Q_OBJECT

  //--------------------------------------------------------------------------------------------------------------------
  // nested types
  //--------------------------------------------------------------------------------------------------------------------
//!@brief Enum class for FunctionTypes
  class FunctionName
  {
  public:
    //! the id of an enum entry
    typedef cedar::aux::EnumId Id;

    //! constructs the enum for all ids
    static void construct()
    {
      mType.type()->def(cedar::aux::Enum(ResetWeights, "RewardHebbTrace->resetWeights()"));
    }

    //! @returns A const reference to the base enum object.
    static const cedar::aux::EnumBase &type()
    {
      return *(mType.type());
    }

    //! @returns A pointer to the base enum object.
    static const cedar::proc::DataRole::TypePtr &typePtr()
    {
      return mType.type();
    }

  public:
    //! flag for linear weights with a noise term at the outskirts
    static const Id ResetWeights = 0;

  private:
    static cedar::aux::EnumType<FunctionName> mType;
  };


  //--------------------------------------------------------------------------------------------------------------------
  // constructors and destructor
  //--------------------------------------------------------------------------------------------------------------------
public:
  //!@brief The standard constructor.
  ExecuteFunctionOnSteps();

  //!@brief Destructor
  virtual ~ExecuteFunctionOnSteps();

  //--------------------------------------------------------------------------------------------------------------------
  // public methods
  //--------------------------------------------------------------------------------------------------------------------
public:
public slots:


  //--------------------------------------------------------------------------------------------------------------------
  // protected methods
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

  //--------------------------------------------------------------------------------------------------------------------
  // private methods
  //--------------------------------------------------------------------------------------------------------------------
private:
  void run();
  void rewardHebbianTraceResetWeights();
  // none yet

  //--------------------------------------------------------------------------------------------------------------------
  // members
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet
private:
  // none yet

  //--------------------------------------------------------------------------------------------------------------------
  // parameters
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

private:
  cedar::aux::EnumParameterPtr _mFunctionName;
  // none yet

};
// class cedar::proc::scripts::ExecuteFunctionOnSteps

#endif // EXECUTE_FUNCTION_ON_STEPS_H
