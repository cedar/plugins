/*======================================================================================================================

    Copyright 2011, 2012, 2013, 2014 Institut fuer Neuroinformatik, Ruhr-Universitaet Bochum, Germany

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

    File:        PowerCubeChain.cpp

    Maintainer:  Mathis Richter
    Email:       mathis.richter@ini.rub.de
    Date:        2014 12 02

    Description:

    Credits:     Ceyhun Pekis

======================================================================================================================*/

// LOCAL INCLUDES
#include "devices/schunk/PowerCubeChain.h"

// CEDAR INCLUDES
#include <cedar/auxiliaries/math/constants.h>
#include <cedar/auxiliaries/casts.h>

// SYSTEM INCLUDES

const cedar::dev::Component::ComponentDataType cedar::dev::schunk::PowerCubeChain::JOINT_CURRENTS = 5;
const cedar::dev::Component::ComponentDataType cedar::dev::schunk::PowerCubeChain::JOINT_DURATIONS = 6;

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::dev::schunk::PowerCubeChain::PowerCubeChain()
:
_mModuleMapping
(
  new cedar::aux::UIntVectorParameter(this, "module mapping", cedar::aux::UIntVectorParameter::LimitType::full())
)
{
  installMeasurementType(cedar::dev::schunk::PowerCubeChain::JOINT_CURRENTS, "Joint Currents");
  installMeasurementType(cedar::dev::schunk::PowerCubeChain::JOINT_DURATIONS, "Joint Durations");

  registerCommandHook(cedar::dev::KinematicChain::JOINT_ANGLES, boost::bind(&cedar::dev::schunk::PowerCubeChain::sendJointAngles, this, _1));
  registerCommandHook(cedar::dev::KinematicChain::JOINT_VELOCITIES, boost::bind(&cedar::dev::schunk::PowerCubeChain::sendJointVelocities, this, _1));
  registerMeasurementHook(cedar::dev::KinematicChain::JOINT_ANGLES, boost::bind(&cedar::dev::schunk::PowerCubeChain::retrieveJointAngles, this));
  registerMeasurementHook(cedar::dev::KinematicChain::JOINT_VELOCITIES, boost::bind(&cedar::dev::schunk::PowerCubeChain::retrieveJointVelocities, this));
  registerMeasurementHook(cedar::dev::schunk::PowerCubeChain::JOINT_CURRENTS, boost::bind(&cedar::dev::schunk::PowerCubeChain::retrieveJointCurrents, this));
  registerMeasurementHook(cedar::dev::schunk::PowerCubeChain::JOINT_DURATIONS, boost::bind(&cedar::dev::schunk::PowerCubeChain::retrieveJointDurations, this));

  defineUserSelectableCommandTypeSubset("angles");
  defineUserSelectableCommandTypeSubset("velocities");

  addCommandTypeToUserSelectableCommandTypeSubset("angles", cedar::dev::KinematicChain::JOINT_ANGLES);
  addCommandTypeToUserSelectableCommandTypeSubset("velocities", cedar::dev::KinematicChain::JOINT_VELOCITIES);
}

cedar::dev::schunk::PowerCubeChain::~PowerCubeChain()
{
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

void cedar::dev::schunk::PowerCubeChain::readConfiguration(const cedar::aux::ConfigurationNode& node)
{
  cedar::dev::KinematicChain::readConfiguration(node);

  SMPCanBusChannelPtr channel = cedar::aux::asserted_pointer_cast<SMPCanBusChannel>(getChannel());
  channel->addModules(_mModuleMapping->getValue());
}

// todo: this method is deprecated in the KinematicChain but Jean-Stephane wants to keep it in. now what?
bool cedar::dev::schunk::PowerCubeChain::isMovable() const
{
  return true;
}

cv::Mat cedar::dev::schunk::PowerCubeChain::retrieveJointAngles()
{
  SMPCanBusChannelPtr channel = cedar::aux::asserted_pointer_cast<SMPCanBusChannel>(getChannel());
  cv::Mat joint_angles = cv::Mat::zeros(getNumberOfJoints(), 1, CV_64F);

  for (unsigned i = 0; i < getNumberOfJoints(); ++i)
  {
    channel->getState(_mModuleMapping->at(i), true, false, false);
    joint_angles.at<double>(i,0) = channel->getLastJointPosition(_mModuleMapping->at(i));
  }

  return joint_angles;
}

cv::Mat cedar::dev::schunk::PowerCubeChain::retrieveJointVelocities()
{
  SMPCanBusChannelPtr channel = cedar::aux::asserted_pointer_cast<SMPCanBusChannel>(getChannel());
  cv::Mat joint_velocities = cv::Mat::zeros(getNumberOfJoints(), 1, CV_64F);

  for (unsigned i = 0; i < getNumberOfJoints(); ++i)
  {
    channel->getState(_mModuleMapping->at(i), true, true, false);
    joint_velocities.at<double>(i,0) = channel->getLastJointVelocity(_mModuleMapping->at(i));
  }

  return joint_velocities;
}

cv::Mat cedar::dev::schunk::PowerCubeChain::retrieveJointCurrents()
{
  SMPCanBusChannelPtr channel = cedar::aux::asserted_pointer_cast<SMPCanBusChannel>(getChannel());
  cv::Mat joint_currents = cv::Mat::zeros(getNumberOfJoints(), 1, CV_64F);

  for (unsigned i = 0; i < getNumberOfJoints(); ++i)
  {
    channel->getState(_mModuleMapping->at(i), true, true, true);
    joint_currents.at<double>(i,0) = channel->getLastJointCurrent(_mModuleMapping->at(i));
  }

  return joint_currents;
}

cv::Mat cedar::dev::schunk::PowerCubeChain::retrieveJointDurations()
{
  SMPCanBusChannelPtr channel = cedar::aux::asserted_pointer_cast<SMPCanBusChannel>(getChannel());
  cv::Mat joint_durations = cv::Mat::zeros(getNumberOfJoints(), 1, CV_64F);

  for (unsigned i = 0; i < getNumberOfJoints(); ++i)
  {
    channel->getState(_mModuleMapping->at(i), false, false, false);
    joint_durations.at<double>(i,0) = channel->getLastJointDuration(_mModuleMapping->at(i));
  }

  return joint_durations;
}

void cedar::dev::schunk::PowerCubeChain::sendJointAngles(cv::Mat angles)
{
  SMPCanBusChannelPtr channel = cedar::aux::asserted_pointer_cast<SMPCanBusChannel>(getChannel());

  for (unsigned int i = 0; i < static_cast<unsigned int>(angles.rows); ++i)
  {
    channel->movePos(_mModuleMapping->at(i), static_cast<float>(angles.at<double>(i, 0)/(cedar::aux::math::pi)*180));
  }
}

void cedar::dev::schunk::PowerCubeChain::sendJointVelocities(cv::Mat velocities)
{
  SMPCanBusChannelPtr channel = cedar::aux::asserted_pointer_cast<SMPCanBusChannel>(getChannel());

  for (unsigned int i = 0; i < static_cast<unsigned int>(velocities.rows); ++i)
  {
    channel->moveVel(_mModuleMapping->at(i), static_cast<float>(velocities.at<double>(i, 0)/(cedar::aux::math::pi)*180));
  }
}
