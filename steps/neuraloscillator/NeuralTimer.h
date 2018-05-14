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

    File:        NeuralTimer.h

    Maintainer:  
    Email:       
    Date:        

    Description:

    Credits:

======================================================================================================================*/

#ifndef CEDAR_PROC_STEPS_NERUAL_TIMER_H
#define CEDAR_PROC_STEPS_NERUAL_TIMER_H

// LOCAL INCLUDES
#include "steps/neuraloscillator/NeuralTimer.fwd.h"

// PROJECT INCLUDES

// SYSTEM INCLUDES
#include <cedar/processing/Step.h>
#include <cedar/auxiliaries/ObjectParameterTemplate.fwd.h>
#include <cedar/auxiliaries/math/TransferFunction.h>
#include <cedar/auxiliaries/BoolParameter.fwd.h>
#include <cedar/auxiliaries/UIntParameter.fwd.h>
#include <cedar/auxiliaries/DoubleParameter.fwd.h>
#include <cedar/auxiliaries/MatData.fwd.h>


/*!@brief A processing step that generates spatial patterns for "left", "right", "above", and "below".
 */
class cedar::proc::steps::NeuralTimer : public cedar::proc::Step
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
  NeuralTimer();

  //!@brief Destructor
  ~NeuralTimer();

  //--------------------------------------------------------------------------------------------------------------------
  // public methods
  //--------------------------------------------------------------------------------------------------------------------
public:
  void compute(const cedar::proc::Arguments&);


  static float calculateUofT(float t, float s, float c, float tau, float h);
  static float calculateUDotofT(float t, float s, float c, float tau, float h);
  static float calculateCouplingFromTF(float tf);
  static float calculateInputS(float D, float c, float tf, float tau, float
  h);
  static float calculateBaseIntegral(float s, float c, float tf, float tau, float h);
  static float calculateTMaxOfU(float c);
  static float calculateModIntegral(float s, float c, float tf, float tau, float h);

  //--------------------------------------------------------------------------------------------------------------------
  // protected methods
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

  //--------------------------------------------------------------------------------------------------------------------
  // private methods
  //--------------------------------------------------------------------------------------------------------------------
private:
  void recompute();

public slots:
  void stuffChanged();

  //--------------------------------------------------------------------------------------------------------------------
  // members
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

private:
  // inputs
  // none

  // outputs
  cedar::aux::MatDataPtr mMatrixWBase;
  cedar::aux::MatDataPtr mMatrixWMod;
  cedar::aux::MatDataPtr mCouplingOut;

  //--------------------------------------------------------------------------------------------------------------------
  // parameters
  //--------------------------------------------------------------------------------------------------------------------
private:

  cedar::aux::DoubleParameterPtr _mTau;
  cedar::aux::DoubleParameterPtr _mH;
  cedar::aux::DoubleParameterPtr _mS;
}; // class cedar::proc::steps::EquidistantRidge

#endif // CEDAR_PROC_STEPS_EQUIDIESTANT_RIDGE3_H
