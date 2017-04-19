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

    File:        MaxPooling.h

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2011 10 24

    Description:

    Credits:

======================================================================================================================*/

#ifndef CEDAR_PROC_STEPS_MAX_POOLING_H
#define CEDAR_PROC_STEPS_MAX_POOLING_H

// FORWARD HEADER
#include "steps/utilities/MaxPooling.fwd.h"

// CEDAR INCLUDES
#include <cedar/processing/Step.h>
#include <cedar/auxiliaries/MatData.h>
#include <cedar/auxiliaries/UIntParameter.h>
#include <cedar/auxiliaries/BoolVectorParameter.h>

// SYSTEM INCLUDES
#include <vector>


class cedar::proc::steps::MaxPooling : public cedar::proc::Step
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
  MaxPooling();

  //--------------------------------------------------------------------------------------------------------------------
  // public methods
  //--------------------------------------------------------------------------------------------------------------------
public:
  void compute(const cedar::proc::Arguments&);

  void inputConnectionChanged(const std::string& inputName);

  //--------------------------------------------------------------------------------------------------------------------
  // protected methods
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

  //--------------------------------------------------------------------------------------------------------------------
  // private methods
  //--------------------------------------------------------------------------------------------------------------------
private slots:
  void updateInternals();

  void poolSizeChanged();

  //--------------------------------------------------------------------------------------------------------------------
  // members
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

private:
  // inputs
  //!@todo These should be mat once ImageData inherits MatData
  cedar::aux::ConstMatDataPtr mInput;

  // outputs
  cedar::aux::MatDataPtr mOutput;

  //--------------------------------------------------------------------------------------------------------------------
  // parameters
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

private:
  cedar::aux::UIntParameterPtr mPoolSize;

  // these values are stored as members in order to improve calculation speed.
  std::vector<int> mIndices;
  std::vector<cv::Range> mRanges;

  cedar::aux::BoolVectorParameterPtr _mPooledDimensions;

}; // class cedar::proc::steps::MaxPooling

#endif // CEDAR_PROC_STEPS_MAX_POOLING_H
