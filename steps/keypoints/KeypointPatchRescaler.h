/*======================================================================================================================

    Copyright 2011, 2012, 2013, 2014, 2015 Institut fuer Neuroinformatik, Ruhr-Universitaet Bochum, Germany
 
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

    File:        Rescaler.h

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2015 02 13

    Description: Header file for the class keypoints::Rescaler.

    Credits:

======================================================================================================================*/

#ifndef CEDAR_PROC_STEPS_KEYPOINT_PATCH_RESCALER_H
#define CEDAR_PROC_STEPS_KEYPOINT_PATCH_RESCALER_H

// CEDAR CONFIGURATION
#include "cedar/configuration.h"

// CEDAR INCLUDES
#include <cedar/processing/Arguments.h>
#include <cedar/processing/Step.h>
#include <cedar/processing/InputSlotHelper.h>
#include <cedar/auxiliaries/DoubleParameter.h>
#include <cedar/auxiliaries/UIntParameter.h>
#include <cedar/auxiliaries/EnumParameter.h>
#include <cedar/auxiliaries/MatData.h>

// FORWARD DECLARATIONS
#include "steps/keypoints/KeypointPatchRescaler.fwd.h"

// SYSTEM INCLUDES


/*!@todo describe.
 *
 * @todo describe more.
 */
class cedar::proc::steps::KeypointPatchRescaler : public cedar::proc::Step
{
  Q_OBJECT
  
  //--------------------------------------------------------------------------------------------------------------------
  // nested types
  //--------------------------------------------------------------------------------------------------------------------

  //--------------------------------------------------------------------------------------------------------------------
  // constructors and destructor
  //--------------------------------------------------------------------------------------------------------------------
public:
  //!@brief The standard constructor.
  KeypointPatchRescaler();

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
  void compute(const cedar::proc::Arguments& arguments);
  
  unsigned int getOutputSize() const
  {
    return this->_mOutputSize->getValue();
  }
  
  unsigned int getPadding() const
  {
    return this->_mPadding->getValue();
  }

private slots:
  void updateLambdas();
  
  void updateOutputSize();

  //--------------------------------------------------------------------------------------------------------------------
  // members
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet
private:
  cedar::proc::InputSlotHelper<cedar::aux::MatData> mImage;
  
  cedar::proc::InputSlotHelper<cedar::aux::MatData> mScaleVector;

  //!@brief The output data.
  cedar::aux::MatDataPtr mOutput;
  
  //! Lambdas used by the keypoint extraction.
  std::vector<double> mLambdas;

  //--------------------------------------------------------------------------------------------------------------------
  // parameters
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

private:
  cedar::aux::DoubleParameterPtr _mLambdasStart;

  cedar::aux::DoubleParameterPtr _mLambdasEnd;

  cedar::aux::UIntParameterPtr _mNumLambdas;

  cedar::aux::EnumParameterPtr _mScaleType;
  
  cedar::aux::UIntParameterPtr _mOutputSize;
  
  cedar::aux::UIntParameterPtr _mPadding;

}; // class cedar::proc::steps::KeypointPatchRescaler

#endif // CEDAR_PROC_STEPS_KEYPOINT_PATCH_RESCALER_H

