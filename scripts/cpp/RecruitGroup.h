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
#include <cedar/auxiliaries/IntParameter.h>
#include "cedar/auxiliaries/EnumType.h"
#include "cedar/auxiliaries/EnumParameter.h"
// SYSTEM INCLUDES

/*!This Script should be run once to connect Steps in the Architecture with a recruit function of this script.
 * Then during runtime, whenever the connected steps emit the specified signal, a new group, specified in the appropiate Scriptparameters, should be created.
 * For each recruit Routine you need to add a new Enum, and a new slot function that connects the newly created group in your desired way.
 *
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
//!@brief Enum class for FunctionTypes
  class AddRoutine
  {
  public:
    //! the id of an enum entry
    typedef cedar::aux::EnumId Id;

    //! constructs the enum for all ids
    static void construct()
    {
      mType.type()->def(cedar::aux::Enum(Episode, "EpisodeMemory"));
      mType.type()->def(cedar::aux::Enum(Ordinal, "OrdinalSlot"));
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
    static const Id Episode = 0;
    static const Id Ordinal = 1;

  private:
    static cedar::aux::EnumType<AddRoutine> mType;
  };


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

  void recruitGroupForEpisode(boost::weak_ptr<cedar::dyn::SerialOrderRecruiting> serialOrderStep);
  void recruitGroupForOrdinal(boost::weak_ptr<cedar::dyn::SerialOrderRecruiting> serialOrderStep);

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

  cedar::proc::GroupPtr recruitGroup(std::string containerName);

  cedar::proc::GroupPtr getGroupContainer(std::string name);

  std::string cutImprintName(std::string fullName);

  cedar::proc::ElementPtr AddAGroupFromFile(const std::string &groupName, const cedar::aux::Path &fileName, cedar::proc::GroupPtr containerGroup);

  void registerForEpisode();

  void registerForOrdinal();
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
  cedar::aux::StringParameterPtr _mInnerContainerName;
  cedar::aux::EnumParameterPtr _mAddRoutine;
  cedar::aux::IntParameterPtr _mXOffSet;
  cedar::aux::IntParameterPtr _mYOffSet;
  // none yet

};
// class cedar::proc::scripts::RecruitGroup

#endif // RECRUIT_GROUP_H

