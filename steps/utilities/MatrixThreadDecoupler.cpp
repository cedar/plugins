/*======================================================================================================================

    Copyright 2011, 2012, 2013, 2014 Institut fuer Neuroinformatik, Ruhr-Universitaet Bochum, Germany
 
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

    File:        MatrixThreadDecoupler.cpp

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2014 09 30

    Description: Source file for the class cedar::proc::steps::MatrixThreadDecoupler.

    Credits:

======================================================================================================================*/

// CEDAR CONFIGURATION
#include "cedar/configuration.h"
#include "cedar/processing/typecheck/Matrix.h"

// CEDAR INCLUDES
#include "steps/utilities/MatrixThreadDecoupler.h"

// SYSTEM INCLUDES

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::proc::steps::MatrixThreadDecoupler::MatrixThreadDecoupler()
:
cedar::proc::Step(true),
mInput(this, "input"),
mOutput(new cedar::aux::MatData(cv::Mat()))
{
  this->setAutoLockInputsAndOutputs(false);

  cedar::proc::typecheck::Matrix check;
  this->mInput.getSlot()->setCheck(check);

  auto output_slot = this->declareOutput("copy", this->mOutput);
  this->mInput.addOutputRelation(cedar::proc::CopyMatrixPropertiesPtr(new cedar::proc::CopyMatrixProperties(output_slot)));
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

void cedar::proc::steps::MatrixThreadDecoupler::compute(const cedar::proc::Arguments&)
{
  QReadLocker r_locker(&this->mInput.get()->getLock());
  cv::Mat copy = this->mInput.getData().clone();
  r_locker.unlock();

  QWriteLocker w_locker(&this->mOutput->getLock());
  this->mOutput->setData(copy);
}
