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
#include "scripts/cpp/RecruitGroup.h"
#include "steps/dynamics/SerialOrderRecruiting.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
// CEDAR INCLUDES

// SYSTEM INCLUDES

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::proc::scripts::RecruitGroup::RecruitGroup()
        :
        mCreatedGroupCounter(0),
        ran(false),
        _mGroupFile(new cedar::aux::StringParameter(this, "path to groupfile", "/dir/architecture.json")),
        _mGroupName(new cedar::aux::StringParameter(this, "group name", "myLoadGroup")),
        _mGroupContainerName(new cedar::aux::StringParameter(this, "container group", "root"))
{
}

cedar::proc::scripts::RecruitGroup::~RecruitGroup()
{
  this->requestStop();
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

void cedar::proc::scripts::RecruitGroup::run()
{
  if (!ran)
  {
    std::cout << "Hello RecruitGroup! Lets start the Thread!" << std::endl;
    std::set <cedar::dyn::SerialOrderRecruitingPtr> imprintHebbElements = this->getGroup()->findAll<cedar::dyn::SerialOrderRecruiting>(
            true);

    for (auto it = imprintHebbElements.begin(); it != imprintHebbElements.end(); it++)
    {
      auto hebb = *it;
      bool isConnected = QObject::connect(hebb.get(),
                                          SIGNAL(evokeFieldRecruitment(cedar::dyn::SerialOrderRecruitingPtr)), this,
                                          SLOT(recruitGroup(cedar::dyn::SerialOrderRecruitingPtr)),
                                          Qt::ConnectionType::DirectConnection);
      std::cout << "Connected: " << hebb->getName() << " Connect Return: " << isConnected << std::endl;
    }
    ran = true;
  }
  else
  {
    //TODO: Only add unmanaged SerialOrderSteps
    std::cout << "Started the Script for the Second Time. This has no effect! If you added steps in between, this should be fixed!" << std::endl;
  }

}

void cedar::proc::scripts::RecruitGroup::recruitGroup(cedar::dyn::SerialOrderRecruitingPtr serialOrderStep)
{
  std::cout << "Recruit a Group!" << std::endl;
  mCreatedGroupCounter = mCreatedGroupCounter + 1;
  cedar::proc::GroupPtr groupContainer = this->getGroup();

  if (_mGroupContainerName->getValue() != "root")
  {
    std::cout << "The Searched Container Name is: " << _mGroupContainerName->getValue() << std::endl;
    auto possibleContainerVector = this->getGroup()->findElementsAcrossGroupsFullName(_mGroupContainerName->getValue());
    if (auto group = boost::dynamic_pointer_cast<const cedar::proc::Group>(possibleContainerVector.at(0)))
    {
      groupContainer = boost::const_pointer_cast<cedar::proc::Group>(group);
      std::cout << "The Found Container is: " << groupContainer->getName() << std::endl;
    } else
    {
      std::cout << "The specified Groupcontainer was not found! Abort Adding a Group!" << std::endl;
      return;
    }
  }

  auto loadedGroupElement = AddAGroupFromFile(_mGroupName->getValue(), _mGroupFile->getValue(), groupContainer);

  loadedGroupElement->setName(loadedGroupElement->getName() + boost::lexical_cast<std::string>(mCreatedGroupCounter));

  if (auto newGroup = boost::dynamic_pointer_cast<cedar::proc::Group>(loadedGroupElement))
  {
    if (_mGroupContainerName->getValue() != "root")
    {
      std::string containerInputName = "M_" + cedar::aux::toString<unsigned int>(mCreatedGroupCounter);
      groupContainer->addConnector(containerInputName, true);
      this->getGroup()->connectSlots(serialOrderStep->getName() + '.' + "ordinal node " + cedar::aux::toString<unsigned int>(mCreatedGroupCounter) + " output", groupContainer->getName() + "." + containerInputName);
      //Connect SerialOrderStep to the ContainerGroup
      connectGroup(groupContainer, newGroup);
    }
  }
}


void cedar::proc::scripts::RecruitGroup::connectGroup(cedar::proc::GroupPtr containerGroup,
                                                      cedar::proc::GroupPtr toBeIncludedGroup)
{
  auto inputMap = toBeIncludedGroup->getDataSlots(cedar::proc::DataRole::INPUT);
  for (auto it = inputMap.begin(); it != inputMap.end(); it++)
  {
    std::string slotName = it->first;
    auto dataSlot = it->second;
    if (slotName != "SequenceActive")
      containerGroup->connectSlots(slotName + "." + "output", toBeIncludedGroup->getName() + "." + slotName);
    else
      containerGroup->connectSlots("M_" + cedar::aux::toString<unsigned int>(mCreatedGroupCounter)+".output",toBeIncludedGroup->getName() + "." + slotName);
  }
}


cedar::proc::ElementPtr
cedar::proc::scripts::RecruitGroup::AddAGroupFromFile(const std::string &groupName, const cedar::aux::Path &fileName,
                                                      cedar::proc::GroupPtr containerGroup)
{
  // first, read in the configuration tree
  cedar::aux::ConfigurationNode configuration;
  boost::property_tree::read_json(fileName.absolute().toString(false), configuration);

  try
  {
    // try to access the "groups" node
    cedar::aux::ConfigurationNode &groups_node = configuration.get_child("groups");
    try
    {
      // try to access the group node with name "groupName"
      cedar::aux::ConfigurationNode &group_node = groups_node.get_child(groupName);
      group_node.put("ui generic.positionY", 100 + mCreatedGroupCounter *
                                                   275); // Todo: These values are totally arbitrary and should be parameters
      // create, add, and configure
      cedar::proc::GroupPtr imported_group(new cedar::proc::Group());
      containerGroup->add(imported_group, containerGroup->getUniqueIdentifier("imported group"));
      group_node.put("name", containerGroup->getUniqueIdentifier(group_node.get<std::string>("name")));
      imported_group->readConfiguration(group_node);
      return imported_group;
    }
    catch (const boost::property_tree::ptree_bad_path &)
    {
      // could not find given group, abort
      CEDAR_THROW
              (
                      cedar::aux::NotFoundException,
                      "Could not find group with name " + groupName + " in file " + fileName
              );
    }
  }
  catch (const boost::property_tree::ptree_bad_path &)
  {
    // could not find a "groups" node, abort
    CEDAR_THROW(cedar::aux::NotFoundException, "Could not find any groups in file " + fileName);
  }
}

std::string cedar::proc::scripts::RecruitGroup::cutImprintName(std::string fullName)
{
  std::string begin = "Intention";
  std::string end = "_Weights";

  auto match = fullName.find(begin);
  if (match != std::string::npos)
    fullName.erase(match, begin.length());
  match = fullName.find(end);
  if (match != std::string::npos)
    fullName.erase(match, end.length());

  return fullName;
}
