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

    File:        ElementwiseDivide.h

    Maintainer:  Michael Berger
    Email:       michael.berger@ini.ruhr-uni-bochum.de
    Date:        2012 08 28

    Description:

    Credits:

======================================================================================================================*/

#ifndef CEDAR_PROC_STEPS_ELEMENTWISE_DIVIDE_H
#define CEDAR_PROC_STEPS_ELEMENTWISE_DIVIDE_H

// LOCAL INCLUDES
#include "steps/utilities/ElementwiseDivide.fwd.h"

// PROJECT INCLUDES

// SYSTEM INCLUDES
#include <cedar/processing/Step.h>
#include <cedar/auxiliaries/DataTemplate.h>
#include <cedar/auxiliaries/MatData.h>
#include <cedar/auxiliaries/DoubleParameter.h>


/*!@brief Abstract description of the class.
 *
 * This class provides a cedar processing step. To input matrices Nom and Denom will be divided elementwisely. The output is Nom/(Denom + scalar).
 * "scalar" is a scalar and configurable parameter. This value is added to all elements in the Denom matrix to prevent dividing by zero or very small values.
 */
class cedar::proc::steps::ElementwiseDivide : public cedar::proc::Step
{
  //--------------------------------------------------------------------------------------------------------------------
  // macros
  //--------------------------------------------------------------------------------------------------------------------
  Q_OBJECT

  //--------------------------------------------------------------------------------------------------------------------
  // constructors and destructor
  //--------------------------------------------------------------------------------------------------------------------
public:
  //!@brief The standard constructor.
  ElementwiseDivide();

  //!@brief Destructor
  ~ElementwiseDivide();

  //--------------------------------------------------------------------------------------------------------------------
  // public methods
  //--------------------------------------------------------------------------------------------------------------------
public:
  //!@brief compute
  void compute(const cedar::proc::Arguments&);

  //!@brief execute if input changes
  void inputConnectionChanged(const std::string& inputName);

  //!@brief check the input
  cedar::proc::DataSlot::VALIDITY determineInputValidity(cedar::proc::ConstDataSlotPtr, cedar::aux::ConstDataPtr) const;

  //!@brief public function to get the denomAdd parameter
  inline double getDenomAdd() const
  {
    return this->_mDenomAdd->getValue();
  }

  //!@brief public function to set the denomAdd parameter
  inline void setDenomAdd(unsigned int newValue)
  {
    // note, that you have to call setValue because this also emits a signal notifying anyone interested of the change
    this->_mDenomAdd->setValue(newValue);
  }

public slots:
  //!@brief This slot is connected to the valueChanged() event of the denomAdd value parameter.
  void denomAddChanged();

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
  //!@brief input members
  cedar::aux::ConstMatDataPtr mNom;
  cedar::aux::ConstMatDataPtr mDenom;
  //!@brief output member
  cedar::aux::MatDataPtr mOutput;

  //--------------------------------------------------------------------------------------------------------------------
  // parameters
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

private:
  //!@brief a scalar value, which is added to all elements in the denominator matrix to prevent dividing by zero
  cedar::aux::DoubleParameterPtr _mDenomAdd;

}; // class cedar::proc::steps::ElementwiseDivide

#endif // CEDAR_PROC_STEPS_ELEMENTWISE_DIVIDE_H
