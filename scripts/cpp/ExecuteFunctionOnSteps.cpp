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

 File:        RecruitGroup.cpp

 Maintainer:  Jan Tekülve
 Email:       jan.tekuelve@ini.rub.de
 Date:        2016 11 03

 Description: Source file for the class cedar::proc::scripts::RecruitGroup.

 Credits:

 ======================================================================================================================*/

// CEDAR CONFIGURATION
#include "cedar/configuration.h"

// CLASS HEADER
#include "scripts/cpp/ExecuteFunctionOnSteps.h"
#include "steps/utilities/RewardHebbTrace.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
// CEDAR INCLUDES

// SYSTEM INCLUDES

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::aux::EnumType<cedar::proc::scripts::ExecuteFunctionOnSteps::FunctionName> cedar::proc::scripts::ExecuteFunctionOnSteps::FunctionName::mType(
        "FunctionName::"
                                                                                                                          );

cedar::proc::scripts::ExecuteFunctionOnSteps::ExecuteFunctionOnSteps()
        :
        _mFunctionName(
                new cedar::aux::EnumParameter(this, "FunctionName",
                                              cedar::proc::scripts::ExecuteFunctionOnSteps::FunctionName::typePtr(),
                                              FunctionName::ResetWeights
                                             ))
{
}

cedar::proc::scripts::ExecuteFunctionOnSteps::~ExecuteFunctionOnSteps()
{
  std::cout << "Destroy the ExectureFunctionOnSteps Script" << std::endl;
  this->requestStop();
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

void cedar::proc::scripts::ExecuteFunctionOnSteps::run()
{

  switch (_mFunctionName->getValue())
  {
    case FunctionName::ResetWeights :
      this->rewardHebbianTraceResetWeights();
      break;
	case FunctionName::ResetMemory :
      this->preshapeResetMemory();
      break;
    default:
      std::cout << "Function was not specified!" << std::endl;
  }
}

void cedar::proc::scripts::ExecuteFunctionOnSteps::rewardHebbianTraceResetWeights()
{
  std::set<cedar::proc::steps::RewardHebbTracePtr> rewardHebbTraces = this->getGroup()->findAll<cedar::proc::steps::RewardHebbTrace>(
          true);

  for (auto it = rewardHebbTraces.begin(); it != rewardHebbTraces.end(); it++)
  {
    auto hebbTraceStep = *it;
    hebbTraceStep->resetWeights();
  }
}

void cedar::proc::scripts::ExecuteFunctionOnSteps::preshapeResetMemory()
{
  std::set<cedar::dyn::PreshapePtr> preShapes = this->getGroup()->findAll<cedar::dyn::Preshape>(
          true);

  for (auto it = preShapes.begin(); it != preShapes.end(); it++)
  {
    auto preshape = *it;
    preshape->resetMemory();
  }
}
