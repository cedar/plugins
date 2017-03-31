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

 File:        RecruitGroup.h

 Maintainer:  Jan Tekülve
 Email:       jan.tekuelve@ini.rub.de
 Date:        2016 11 03

 Description: Header file for the class promoplugin::RecruitGroup.

 Credits:

 ======================================================================================================================*/

#ifndef RECRUIT_GROUP_H
#define RECRUIT_GROUP_H

// CEDAR CONFIGURATION
#include "cedar/configuration.h"

// CEDAR INCLUDES

// FORWARD DECLARATIONS
#include "scripts/cpp/RecruitGroup.fwd.h"
#include "cedar/processing/CppScript.h"
#include "cedar/processing/Element.h"
#include "cedar/processing/Group.h"
#include "cedar/processing/steps/StaticGain.h"
#include "cedar/dynamics/fields/NeuralField.h"
#include "steps/dynamics/SerialOrderRecruiting.fwd.h"
#include <cedar/auxiliaries/StringParameter.h>

// SYSTEM INCLUDES

/*!@todo describe.
 *
 * @todo describe more.
 */
class cedar::proc::scripts::RecruitGroup : public cedar::proc::CppScript
{
  //--------------------------------------------------------------------------------------------------------------------
  // macros
  //--------------------------------------------------------------------------------------------------------------------
  Q_OBJECT
  //--------------------------------------------------------------------------------------------------------------------
  // nested types
  //--------------------------------------------------------------------------------------------------------------------

  //--------------------------------------------------------------------------------------------------------------------
  // constructors and destructor
  //--------------------------------------------------------------------------------------------------------------------
public:
  //!@brief The standard constructor.
  RecruitGroup();

  //!@brief Destructor
  virtual ~RecruitGroup();

  //--------------------------------------------------------------------------------------------------------------------
  // public methods
  //--------------------------------------------------------------------------------------------------------------------
public:
public slots:
  void recruitGroup(cedar::dyn::SerialOrderRecruitingPtr serialOrderStep);
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
  void run();
  std::string cutImprintName(std::string fullName);
  cedar::proc::ElementPtr AddAGroupFromFile(const std::string& groupName, const cedar::aux::Path& fileName, cedar::proc::GroupPtr containerGroup);
  void connectGroup(cedar::proc::GroupPtr containerGroup, cedar::proc::GroupPtr toBeIncludedGroup);
  // none yet

  //--------------------------------------------------------------------------------------------------------------------
  // members
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet
private:
  int mCreatedGroupCounter;
  bool ran = false;
  // none yet

  //--------------------------------------------------------------------------------------------------------------------
  // parameters
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

private:
  cedar::aux::StringParameterPtr _mGroupFile;
  cedar::aux::StringParameterPtr _mGroupName;
  cedar::aux::StringParameterPtr _mGroupContainerName;
  // none yet

};
// class cedar::proc::scripts::RecruitGroup

#endif // RECRUIT_GROUP_H

