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

    File:        ViewCombination.h

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2014 04 10

    Description: Header file for the class recognition::ViewCombination.

    Credits:

======================================================================================================================*/

#ifndef CEDAR_PROC_STEPS_VIEW_COMBINATION_H
#define CEDAR_PROC_STEPS_VIEW_COMBINATION_H

// CEDAR CONFIGURATION
#include "cedar/configuration.h"

// CEDAR INCLUDES
#include <cedar/processing/Step.h>
#include <cedar/auxiliaries/UIntParameter.h>

// FORWARD DECLARATIONS
#include "steps/object_recognition/ViewCombination.fwd.h"
#include <cedar/auxiliaries/MatData.fwd.h>

// SYSTEM INCLUDES


/*!@brief A class that takes the output from a label field and combines multiple different label activations that
 *        represent the same object from different views into a single activation.
 */
class cedar::proc::steps::ViewCombination : public cedar::proc::Step
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
  ViewCombination();

  //--------------------------------------------------------------------------------------------------------------------
  // public methods
  //--------------------------------------------------------------------------------------------------------------------
public:
  void setNumberOfViews(unsigned int numViews);

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

  void reallocateOutputMatrix();

  unsigned int getNumberOfLabels() const;

  unsigned int getNumberOfObjects() const;

  inline unsigned int getNumberOfViews() const
  {
    return this->_mNumberOfViews->getValue();
  }

  void inputConnectionChanged(const std::string& inputName);

private slots:
  void numberOfViewsChanged();

  void reverseChanged();

  //--------------------------------------------------------------------------------------------------------------------
  // members
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet
private:
  cedar::aux::ConstMatDataPtr mLabelActivation;

  cedar::aux::MatDataPtr mObjectActivation;

  //--------------------------------------------------------------------------------------------------------------------
  // parameters
  //--------------------------------------------------------------------------------------------------------------------
private:
  cedar::aux::UIntParameterPtr _mNumberOfViews;

  cedar::aux::BoolParameterPtr _mReverse;

}; // cedar::proc::steps::ViewCombination

#endif // CEDAR_PROC_STEPS_VIEW_COMBINATION_H

