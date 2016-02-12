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

    File:        CounterChangeCombination.cpp

    Maintainer:  Michael Berger

    Email:       michael.berger@ini.ruhr-uni-bochum.de

    Date:        2012 08 09

    Description: Combination of toward and away signals, providing input for a counterchange motion detection field.
                 For further explanations please look at the plugin documentation.

    Credits:

======================================================================================================================*/

// LOCAL INCLUDES
#include "steps/motion_detection/CounterChangeCombination.h"

// PROJECT INCLUDES

// SYSTEM INCLUDES
#include <cedar/auxiliaries/math/tools.h>
#include <cedar/auxiliaries/MatData.h>
#include <cedar/auxiliaries/assert.h>
#include <cedar/processing/ExternalData.h> // getInputSlot() returns ExternalData
#include <iostream>

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::proc::steps::CounterChangeCombination::CounterChangeCombination()
:
mConvolution(new cedar::aux::conv::Convolution()),
mOutputBtW(new cedar::aux::MatData(cv::Mat::zeros(1, 1, CV_32F))),
mOutputWtB(new cedar::aux::MatData(cv::Mat::zeros(1, 1, CV_32F))),
_mShift
(
  new cedar::aux::UIntParameter
  (
    this, // this tells the parameter what configurable it belongs to
    "shift size", // this is the name of the parameter;
    5, // this is the default value for the parameter;
    cedar::aux::UIntParameter::LimitType::positiveZero(500) // this are the upper and lower limits of the parameter
  )
)
{
  // configuration
  this->addConfigurableChild("shift per convolution", this->mConvolution);

  // declare transient input
  this->declareInput("away", true);
  this->declareInput("toward", true);

  // declare output
  this->declareOutput("WhiteToBlack", mOutputWtB);
  this->declareOutput("BlackToWhite", mOutputBtW);

  // inherit QObject and declare a slot to connect to the parameter's valueChanged() signal in the constructor
  QObject::connect(_mShift.get(), SIGNAL(valueChanged()), this, SLOT(shiftChanged()));
}

