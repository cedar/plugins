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

    File:        ApproximateCouplingVector.h

    Maintainer:  
    Email:       
    Date:        

    Description: 

    Credits:

======================================================================================================================*/

#ifndef CEDAR_PROC_STEPS_APPROXIMATE_VECTOR_COUPLING_H
#define CEDAR_PROC_STEPS_APPROXIMATE_VECTOR_COUPLING_H

// LOCAL INCLUDES
#include "steps/neuraloscillator/ApproximateCouplingVector.fwd.h"

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
class cedar::proc::steps::ApproximateCouplingVector : public cedar::proc::Step
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
  ApproximateCouplingVector();

  //--------------------------------------------------------------------------------------------------------------------
  // public slots
  //--------------------------------------------------------------------------------------------------------------------
public slots:
  //@called when the vector dimension changes
  void vectorDimensionChanged();
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

  static float calculateDurationFromIndex(unsigned int i, unsigned int size, float min, float max);

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

  //--------------------------------------------------------------------------------------------------------------------
  // parameters
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

private:
  //Parameter for the dimension of the output vector
  cedar::aux::UIntParameterPtr _mOutputDimension;
  cedar::aux::UIntParameterPtr _mSize;
  cedar::aux::DoubleParameterPtr _mMinimalDuration;
  cedar::aux::DoubleParameterPtr _mMaximalDuration;

}; // cedar::proc::steps::ApproximateCouplingVector

#endif // CEDAR_PROC_STEPS_APPROXIMATE_VECTOR_COUPLING_H


