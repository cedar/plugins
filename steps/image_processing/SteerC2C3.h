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

    File:        SteerC2C3.h

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2011 10 19

    Description:

    Credits:

======================================================================================================================*/

#ifndef CEDAR_PROC_STEPS_STEER_C2_C3_H
#define CEDAR_PROC_STEPS_STEER_C2_C3_H

// FORWARD HEADER
#include "steps/image_processing/SteerC2C3.fwd.h"

// PROJECT INCLUDES

// SYSTEM INCLUDES
#include <cedar/processing/Step.h>
#include <cedar/auxiliaries/MatData.h>


class cedar::proc::steps::SteerC2C3 : public cedar::proc::Step
{
  //--------------------------------------------------------------------------------------------------------------------
  // macros
  //--------------------------------------------------------------------------------------------------------------------

  //--------------------------------------------------------------------------------------------------------------------
  // constructors and destructor
  //--------------------------------------------------------------------------------------------------------------------
public:
  //!@brief The standard constructor.
  SteerC2C3();

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
  cedar::aux::ConstMatDataPtr mG2a;
  cedar::aux::ConstMatDataPtr mG2b;
  cedar::aux::ConstMatDataPtr mG2c;

  cedar::aux::ConstMatDataPtr mH2a;
  cedar::aux::ConstMatDataPtr mH2b;
  cedar::aux::ConstMatDataPtr mH2c;
  cedar::aux::ConstMatDataPtr mH2d;

  // outputs
  cedar::aux::MatDataPtr mC2;
  cedar::aux::MatDataPtr mC3;

  //--------------------------------------------------------------------------------------------------------------------
  // parameters
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

private:
  // none yet

}; // class cedar::proc::steps::SteerC2C3

#endif // CEDAR_PROC_STEPS_STEER_C2_C3_H
