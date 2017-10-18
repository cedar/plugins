/*======================================================================================================================

    Copyright 2011, 2012, 2013 Institut fuer Neuroinformatik, Ruhr-Universitaet Bochum, Germany
 
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

    File:        ApproximateInput.h

    Maintainer:  Guido Knips
    Email:       guido.knips@ini.rub.de
    Date:        2013 12 04

    Description: Takes a number of scalar inputs and copies them into a one-dimensional vector

    Credits:

======================================================================================================================*/

#ifndef CEDAR_PROC_STEPS_APPROXIMATE_INPUT_H
#define CEDAR_PROC_STEPS_APPROXIMATE_INPUT_H

// LOCAL INCLUDES
#include "steps/neuraloscillator/ApproximateInput.fwd.h"
// PROJECT INCLUDES

// SYSTEM INCLUDES
#include <cedar/processing/Step.h>
#include <cedar/auxiliaries/MatData.h>
#include <cedar/auxiliaries/UIntParameter.h>
#include <cedar/auxiliaries/DoubleParameter.h>
#include <vector>


/*!@todo describe.
 *
 * @todo describe more.
 */
class cedar::proc::steps::ApproximateInput : public cedar::proc::Step
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
  ApproximateInput();

  static float calculateMaxVelocity(float D, float c, float tf, float tau, float
  h);
  static float calculateDistance(float s, float c, float tf, float tau, float h);

  //--------------------------------------------------------------------------------------------------------------------
  // public slots
  //--------------------------------------------------------------------------------------------------------------------
public slots:
  //--------------------------------------------------------------------------------------------------------------------
  // public methods
  //--------------------------------------------------------------------------------------------------------------------
public:
  void inputConnectionChanged(const std::string& inputName);
  //!@brief input verification
  cedar::proc::DataSlot::VALIDITY determineInputValidity
  (
    cedar::proc::ConstDataSlotPtr slot,
    cedar::aux::ConstDataPtr data
  )const;

  //--------------------------------------------------------------------------------------------------------------------
  // protected methods
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

  //--------------------------------------------------------------------------------------------------------------------
  // private methods
  //--------------------------------------------------------------------------------------------------------------------
private:
  void compute(const cedar::proc::Arguments&);

  //--------------------------------------------------------------------------------------------------------------------
  // members
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet
private:
  //output vector
  cedar::aux::MatDataPtr mOutput;
  //input scalars
  std::vector< cedar::aux::ConstMatDataPtr > mInputs;

  //--------------------------------------------------------------------------------------------------------------------
  // parameters
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

private:
  //Parameter for the dimension of the output vector
  cedar::aux::DoubleParameterPtr _mTau;
  cedar::aux::DoubleParameterPtr _mH;

}; // cedar::proc::steps::ApproximateInput

#endif // CEDAR_PROC_STEPS_APPROXIMATE_INPUT_H


