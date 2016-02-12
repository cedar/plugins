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

    File:        ElementwiseDivide.cpp

    Maintainer:  Michael Berger
    Email:       michael.berger@ini.ruhr-uni-bochum.de
    Date:        2012 08 28

    Description:

    Credits:

======================================================================================================================*/

// LOCAL INCLUDES
#include "steps/utilities/ElementwiseDivide.h"

// PROJECT INCLUDES

// SYSTEM INCLUDES
#include <cedar/processing/ExternalData.h>
#include <cedar/processing/DataSlot.h>
#include <cedar/auxiliaries/DataTemplate.h>
#include <cedar/auxiliaries/math/tools.h>
#include <cedar/auxiliaries/MatData.h>

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::proc::steps::ElementwiseDivide::ElementwiseDivide()
:
mOutput(new cedar::aux::MatData(cv::Mat::zeros(1, 1, CV_32F))),
_mDenomAdd
(
  new cedar::aux::DoubleParameter
  (
    this, // this tells the parameter what configurable it belongs to
    "Value Added to the Denominator ", // this is the name of the parameter;
    0.001, // this is the default value for the parameter;
    -10, // lower limit
    10 // upper limit
  )
)
{
  //input declaration
  this->declareInput("nominator", true);
  this->declareInput("denominator", true);
  //output declaration
  this->declareOutput("output", mOutput);

  // inherit QObject and declare a slot to connect to the parameter's valueChanged() signal in the constructor
  QObject::connect(this->_mDenomAdd.get(), SIGNAL(valueChanged()), this, SLOT(denomAddChanged()));
}

cedar::proc::steps::ElementwiseDivide::~ElementwiseDivide()
{
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

cedar::proc::DataSlot::VALIDITY cedar::proc::steps::ElementwiseDivide::determineInputValidity
                                                            (
                                                              cedar::proc::ConstDataSlotPtr slot,
                                                              cedar::aux::ConstDataPtr data
                                                            ) const
{

  // check whether input is MatData
  if (cedar::aux::ConstMatDataPtr mat = boost::dynamic_pointer_cast<const cedar::aux::MatData>(data))
  {
    // check whether both input matrices have the same size if the other input is already declared
    if ( slot == this->getInputSlot("nominator") && mDenom && !cedar::aux::math::matrixSizesEqual(mDenom->getData(),mat->getData()) )
    {
      return cedar::proc::DataSlot::VALIDITY_ERROR;
    }

    if ( slot == this->getInputSlot("denominator") && mNom && !cedar::aux::math::matrixSizesEqual(mNom->getData(),mat->getData()) )
    {
      return cedar::proc::DataSlot::VALIDITY_ERROR;
    }

    //input is valid
    return cedar::proc::DataSlot::VALIDITY_VALID;
  }
  else
  {
    return cedar::proc::DataSlot::VALIDITY_ERROR;
  }
}

void cedar::proc::steps::ElementwiseDivide::inputConnectionChanged(const std::string& inputName)
{

  //set the input data to the member values and define the output matrices.
  if (inputName == "nominator")
  {
    this->mNom = boost::dynamic_pointer_cast<const cedar::aux::MatData>( this->getInput(inputName) );

    // check whether the input is a zero pointer due to cancellation of the connection
    if (!mNom)
    {
      return;
    }

    this->mOutput->setData( this->mNom->getData().clone() );

    this->onTrigger();
  }

  if (inputName == "denominator")
  {
    this->mDenom = boost::dynamic_pointer_cast<const cedar::aux::MatData>( this->getInput(inputName) );

    if (!mDenom)
    {
      return;
    }

    this->mOutput->setData( this->mDenom->getData().clone() );

    this->onTrigger();
  }

}

void cedar::proc::steps::ElementwiseDivide::compute(const cedar::proc::Arguments&)
{

  // set ptr to the input data
  const cv::Mat& nom = this->mNom->getData();
  const cv::Mat& denom = this->mDenom->getData();
  // set ptr to the output data
  cv::Mat& out = mOutput->getData();

  // division
  out = nom / ( denom + this->getDenomAdd() );

}

void cedar::proc::steps::ElementwiseDivide::denomAddChanged()
{
  // when the parameter changes, the output needs to be recalculated.
  this->onTrigger();
}
