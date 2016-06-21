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

    File:        InhibitoryNeuron.h

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2014 10 01

    Description: Header file for the class dynamics::InhibitoryNeuron.

    Credits:

======================================================================================================================*/

#ifndef CEDAR_DYN_INHIBITORY_NEURON_H
#define CEDAR_DYN_INHIBITORY_NEURON_H

// CEDAR CONFIGURATION
#include "cedar/configuration.h"

// CEDAR INCLUDES
#include "cedar/dynamics/Dynamics.h"
#include "cedar/processing/InputSlotHelper.h"
#include "cedar/auxiliaries/DoubleParameter.h"
#include "cedar/auxiliaries/TimeParameter.h"
#include "cedar/auxiliaries/ObjectParameterTemplate.h"
#include "cedar/auxiliaries/math/TransferFunction.h"

// FORWARD DECLARATIONS
#include "steps/dynamics/InhibitoryNeuron.fwd.h"

// SYSTEM INCLUDES


/*!@brief A neuron for calculating global inhibition of a field externally.
 */
class cedar::dyn::InhibitoryNeuron : public cedar::dyn::Dynamics
{
  //--------------------------------------------------------------------------------------------------------------------
  // nested types
  //--------------------------------------------------------------------------------------------------------------------
private:
  //!@brief a parameter for sigmoid objects
  typedef cedar::aux::ObjectParameterTemplate<cedar::aux::math::TransferFunction> SigmoidParameter;
  CEDAR_GENERATE_POINTER_TYPES_INTRUSIVE(SigmoidParameter);

  //--------------------------------------------------------------------------------------------------------------------
  // constructors and destructor
  //--------------------------------------------------------------------------------------------------------------------
public:
  //!@brief The standard constructor.
  InhibitoryNeuron();

  //--------------------------------------------------------------------------------------------------------------------
  // public methods
  //--------------------------------------------------------------------------------------------------------------------
public:
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
  void eulerStep(const cedar::unit::Time& time);

  //--------------------------------------------------------------------------------------------------------------------
  // members
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet
private:
  cedar::proc::InputSlotHelper<cedar::aux::MatData> mSigmoidedActivity;

  cedar::aux::MatDataPtr mInhibition;

  cedar::aux::MatDataPtr mSum;


  //--------------------------------------------------------------------------------------------------------------------
  // parameters
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

private:
  //!@brief any sigmoid function
  SigmoidParameterPtr _mSigmoid;

  cedar::aux::DoubleParameterPtr _mTanhSlope;

  cedar::aux::TimeParameterPtr _mTau;

}; // class cedar::dyn::InhibitoryNeuron

#endif // CEDAR_DYN_INHIBITORY_NEURON_H

