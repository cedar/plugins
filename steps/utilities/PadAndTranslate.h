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

    File:        PadAndTranslate.h

    Maintainer:  Jan Tekülve
    Email:       jan.tekuelve@ini.rub.de
    Date:        2016 01 12

    Description: Header file for the class cedar::proc::steps::PadAndTranslate.

    Credits:

======================================================================================================================*/

#ifndef PROMOPLUGIN_PAD_AND_TRANSLATE_H
#define PROMOPLUGIN_PAD_AND_TRANSLATE_H

// CEDAR CONFIGURATION
#include "cedar/configuration.h"

// CEDAR INCLUDES
#include <cedar/processing/Step.h>
#include <cedar/auxiliaries/MatData.fwd.h>
#include "cedar/auxiliaries/IntParameter.h"

// FORWARD DECLARATIONS
#include "steps/utilities/PadAndTranslate.fwd.h"


// SYSTEM INCLUDES


/*!@todo describe.
 *
 * @todo describe more.
 */
class cedar::proc::steps::PadAndTranslate: public cedar::proc::Step
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
  PadAndTranslate();

  //!@brief Destructor
  virtual ~PadAndTranslate();

  //--------------------------------------------------------------------------------------------------------------------
  // public methods
  //--------------------------------------------------------------------------------------------------------------------
public:

public slots:
    void updateOutputSize();

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
    void compute(const cedar::proc::Arguments&);
    void reset();
    cedar::proc::DataSlot::VALIDITY determineInputValidity(cedar::proc::ConstDataSlotPtr slot,cedar::aux::ConstDataPtr data) const;


  //--------------------------------------------------------------------------------------------------------------------
  // members
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet
private:

  cedar::aux::IntParameterPtr  mOutputSize;
  cedar::aux::MatDataPtr mOutput;
  // none yet

  //--------------------------------------------------------------------------------------------------------------------
  // parameters
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

private:
  // none yet

}; // class cedar::proc::steps::PadAndTranslate

#endif // PROMOPLUGIN_PAD_AND_TRANSLATE_H

