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

    File:        MotionGradient.h

    Maintainer:  Michael Berger
    Email:       michael.berger@ini.ruhr-uni-bochum.de
    Date:        2012 10 17

    Description:

    Credits:

======================================================================================================================*/

#ifndef CEDAR_PROC_STEPS_MOTION_GRADIENT_H
#define CEDAR_PROC_STEPS_MOTION_GRADIENT_H

// LOCAL INCLUDES
#include "steps/motion_detection/MotionGradient.fwd.h"

// PROJECT INCLUDES

// SYSTEM INCLUDES
#include <cedar/processing/Step.h>
#include <cedar/auxiliaries/MatData.h>
#include <cedar/auxiliaries/UIntParameter.h>
#include <cedar/auxiliaries/DoubleParameter.h>


/*!@brief Abstract description of the class.
 *
 * This class provides a cedar processing step implementing the OpenCV function calcMotionGradient
 */
class cedar::proc::steps::MotionGradient : public cedar::proc::Step
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
  MotionGradient();

  //!@brief Destructor
  ~MotionGradient();

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

  //!@brief public function to get the SobelSize parameter
  inline unsigned int getSobelSize() const
  {
    return this->_mSobelSize->getValue();
  }

  //!@brief public function to set the SobelSize parameter
  inline void setSobelSize(unsigned int newValue)
  {
    // note, that you have to call setValue because this also emits a signal notifying anyone interested of the change
    this->_mSobelSize->setValue(newValue);
  }

  //!@brief public function to get the delta1 parameter
  inline double getDelta1() const
  {
    return this->_mDelta1->getValue();
  }

  //!@brief public function to set the delta1 parameter
  inline void setDelta1(unsigned int newValue)
  {
    // note, that you have to call setValue because this also emits a signal notifying anyone interested of the change
    this->_mDelta1->setValue(newValue);
  }

  //!@brief public function to get the delta2 parameter
  inline double getDelta2() const
  {
    return this->_mDelta2->getValue();
  }

  //!@brief public function to set the delta2 parameter
  inline void setDelta2(unsigned int newValue)
  {
    // note, that you have to call setValue because this also emits a signal notifying anyone interested of the change
    this->_mDelta2->setValue(newValue);
  }

public slots:
  //!@brief This slots are connected to the valueChanged() event of the parameters.
  void sobelSizeChanged();
  void delta1Changed();
  void delta2Changed();

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
  cedar::aux::ConstMatDataPtr mInput;
  //!@brief output member
  cedar::aux::MatDataPtr mOutMask;
  cedar::aux::MatDataPtr mOutOrient;

  //--------------------------------------------------------------------------------------------------------------------
  // parameters
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

private:
  //!@brief
  cedar::aux::UIntParameterPtr _mSobelSize;
  //!@brief
  cedar::aux::DoubleParameterPtr _mDelta1;
  //!@brief
  cedar::aux::DoubleParameterPtr _mDelta2;

}; // class cedar::proc::steps::MotionGradient

#endif // CEDAR_PROC_STEPS_MOTION_GRADIENT_H
