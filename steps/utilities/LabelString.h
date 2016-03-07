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

    File:        LabelString.h

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2015 06 09

    Description: Header file for the class utilities::LabelString.

    Credits:

======================================================================================================================*/

#ifndef CEDAR_PROC_STEPS_LABEL_STRING_H
#define CEDAR_PROC_STEPS_LABEL_STRING_H

// CEDAR CONFIGURATION
#include "cedar/configuration.h"

// CEDAR INCLUDES
#include <cedar/processing/Step.h>
#include <cedar/processing/InputSlotHelper.h>
#include <cedar/auxiliaries/StringVectorParameter.h>
#include <cedar/auxiliaries/DoubleParameter.h>
#include <cedar/auxiliaries/UIntParameter.h>
#include "data_structures/StringData.h"

// FORWARD DECLARATIONS
#include "steps/utilities/LabelString.fwd.h"

// SYSTEM INCLUDES


/*!@brief Extracts the maximum from an input vector and outputs a string corresponding to the index of that maximum.
 */
class cedar::proc::steps::LabelString : public cedar::proc::Step
{
  Q_OBJECT

  //--------------------------------------------------------------------------------------------------------------------
  // nested types
  //--------------------------------------------------------------------------------------------------------------------
public:

  //--------------------------------------------------------------------------------------------------------------------
  // constructors and destructor
  //--------------------------------------------------------------------------------------------------------------------
public:
  //!@brief The standard constructor.
  LabelString();

  //--------------------------------------------------------------------------------------------------------------------
  // public methods
  //--------------------------------------------------------------------------------------------------------------------
public:
  void setNumberOfStrings(unsigned int numberOfStrings);

  void setString(unsigned int stringNumber, const std::string& string);

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

private slots:
  void numberOfStringsChanged();

  //--------------------------------------------------------------------------------------------------------------------
  // members
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet
private:
  cedar::proc::InputSlotHelper<cedar::aux::MatData> mInput;

  cedar::aux::StringDataPtr mOutput;

  cedar::aux::StringDataPtr mRankedList;

  //--------------------------------------------------------------------------------------------------------------------
  // parameters
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

private:
  cedar::aux::UIntParameterPtr _mNumberOfStrings;

  cedar::aux::StringVectorParameterPtr _mStrings;

  cedar::aux::DoubleParameterPtr _mThreshold;

}; // class cedar::proc::steps::LabelString

#endif // CEDAR_PROC_STEPS_LABEL_STRING_H

