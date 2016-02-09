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

    File:        SpatialPattern.h

    Maintainer:  Mathis Richter
    Email:       mathis.richter@ini.rub.de
    Date:        2013 10 30

    Description:

    Credits:

======================================================================================================================*/

#ifndef CEDAR_PROC_STEPS_SPATIAL_PATTERN_H
#define CEDAR_PROC_STEPS_SPATIAL_PATTERN_H

// LOCAL INCLUDES
#include "steps/sources/SpatialPattern.fwd.h"

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
class cedar::proc::steps::SpatialPattern : public cedar::proc::Step
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
  SpatialPattern();

  //!@brief Destructor
  ~SpatialPattern();

  //--------------------------------------------------------------------------------------------------------------------
  // public methods
  //--------------------------------------------------------------------------------------------------------------------
public:
  void compute(const cedar::proc::Arguments&);

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

  //--------------------------------------------------------------------------------------------------------------------
  // members
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

private:
  // inputs
  // none

  // outputs
  cedar::aux::MatDataPtr mPattern;

  //--------------------------------------------------------------------------------------------------------------------
  // parameters
  //--------------------------------------------------------------------------------------------------------------------
private:
  cedar::aux::BoolParameterPtr _mInvertSides;
  cedar::aux::BoolParameterPtr _mHorizontalPattern;
  cedar::aux::UIntParameterPtr _mSizeX;
  cedar::aux::UIntParameterPtr _mSizeY;
  cedar::aux::DoubleParameterPtr _mSigmaTh;
  cedar::aux::UIntParameterPtr _mMuR;
  cedar::aux::DoubleParameterPtr _mSigmaR;
  cedar::aux::DoubleParameterPtr _mScaleSigmoid;
  SigmoidParameterPtr _mSigmoid;
}; // class cedar::proc::steps::SpatialPattern

#endif // CEDAR_PROC_STEPS_SPATIAL_PATTERN_H
