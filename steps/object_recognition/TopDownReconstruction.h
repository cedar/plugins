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

    File:        TopDownReconstruction.h

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2016 02 16

    Description: Header file for the class shapeBasedRecognition::TopDownReconstruction.

    Credits:

======================================================================================================================*/

#ifndef CEDAR_PROC_STEPS_TOP_DOWN_RECONSTRUCTION_H
#define CEDAR_PROC_STEPS_TOP_DOWN_RECONSTRUCTION_H

// CEDAR CONFIGURATION
#include "cedar/configuration.h"

// CEDAR INCLUDES
#include <cedar/processing/Step.h>
#include <cedar/processing/InputSlotHelper.h>
#include <cedar/auxiliaries/DoubleParameter.h>
#include <cedar/auxiliaries/MatData.h>

// FORWARD DECLARATIONS
#include "steps/object_recognition/TopDownReconstruction.fwd.h"

// SYSTEM INCLUDES


/*!@todo describe.
 *
 * @todo describe more.
 */
class cedar::proc::steps::TopDownReconstruction : public cedar::proc::Step
{
  //--------------------------------------------------------------------------------------------------------------------
  // nested types
  //--------------------------------------------------------------------------------------------------------------------

  //--------------------------------------------------------------------------------------------------------------------
  // constructors and destructor
  //--------------------------------------------------------------------------------------------------------------------
public:
  //!@brief The standard constructor.
  TopDownReconstruction();

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

  void computeTopDownGradients();

  void calculateReconstructionError();

  //--------------------------------------------------------------------------------------------------------------------
  // members
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet
private:
  cedar::proc::InputSlotHelper<cedar::aux::MatData> mImage;

  cedar::proc::InputSlotHelper<cedar::aux::MatData> mTopDownPrediction;

  cedar::proc::InputSlotHelper<cedar::aux::MatData> mGradientOrientationImage;

  cedar::proc::InputSlotHelper<cedar::aux::MatData> mGradientMagnitudeImage;

  // buffers
  cedar::aux::MatDataPtr mReconstructedOrientations;
  cedar::aux::MatDataPtr mReconstructedMagnitudes;
  cedar::aux::MatDataPtr mReconstructionError;

  // outputs
  cedar::aux::MatDataPtr mReconstructedEdges;

  //--------------------------------------------------------------------------------------------------------------------
  // parameters
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

private:
  cedar::aux::DoubleParameterPtr _mReconstructionMagnitudeThreshold;

  cedar::aux::DoubleParameterPtr _mGradientMagnitudeThreshold;

}; // class cedar::proc::steps::TopDownReconstruction

#endif // CEDAR_PROC_STEPS_TOP_DOWN_RECONSTRUCTION_H

