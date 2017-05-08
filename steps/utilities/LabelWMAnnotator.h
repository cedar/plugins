/*======================================================================================================================

    Copyright 2011, 2012, 2013, 2014, 2015, 2016 Institut fuer Neuroinformatik, Ruhr-Universitaet Bochum, Germany
 
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

    File:        LabelWMAnnotator.h

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2015 06 29

    Description: Header file for the class cedar::proc::steps::LabelWMAnnotator.

    Credits:

======================================================================================================================*/

#ifndef CEDAR_PROC_STEPS_LABEL_WMANNOTATOR_H
#define CEDAR_PROC_STEPS_LABEL_WMANNOTATOR_H

// CEDAR CONFIGURATION
#include "cedar/configuration.h"

// CEDAR INCLUDES
#include <cedar/processing/Step.h>
#include <cedar/processing/InputSlotHelper.h>
#include <cedar/auxiliaries/MatData.h>
#include <cedar/auxiliaries/StringVectorParameter.h>
#include <cedar/auxiliaries/DoubleParameter.h>
#include <cedar/auxiliaries/IntParameter.h>
#include <cedar/auxiliaries/UIntParameter.h>

// FORWARD DECLARATIONS
#include "steps/utilities/LabelWMAnnotator.fwd.h"

// SYSTEM INCLUDES


/*!@todo describe.
 *
 * @todo describe more.
 */
class cedar::proc::steps::LabelWMAnnotator : public cedar::proc::Step
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
  LabelWMAnnotator();

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

private slots:
  void numberOfStringsChanged();

  //--------------------------------------------------------------------------------------------------------------------
  // members
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet
private:
  //!@brief The input image.
  cedar::proc::InputSlotHelper<cedar::aux::MatData> mImage;

  cedar::proc::InputSlotHelper<cedar::aux::MatData> mWMToSpace;

  cedar::proc::InputSlotHelper<cedar::aux::MatData> mRateCodedLabels;

  //!@brief The output image.
  cedar::aux::MatDataPtr mOutput;

  //--------------------------------------------------------------------------------------------------------------------
  // parameters
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

private:
  cedar::aux::UIntParameterPtr _mNumberOfStrings;

  cedar::aux::StringVectorParameterPtr _mStrings;

  cedar::aux::DoubleParameterPtr _mThreshold;

  cedar::aux::DoubleParameterPtr _mFontScale;

  cedar::aux::IntParameterPtr _mFontThickness;

  cedar::aux::DoubleParameterPtr _mCrossRadius;

}; // class cedar::proc::steps::LabelWMAnnotator

#endif // CEDAR_PROC_STEPS_LABEL_WMANNOTATOR_H

