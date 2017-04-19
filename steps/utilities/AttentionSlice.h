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

    File:        AttentionSlice.h

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2014 08 01

    Description:

    Credits:     Originally written by Philipp Hebing.

======================================================================================================================*/

#ifndef CEDAR_PROC_STEPS_ATTENTION_SLICE_H
#define CEDAR_PROC_STEPS_ATTENTION_SLICE_H

#include "steps/utilities/AttentionSlice.fwd.h"

#include <cedar/processing/Step.h> // if we are going to inherit from cedar::proc::Step, we have to include the header
#include <cedar/auxiliaries/IntParameter.h>

// CEDAR INCLUDES
#include <cedar/processing/ExternalData.h> // getInputSlot() returns ExternalData
#include <cedar/auxiliaries/MatData.h> // this is the class MatData, used internally in this step
#include <cedar/auxiliaries/BoolParameter.h>

#include <cedar/processing/ElementDeclaration.h>
#include <cedar/processing/DeclarationRegistry.h>

#include <cedar/processing/typecheck/DerivedFrom.h>

#include <cedar/auxiliaries/exceptions.fwd.h>

class cedar::proc::steps::AttentionSlice: public cedar::proc::Step
{
	//--------------------------------------------------------------------------------------------------------------------
  // macros
  //--------------------------------------------------------------------------------------------------------------------
  Q_OBJECT
  
  //--------------------------------------------------------------------------------------------------------------------
  // constructors and destructor
  //--------------------------------------------------------------------------------------------------------------------
public:
	AttentionSlice();
	
	//--------------------------------------------------------------------------------------------------------------------
  // public methods
  //--------------------------------------------------------------------------------------------------------------------
public:
	cv::Point getCorrectedAttention();

	//--------------------------------------------------------------------------------------------------------------------
  // protected methods
  //--------------------------------------------------------------------------------------------------------------------  
protected:
	void inputConnectionChanged(const std::string&);

  //--------------------------------------------------------------------------------------------------------------------
  // private methods
  //--------------------------------------------------------------------------------------------------------------------
private:
	void updateOutput();

  void compute(const cedar::proc::Arguments&);

private slots:
	void rangeChanged();

	//--------------------------------------------------------------------------------------------------------------------
  // members
  //--------------------------------------------------------------------------------------------------------------------
private:
	cedar::aux::MatDataPtr mOutput;
	
	//! Contains an activation map in which the center of the cut-out region has activity one, and the rest zero.
  cedar::aux::MatDataPtr mWTAActivation;
  
   //! Parameter used for determining the width of the matrix slice.
  float mLastAttentionCenterX;

  //! Parameter used for determining the height of the matrix slice.
  float mLastAttentionCenterY;

  //--------------------------------------------------------------------------------------------------------------------
  // parameters
  //--------------------------------------------------------------------------------------------------------------------
private:

  //! Parameter used for determining the width of the matrix slice.
  cedar::aux::IntParameterPtr _mWidth;

  //! Parameter used for determining the height of the matrix slice.
  cedar::aux::IntParameterPtr _mHeight;

  //! Whether or not the input should be padded prior to cutting out the slice
  cedar::aux::BoolParameterPtr _mPadInput;
};

#endif // CEDAR_PROC_STEPS_ATTENTION_SLICE_H