cedar::proc::steps::CounterChangeCombination::~CounterChangeCombination()
{
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

cedar::proc::DataSlot::VALIDITY cedar::proc::steps::CounterChangeCombination::determineInputValidity
                                                            (
                                                              cedar::proc::ConstDataSlotPtr slot,
                                                              cedar::aux::ConstDataPtr data
                                                            ) const
{
  // check whether input is MatData (or at least whether it can be converted to it !?)
  if(cedar::aux::ConstMatDataPtr mat_data = boost::dynamic_pointer_cast<const cedar::aux::MatData>(data))
  {
    // check whether toward and away input have a dimensionality of 3
    if ( cedar::aux::math::getDimensionalityOf( mat_data->getData()) == 3 )
    {
      // check whether both input matrices have the same size if the other input is alreay declared
      if ( slot == this->getInputSlot("away") && mToward && !cedar::aux::math::matrixSizesEqual(mToward->getData(),mat_data->getData()) )
      {
        return cedar::proc::DataSlot::VALIDITY_ERROR;
      }

      if ( slot == this->getInputSlot("toward") && mAway && !cedar::aux::math::matrixSizesEqual(mAway->getData(),mat_data->getData()) )
      {
        return cedar::proc::DataSlot::VALIDITY_ERROR;
      }

      // input is valid
      return cedar::proc::DataSlot::VALIDITY_VALID;

    }
  }
  return cedar::proc::DataSlot::VALIDITY_ERROR;
}

void cedar::proc::steps::CounterChangeCombination::inputConnectionChanged(const std::string& inputName)
{
	//set the input data to the member values and define the output matrices.
	if (inputName == "toward")
  {
    this->mToward = boost::dynamic_pointer_cast<const cedar::aux::MatData>(this->getInput(inputName));

    if (!mToward)
    {
      return;
    }

    this->mOutputBtW->setData(this->mToward->getData().clone() );

    this->onTrigger();
  }
  
  if (inputName == "away")
  {
    this->mAway = boost::dynamic_pointer_cast<const cedar::aux::MatData>(this->getInput(inputName));

    if (!mAway)
    {
      return;
    }

    this->mOutputWtB->setData(this->mAway->getData().clone() );

    this->onTrigger();
  }
}

void cedar::proc::steps::CounterChangeCombination::compute(const cedar::proc::Arguments&)
{
  // in and out data
  const cv::Mat& away = this->mAway->getData();
  const cv::Mat& toward = this->mToward->getData();
  cv::Mat& BtW = this->mOutputBtW->getData();
  cv::Mat& WtB = this->mOutputWtB->getData();

  // number of Orientations
  int nOrient = away.size[0];

  // iteration over first input dimension (orientation)
  for (int i=0; i<nOrient; i++)
  {
    // input slices
    void* ptr_in_toward = toward.data +  toward.step[0]*i;
    void* ptr_in_away = away.data +  away.step[0]*i;
    cv::Mat slice_in_toward = cv::Mat( toward.size[1], toward.size[2], toward.type(), ptr_in_toward );
    cv::Mat slice_in_away = cv::Mat( away.size[1], away.size[2], away.type(), ptr_in_away );


    // index of output slice have to be combined according to the counterchange rule in that way
    // that the first direction is rightward motion and the following are rotated counter-clockwise
    // White to Black: index matches with the edge orientation
    void* ptr_out_WtB = WtB.data +  WtB.step[0]*i;
    cv::Mat slice_out_WtB = cv::Mat( WtB.size[1], WtB.size[2], WtB.type(), ptr_out_WtB );
    // Black to White: the index is circularly shifted by half of the number of orientations
    // index
    int j;
    if ( i < nOrient/2 )
    {
      j = i+floor(nOrient/2);
    }
    else
    {
      j = i-ceil(nOrient/2);
    }
    // output slices
    void* ptr_out_BtW = BtW.data +  BtW.step[0]*j;
    cv::Mat slice_out_BtW = cv::Mat( BtW.size[1], BtW.size[2], BtW.type(), ptr_out_BtW );

    // define shift position
    const double pi = 3.141592653589793;
    int x_shift = round( cos( i*2*pi/nOrient + pi/2 ) * this->getShift() / 2 );
    int y_shift = round( sin( i*2*pi/nOrient + pi/2 ) * this->getShift() / 2 );

    // define shift kernel
    //@todo: make separable, do it when there is an easy way in the framework ... or think about a more efficient way of shifting
    cv::Mat pos_shift_kernel;
    cv::Mat neg_shift_kernel;
    // separate between odd an even shift sizes
    if ( this->getShift()%2 == 1 ) // odd
    {
      pos_shift_kernel = cv::Mat::zeros( this->getShift()+2, this->getShift()+2, BtW.type() );
      pos_shift_kernel.at<float>( (this->getShift()+1)/2  + x_shift, (this->getShift()+1)/2 + y_shift ) = 1;
      neg_shift_kernel = cv::Mat::zeros( this->getShift()+2, this->getShift()+2, BtW.type() );
      neg_shift_kernel.at<float>( (this->getShift()+1)/2  - x_shift, (this->getShift()+1)/2 - y_shift ) = 1;
    }
    else // even
    {
      pos_shift_kernel = cv::Mat::zeros( this->getShift()+1, this->getShift()+1, BtW.type() );
      pos_shift_kernel.at<float>( (this->getShift())/2  + x_shift, (this->getShift())/2 + y_shift ) = 1;
      neg_shift_kernel = cv::Mat::zeros( this->getShift()+1, this->getShift()+1, BtW.type() );
      neg_shift_kernel.at<float>( (this->getShift())/2  - x_shift, (this->getShift())/2 - y_shift ) = 1;
    }

    // define convolution output
    cv::Mat pos_shifted_away = cv::Mat( away.size[1], away.size[2], away.type());
    cv::Mat neg_shifted_away = cv::Mat( away.size[1], away.size[2], away.type());
    cv::Mat pos_shifted_toward = cv::Mat( toward.size[1], toward.size[2], toward.type());
    cv::Mat neg_shifted_toward = cv::Mat( toward.size[1], toward.size[2], toward.type());

    // convolution
    mConvolution->convolve( slice_in_toward, pos_shift_kernel ).copyTo( pos_shifted_toward );
    mConvolution->convolve( slice_in_toward, neg_shift_kernel ).copyTo( neg_shifted_toward );
    mConvolution->convolve( slice_in_away, pos_shift_kernel ).copyTo( pos_shifted_away );
    mConvolution->convolve( slice_in_away, neg_shift_kernel ).copyTo( neg_shifted_away );

    // combine the shifted toward and away signals to the desired output
    slice_out_BtW = neg_shifted_toward + pos_shifted_away;
    slice_out_WtB = pos_shifted_toward + neg_shifted_away;
  }
}

void cedar::proc::steps::CounterChangeCombination::shiftChanged()
{
  // the output needs to be recalculated.
  this->onTrigger();
}
