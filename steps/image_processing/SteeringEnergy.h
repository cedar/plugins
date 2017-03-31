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

    File:        SteeringEnergy.h

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2011 10 24

    Description:

    Credits:

======================================================================================================================*/

#ifndef CEDAR_PROC_STEPS_STEERING_ENERGY_H
#define CEDAR_PROC_STEPS_STEERING_ENERGY_H

// LOCAL INCLUDES
#include "steps/image_processing/SteeringEnergy.fwd.h"

// PROJECT INCLUDES

// SYSTEM INCLUDES
#include <cedar/processing/Step.h>
#include <cedar/auxiliaries/MatData.h>


class cedar::proc::steps::SteeringEnergy : public cedar::proc::Step
{
  //--------------------------------------------------------------------------------------------------------------------
  // macros
  //--------------------------------------------------------------------------------------------------------------------

  //--------------------------------------------------------------------------------------------------------------------
  // constructors and destructor
  //--------------------------------------------------------------------------------------------------------------------
public:
  //!@brief The standard constructor.
  SteeringEnergy();

  //--------------------------------------------------------------------------------------------------------------------
  // public methods
  //--------------------------------------------------------------------------------------------------------------------
public:
  void compute(const cedar::proc::Arguments&);

  void inputConnectionChanged(const std::string& inputName);

  //--------------------------------------------------------------------------------------------------------------------
  // protected methods
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

  //--------------------------------------------------------------------------------------------------------------------
  // private methods
  //--------------------------------------------------------------------------------------------------------------------
private:
  cv::Mat connectData(const std::string& name, cedar::aux::ConstMatDataPtr& member);

  //--------------------------------------------------------------------------------------------------------------------
  // members
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

private:
  // inputs
  //!@todo These should be mat once MatData inherits MatData
  cedar::aux::ConstMatDataPtr mC2;
  cedar::aux::ConstMatDataPtr mC3;

  // outputs
  cedar::aux::MatDataPtr mEnergy;

  //--------------------------------------------------------------------------------------------------------------------
  // parameters
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

private:
  // none yet

}; // class cedar::proc::steps::SteeringEnergy

#endif // CEDAR_PROC_STEPS_STEERING_ENERGY_H
