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
#include "steps/utilities/RewardHebbTrace.h"
// CEDAR INCLUDES

// SYSTEM INCLUDES

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::proc::scripts::RecruitGroup::RecruitGroup()
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
  std::cout << "Hello RecruitGroup! Lets start the Thread!" << std::endl;
//  std::vector < cedar::proc::ConstElementPtr > imprintHebbElements = this->getGroup()->findElementsAcrossGroupsContainsString("Recruit");

  std::set < boost::shared_ptr<cedar::proc::steps::RewardHebbTrace> > imprintHebbElements = this->getGroup()->findAll < cedar::proc::steps::RewardHebbTrace > (true);

  for (auto it = imprintHebbElements.begin(); it != imprintHebbElements.end(); it++)
  {
    auto hebb = *it;
    bool isConnected = QObject::connect(hebb.get(), SIGNAL(evokeFieldRecruitment(cedar::proc::steps::RewardHebbTrace)), this, SLOT(addCorrespondingField(cedar::proc::steps::RewardHebbTrace)),
        Qt::ConnectionType::DirectConnection);
    std::cout << "Connected: " << hebb->getName() << " Connect Return: " << isConnected << std::endl;
  }
}

void cedar::proc::scripts::RecruitGroup::addCorrespondingField(cedar::proc::steps::RewardHebbTracePtr hebbStep)
{
  std::cout << "Recruit A Group here!" << std::endl;
//  //Add Field and Connect it to the HebbStep
//  std::string mainName = cutImprintName(hebbStep->getName());
//  std::string fieldName = mainName + "_recruit";
//  auto addGroup = hebbStep->getGroup();
//  if (addGroup->findElementsAcrossGroupsFullName(fieldName).empty())
//  {
//    cedar::dyn::NeuralFieldPtr newField = cedar::dyn::NeuralFieldPtr(new cedar::dyn::NeuralField());
//    newField->setDimensionality(hebbStep->getDimensionality());
//    for(unsigned int d = 0; d< hebbStep->getDimensionality();d++)
//    {
//      newField->setSize(d,hebbStep->getSizes().at<float>(d,0));
//    }
//    addGroup->add(newField, fieldName);
//    addGroup->connectSlots(hebbStep->getName() +"."+ hebbStep->getOutputName(), fieldName + ".input");
//    //Connect Field with potential Cue input and create an connection to the Output!
//
//    std::string cueString = "Cue"+mainName+"_Input";
//    std::vector<cedar::proc::ConstElementPtr> cueInput =  addGroup->findElementsAcrossGroupsFullName(cueString);
//    if(cueInput.size()==1)            //One Candidate only!
//    {
//      cedar::proc::steps::StaticGainPtr connectGain = cedar::proc::steps::StaticGainPtr(new cedar::proc::steps::StaticGain);
//      connectGain->setName("CueConnect_"+mainName);
//      addGroup->add(connectGain);
//      addGroup->connectSlots(cueInput.at(0)->getName() +".output",connectGain->getName()+ ".input");
//      addGroup->connectSlots(connectGain->getName()+ ".output",fieldName + ".input");
//      addGroup->addConnector(mainName+"CueOutput", false);
//      addGroup->connectSlots(fieldName + ".sigmoided activation",mainName+"CueOutput.input");
//    }
//    else
//    {
//      std::cout<<"Something with the Name Matching went wrong! The mainName is: "<< mainName<<std::endl;
//    }
//
//  }

}

std::string cedar::proc::scripts::RecruitGroup::cutImprintName(std::string fullName)
{
  std::string begin = "Intention";
  std::string end = "_Weights";

  auto match = fullName.find(begin);
  if ( match!= std::string::npos)
    fullName.erase(match, begin.length());
  match = fullName.find(end);
  if ( match!= std::string::npos)
      fullName.erase(match, end.length());

  return fullName;
}
