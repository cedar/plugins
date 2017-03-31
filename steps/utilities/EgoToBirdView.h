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

    File:        EgoToBirdView.h

    Maintainer:  Tutorial Writer Person
    Email:       cedar@ini.rub.de
    Date:        2011 12 09

    Description:

    Credits:

======================================================================================================================*/

#ifndef EGO_TO_BIRD_VIEW
#define EGO_TO_BIRD_VIEW

// CEDAR INCLUDES
#include <cedar/processing/Step.h> // if we are going to inherit from cedar::proc::Step, we have to include the header

// FORWARD DECLARATIONS
#include "steps/utilities/EgoToBirdView.fwd.h"
#include <cedar/auxiliaries/MatData.fwd.h>
#include <cedar/auxiliaries/UIntVectorParameter.h>
#include <cedar/auxiliaries/DoubleParameter.h>

// SYSTEM INCLUDES
#include <math.h>



class cedar::proc::steps::EgoToBirdView : public cedar::proc::Step
{
Q_OBJECT
  //--------------------------------------------------------------------------------------------------------------------
  // constructors and destructor
  //--------------------------------------------------------------------------------------------------------------------
public:
  //!@brief The standard constructor.
  EgoToBirdView();

  //!@brief Destructor

  //--------------------------------------------------------------------------------------------------------------------
  // public methods
  //--------------------------------------------------------------------------------------------------------------------
public slots:
  void updateOutputSize();

  //--------------------------------------------------------------------------------------------------------------------
  // protected methods
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

  //--------------------------------------------------------------------------------------------------------------------
  // private methods
  //--------------------------------------------------------------------------------------------------------------------
private:
  // The arguments are unused here
  void compute(const cedar::proc::Arguments&);
  std::vector<int> retrieveBlobPositions(cv::Mat);
  cedar::proc::DataSlot::VALIDITY determineInputValidity(cedar::proc::ConstDataSlotPtr slot,cedar::aux::ConstDataPtr data) const;
  void reset();


  //--------------------------------------------------------------------------------------------------------------------
  // members
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet
private:
  //!@brief this is the output of the computation (in this case, the summed inputs)
  cedar::aux::UIntVectorParameterPtr  mOutputSize;
  cedar::aux::DoubleParameterPtr mMeterToField;
  cedar::aux::DoubleParameterPtr mAgentPosInFieldX;
  cedar::aux::DoubleParameterPtr mAgentPosInFieldY;
  cedar::aux::MatDataPtr mOutput;
  const double pi = 3.14159265;

  //--------------------------------------------------------------------------------------------------------------------
  // parameters
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

private:
  // none yet

}; // class EgoToBirdView

#endif // EGO_TO_BIRD_VIEW

