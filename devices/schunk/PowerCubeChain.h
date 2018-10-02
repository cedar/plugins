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

    File:        PowerCubeChain.h

    Maintainer:  Mathis Richter
    Email:       mathis.richter@ini.rub.de
    Date:        2014 12 02

    Description:

    Credits:     Ceyhun Pekis

======================================================================================================================*/

#ifndef CEDAR_DEV_SCHUNK_POWER_CUBE_CHAIN_H
#define CEDAR_DEV_SCHUNK_POWER_CUBE_CHAIN_H

// LOCAL INCLUDES
#include "devices/schunk/PowerCubeChain.fwd.h"
#include "devices/schunk/SMPCanBusChannel.h"

// CEDAR INCLUDES
#include <cedar/devices/Component.h>
#include <cedar/devices/KinematicChain.h>
#include <cedar/auxiliaries/UIntParameter.h>
#include <cedar/auxiliaries/UIntVectorParameter.h>

// SYSTEM INCLUDES


/*! KinematicChain for the Schunk PowerCubes. */
class cedar::dev::schunk::PowerCubeChain : public cedar::dev::KinematicChain
{
  //--------------------------------------------------------------------------------------------------------------------
  // nested types
  //--------------------------------------------------------------------------------------------------------------------

  //--------------------------------------------------------------------------------------------------------------------
  // constructors and destructor
  //--------------------------------------------------------------------------------------------------------------------
public:
  //!@brief The standard constructor.
  PowerCubeChain();

  //!@brief Destructor
  virtual ~PowerCubeChain();

  //--------------------------------------------------------------------------------------------------------------------
  // public methods
  //--------------------------------------------------------------------------------------------------------------------
public:
  void sendJointAngles(cv::Mat angles);
  void sendJointVelocities(cv::Mat angles);
  cv::Mat retrieveJointAngles();
  cv::Mat retrieveJointVelocities();
  cv::Mat retrieveJointCurrents();
  cv::Mat retrieveJointDurations();

  CEDAR_DECLARE_DEPRECATED(virtual bool isMovable() const);

  void readConfiguration(const cedar::aux::ConfigurationNode& node);

  //--------------------------------------------------------------------------------------------------------------------
  // protected methods
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

  //--------------------------------------------------------------------------------------------------------------------
  // private methods
  //--------------------------------------------------------------------------------------------------------------------
private:
  // none yet

  //--------------------------------------------------------------------------------------------------------------------
  // members
  //--------------------------------------------------------------------------------------------------------------------
public:
  static const cedar::dev::Component::ComponentDataType JOINT_CURRENTS;
  static const cedar::dev::Component::ComponentDataType JOINT_DURATIONS;

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
  //! a mapping from joint position in the chain to module id
  cedar::aux::UIntVectorParameterPtr _mModuleMapping;

}; // class cedar::dev::schunk::PowerCubeChain

#endif // CEDAR_DEV_SCHUNK_POWER_CUBE_CHAIN_H
