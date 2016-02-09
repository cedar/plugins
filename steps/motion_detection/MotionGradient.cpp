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

    File:        MotionGradient.cpp

    Maintainer:  Michael Berger
    Email:       michael.berger@ini.ruhr-uni-bochum.de
    Date:        2012 10 17

    Description:

    Credits:

======================================================================================================================*/

// LOCAL INCLUDES
#include "steps/motion_detection/MotionGradient.h"

// PROJECT INCLUDES

// SYSTEM INCLUDES
#include <cedar/processing/ExternalData.h>
#include <cedar/processing/DataSlot.h>
#include <cedar/auxiliaries/DataTemplate.h>
//#include <cedar/auxiliaries/assert.h>
#include <cedar/auxiliaries/math/tools.h>
#include <cedar/auxiliaries/MatData.h>
#include <opencv2/core/version.hpp>
#if CV_MAJOR_VERSION >= 3
#endif

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::proc::steps::MotionGradient::MotionGradient()
:
mOutMask(new cedar::aux::MatData(cv::Mat::zeros(1, 1, CV_8UC1))),
mOutOrient(new cedar::aux::MatData(cv::Mat::zeros(1, 1, CV_32F))),
_mSobelSize
(
  new cedar::aux::UIntParameter // has to be odd .. how?
  (
    this, // this tells the parameter what configurable it belongs to
    "Size of Sobel", // this is the name of the parameter;
    3, // this is the default value for the parameter;
    3, // lower limit
    7 // upper limit
  )
),
_mDelta1
(
  new cedar::aux::DoubleParameter
  (
    this, // this tells the parameter what configurable it belongs to
    "lower limit of gradient values", // this is the name of the parameter;
    0.001, // this is the default value for the parameter;
    0.001, // lower limit
    100 // upper limit
  )
),
_mDelta2
(
  new cedar::aux::DoubleParameter
  (
    this, // this tells the parameter what configurable it belongs to
    "upper limit of gradient values", // this is the name of the parameter;
    10, // this is the default value for the parameter;
    0.001, // lower limit
    100 // upper limit
  )
)
{
  //input declaration
  this->declareInput("input", true);
  //output declaration
  this->declareOutput("mask", mOutMask);
  this->declareOutput("orientation", mOutOrient);

  // inherit QObject and declare a slot to connect to the parameter's valueChanged() signal in the constructor
  QObject::connect(this->_mSobelSize.get(), SIGNAL(valueChanged()), this, SLOT(sobelSizeChanged()));
  QObject::connect(this->_mDelta1.get(), SIGNAL(valueChanged()), this, SLOT(delta1Changed()));
  QObject::connect(this->_mDelta2.get(), SIGNAL(valueChanged()), this, SLOT(delta2Changed()));
}

cedar::proc::steps::MotionGradient::~MotionGradient()
{
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

cedar::proc::DataSlot::VALIDITY cedar::proc::steps::MotionGradient::determineInputValidity
                                                            (
                                                              cedar::proc::ConstDataSlotPtr,
                                                              cedar::aux::ConstDataPtr data
                                                            ) const
{

  // check whether input is MatData
  if (cedar::aux::ConstMatDataPtr mat = boost::dynamic_pointer_cast<const cedar::aux::MatData>(data))
  {
    //input is valid
    return cedar::proc::DataSlot::VALIDITY_VALID;
  }
  else
  {
    return cedar::proc::DataSlot::VALIDITY_ERROR;
  }
}

void cedar::proc::steps::MotionGradient::inputConnectionChanged(const std::string& inputName)
{

  //set the input data to the member values and define the output matrices.
  if (inputName == "input")
  {
    this->mInput = boost::dynamic_pointer_cast<const cedar::aux::MatData>( this->getInput(inputName) );

    // check whether the input is a zero pointer due to cancellation of the connection
    if (!mInput)
    {
      return;
    }

    this->mOutMask->setData( this->mInput->getData().clone() ); // should be CV_8UC1 ... problem?
    this->mOutOrient->setData( this->mInput->getData().clone() );

    this->onTrigger();
  }

}

void cedar::proc::steps::MotionGradient::compute(const cedar::proc::Arguments&)
{

  // set ptr to the input data
  const cv::Mat& in = this->mInput->getData();
  // set ptr to the output data
  cv::Mat& out_mask = mOutMask->getData();
  cv::Mat& out_orient = mOutOrient->getData();

  // MotionGradient
//  cv::
//#if CV_MAJOR_VERSION >= 3
//  motempl::
//#endif
//  calcMotionGradient(in, out_mask, out_orient, this->getDelta1(), this->getDelta2(), this->getSobelSize());
}

void cedar::proc::steps::MotionGradient::sobelSizeChanged()
{
  // when the parameter changes, the output needs to be recalculated.
  this->onTrigger();
}

void cedar::proc::steps::MotionGradient::delta1Changed()
{
  // when the parameter changes, the output needs to be recalculated.
  this->onTrigger();
}

void cedar::proc::steps::MotionGradient::delta2Changed()
{
  // when the parameter changes, the output needs to be recalculated.
  this->onTrigger();
}
