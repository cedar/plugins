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

    File:        EquidistantRidge.h

    Maintainer:  
    Email:       
    Date:        

    Description:

    Credits:

======================================================================================================================*/

#ifndef CEDAR_PROC_STEPS_EQUIDISTANT_MATRIX_RIDGE_H
#define CEDAR_PROC_STEPS_EQUIDISTANT_MATRIX_RIDGE_H

// LOCAL INCLUDES
#include "steps/neuraloscillator/EquidistantMatrix.fwd.h"

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
class cedar::proc::steps::EquidistantMatrix : public cedar::proc::Step
{
  //--------------------------------------------------------------------------------------------------------------------
  // macros
  //--------------------------------------------------------------------------------------------------------------------
  Q_OBJECT

  //--------------------------------------------------------------------------------------------------------------------
  // nested types
  //--------------------------------------------------------------------------------------------------------------------
  typedef cedar::aux::ObjectParameterTemplate<cedar::aux::math::TransferFunction> SigmoidParameter;

  //!@cond SKIPPED_DOCUMENTATION
  CEDAR_GENERATE_POINTER_TYPES_INTRUSIVE(SigmoidParameter);
  //!@endcond

  //--------------------------------------------------------------------------------------------------------------------
  // constructors and destructor
  //--------------------------------------------------------------------------------------------------------------------
public:
  //!@brief The standard constructor.
  EquidistantMatrix();

  //!@brief Destructor
  ~EquidistantMatrix();

  //--------------------------------------------------------------------------------------------------------------------
  // public methods
  //--------------------------------------------------------------------------------------------------------------------
public:
  void compute(const cedar::proc::Arguments&);
  static float calculateDistanceFromIndex(unsigned int i, unsigned int siz, unsigned int min, unsigned int max);
  static float calculateVelocityFromIndex(unsigned int i, unsigned int siz, unsigned int min, unsigned int max);

public slots:
  void recompute();

  //--------------------------------------------------------------------------------------------------------------------
  // protected methods
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

  //--------------------------------------------------------------------------------------------------------------------
  // private methods
  //--------------------------------------------------------------------------------------------------------------------
private:
  void internal_recompute();

  //--------------------------------------------------------------------------------------------------------------------
  // members
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

private:
  // inputs
  // none

  // outputs
  cedar::aux::MatDataPtr mMatrix;

  //--------------------------------------------------------------------------------------------------------------------
  // parameters
  //--------------------------------------------------------------------------------------------------------------------
private:
  cedar::aux::UIntParameterPtr _mSizeX;
  cedar::aux::UIntParameterPtr _mSizeY;

  cedar::aux::DoubleParameterPtr _mMinimalDuration;
  cedar::aux::DoubleParameterPtr _mMaximalDuration;
  cedar::aux::DoubleParameterPtr _mDurationOffset;
  cedar::aux::DoubleParameterPtr _mMinimalDistance;
  cedar::aux::DoubleParameterPtr _mMaximalDistance;

  cedar::aux::DoubleParameterPtr _mTau;
  cedar::aux::DoubleParameterPtr _mH;
}; // class cedar::proc::steps::EquidistantRidge

#endif // CEDAR_PROC_STEPS_EQUIDIESTANT_RIDGE3_H
