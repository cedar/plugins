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

 File:        2DTo2DHebbMap.cpp

 Maintainer:  Jan Tekülve
 Email:       jan.tekuelve@ini.rub.de
 Date:        2016 10 28

 Description: Source file for the class cedar::proc::steps::2DTo2DHebbMap.

 Credits:

 ======================================================================================================================*/

// CEDAR CONFIGURATION
#include "cedar/configuration.h"

// CLASS HEADER
#include "steps/sources/OpenGLTargetVisualisation.h"

#include "cedar/processing/typecheck/IsMatrix.h"
#include "cedar/processing/DataSlot.h"
#include "cedar/processing/ElementDeclaration.h"
#include "cedar/processing/DeclarationRegistry.h"
#include "cedar/auxiliaries/assert.h"
#include "cedar/auxiliaries/exceptions.h"
#include "cedar/units/Time.h"
#include "cedar/units/prefixes.h"
#include "cedar/processing/steps/Sum.h"

// SYSTEM INCLUDES
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------
cedar::proc::sources::OpenGLTargetVisualisation::OpenGLTargetVisualisation()
        :
// parameters
        mImageSizes(new cedar::aux::UIntVectorParameter(this, "image sizes", 2, 50)),
        mObjectCount(new cedar::aux::UIntParameter(this, "object count", 1, 0, 10)),
        mXPositions(
                new cedar::aux::DoubleVectorParameter(this, "x positions", 1,2)),
        mYPositions(
                new cedar::aux::DoubleVectorParameter(this, "y positions", 1,2)),
        mZPosition(new cedar::aux::DoubleParameter(this, "z position", 0.001)),
        mImageOutput((new cedar::aux::MatData(
                cv::Mat::zeros(mImageSizes->getValue().at(0), mImageSizes->getValue().at(1), CV_32F))))
{
  // declare all data
  auto weightTriggerOutput = this->declareOutput(mImageOutputName, mImageOutput);

//  QObject::connect(mAssociationSizes.get(), SIGNAL(valueChanged()), this, SLOT(resetWeights()));
}
//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

void cedar::proc::sources::OpenGLTargetVisualisation::compute(const cedar::proc::Arguments&)
{
  //not yet implemented!
}