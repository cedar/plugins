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

    File:        ShiftedAddition.h

    Maintainer:  Mathis Richter
    Email:       mathis.richter@ini.rub.de
    Date:        2016 01 08

    Description:

    Credits:

======================================================================================================================*/

#ifndef CEDAR_PROC_STEPS_SHIFTED_ADDITION_H
#define CEDAR_PROC_STEPS_SHIFTED_ADDITION_H

// LOCAL INCLUDES
#include "steps/utilities/ShiftedAddition.fwd.h"

// PROJECT INCLUDES

// SYSTEM INCLUDES
#include <cedar/processing/Step.h>
#include <cedar/auxiliaries/UIntParameter.fwd.h>
#include <cedar/auxiliaries/MatData.fwd.h>


/*!@brief A processing step that generates spatial patterns for "left", "right", "above", and "below".
 */
class cedar::proc::steps::ShiftedAddition : public cedar::proc::Step
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
  ShiftedAddition();

  //!@brief Destructor
  ~ShiftedAddition();

  //--------------------------------------------------------------------------------------------------------------------
  // public methods
  //--------------------------------------------------------------------------------------------------------------------
public:
  void compute(const cedar::proc::Arguments&);

public slots:
  void recompute();
  void reconfigure();
  void inputConnectionChanged(const std::string&);

  //--------------------------------------------------------------------------------------------------------------------
  // protected methods
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

  //--------------------------------------------------------------------------------------------------------------------
  // private methods
  //--------------------------------------------------------------------------------------------------------------------
private:

  //--------------------------------------------------------------------------------------------------------------------
  // members
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

private:
  // inputs
  cedar::aux::MatDataPtr mToward;
  cedar::aux::MatDataPtr mAway;

  // outputs
  cedar::aux::MatDataPtr mOutput;

  //--------------------------------------------------------------------------------------------------------------------
  // parameters
  //--------------------------------------------------------------------------------------------------------------------
private:
  cedar::aux::UIntParameterPtr _mDistance;
  cedar::aux::UIntParameterPtr _mOrientationSize;
}; // class cedar::proc::steps::ShiftedAddition

#endif // CEDAR_PROC_STEPS_SHIFTED_ADDITION_H
