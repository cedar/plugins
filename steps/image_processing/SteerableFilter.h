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

    File:        SteeringFilter.h

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2011 10 19

    Description:

    Credits:

======================================================================================================================*/

#ifndef CEDAR_PROC_STEPS_STEERABLE_FILTER_H
#define CEDAR_PROC_STEPS_STEERABLE_FILTER_H

// FORWARD DECLARATIONS
#include "steps/image_processing/SteerableFilter.fwd.h"
#include "kernels/SteerableKernel.fwd.h"

// PROJECT INCLUDES

// SYSTEM INCLUDES
#include <cedar/processing/Step.h>
#include <cedar/auxiliaries/convolution/Convolution.h>
#include <cedar/auxiliaries/MatData.h>


/*!@brief A filter that uses a steering kernel.
 *
 * @todo Adapt this to the new convolution classes.
 */
class cedar::proc::steps::SteerableFilter : public cedar::proc::Step
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
  SteerableFilter();

  //--------------------------------------------------------------------------------------------------------------------
  // public methods
  //--------------------------------------------------------------------------------------------------------------------
public:
  void compute(const cedar::proc::Arguments&);

  void inputConnectionChanged(const std::string& inputName);

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
  cedar::aux::ConstMatDataPtr mImage;

  // outputs
  cedar::aux::MatDataPtr mFilteredImage;

  // other members
  cedar::aux::kernel::SteerableKernelPtr mSteeringKernel;

  //! The convolution object.
  cedar::aux::conv::ConvolutionPtr mConvolution;

  //--------------------------------------------------------------------------------------------------------------------
  // parameters
  //--------------------------------------------------------------------------------------------------------------------
private:
  // none yet
}; // class cedar::proc::steps::SteerableFilter

#endif // CEDAR_PROC_STEPS_STEERABLE_FILTER_H
