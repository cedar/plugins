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

    File:        CounterChangeCombination.cpp

    Maintainer:  Michael Berger

    Email:       michael.berger@ini.ruhr-uni-bochum.de

    Date:        2012 08 09

    Description: Combination of toward and away signals, providing input for a counterchange motion detection field

    Credits:

======================================================================================================================*/

#ifndef CEDAR_PROC_STEPS_COUNTER_CHANGE_COMBINATION_H
#define CEDAR_PROC_STEPS_COUNTER_CHANGE_COMBINATION_H

// LOCAL INCLUDES
#include "steps/motion_detection/CounterChangeCombination.fwd.h"

// PROJECT INCLUDESutilities

// SYSTEM INCLUDES
#include <cedar/processing/Step.h>
#include <cedar/auxiliaries/MatData.h>
#include <cedar/auxiliaries/convolution/Convolution.h>
#include "cedar/auxiliaries/UIntParameter.h"
#include <cedar/auxiliaries/kernel/Gauss.h>


/*!@brief
 * This class provides a cedar processing step. The inputs have to be 3D toward and away matrices from a single edge filtered video or camera input.
 * The first dimension of the input matrices is considered as the edge orientation dimension. The input matrices are shifted orthogonal the edge
 * orientation by the size of the configurable shift parameter. Output matrices are two 3D matrices that reflect the combination of toward and away
 * signals according to the counter-change rule. The first dimension is assigned to the motion direction. The two matrices represent the polarity if
 * a dark pattern moves on bright ground (BtW: Black to White motion) or the other way around (WtB: White to Black motion)
 */
class cedar::proc::steps::CounterChangeCombination : public cedar::proc::Step
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
  CounterChangeCombination();

  //!@brief Destructor
  ~CounterChangeCombination();

  //--------------------------------------------------------------------------------------------------------------------
  // public methods
  //--------------------------------------------------------------------------------------------------------------------
public:
  //!@brief execute if input changes
  void inputConnectionChanged(const std::string& inputName);

  //!@brief check the input
  cedar::proc::DataSlot::VALIDITY determineInputValidity(cedar::proc::ConstDataSlotPtr, cedar::aux::ConstDataPtr) const;

  //!@brief public function to get the shift parameter
  inline unsigned int getShift() const
  {
    return this->_mShift->getValue();
  }

  //!@brief public function to set the shift parameter
  inline void setShift(unsigned int newValue)
  {
    // note, that you have to call setValue because this also emits a signal notifying anyone interested of the change
    this->_mShift->setValue(newValue);
  }

public slots:
  //!@brief This slot is connected to the valueChanged() event of the shift value parameter.
  void shiftChanged();

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
  //!@brief the convolution object.
  cedar::aux::conv::ConvolutionPtr mConvolution;

  //!@brief input
  cedar::aux::ConstMatDataPtr mAway;
  cedar::aux::ConstMatDataPtr mToward;
  //!@brief Output
  cedar::aux::MatDataPtr mOutputBtW;
  cedar::aux::MatDataPtr mOutputWtB;
  

  //--------------------------------------------------------------------------------------------------------------------
  // parameters
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

private:
  //!@brief shift size
  cedar::aux::UIntParameterPtr _mShift;


}; // class cedar::proc::steps::CounterChangeCombination

#endif // CEDAR_PROC_STEPS_COUNTER_CHANGE_COMBINATION_H
