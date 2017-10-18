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

    File:        AttentionSlice.cpp

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2014 08 01

    Description:

    Credits:     Originally written by Philipp Hebing.

======================================================================================================================*/


#include "steps/utilities/AttentionSlice.h"
#include "cedar/processing/Arguments.h"
#include "cedar/processing/typecheck/Matrix.h"



cedar::proc::steps::AttentionSlice::AttentionSlice()
: // <- the colon starts the member initialization list
mOutput(new cedar::aux::MatData(cv::Mat::zeros(1, 1, CV_32F))),
mWTAActivation(new cedar::aux::MatData()),
mLastAttentionCenterX(-1.0f),
mLastAttentionCenterY(-1.0f),
_mWidth
(
  new cedar::aux::IntParameter
  (
    this,
    "width",
    50,
    cedar::aux::IntParameter::LimitType::positive()
  )
),
_mHeight
(
  new cedar::aux::IntParameter
  (
    this,
    "height",
    50,
    cedar::aux::IntParameter::LimitType::positive()
  )
),
_mPadInput
(
  new cedar::aux::BoolParameter
  (
    this,
    "pad input",
    false
  )
)
{
	/* Declare both inputs; the "true" means that the inputs are mandatory, i.e.,
     the step will not run unless both of the inputs are connected to data.
	 */
	cedar::proc::typecheck::Matrix image_check;
	auto image_slot = this->declareInput("image", true);
	image_slot->setCheck(image_check);
	this->declareInput("attentionCenterX", true);
	this->declareInput("attentionCenterY", true);

	// store a pointer to the slot
	//cedar::proc::DataSlotPtr operands = this->declareInputCollection("operands");

	// tell the slot what to expect
	//operands->setCheck(cedar::proc::typecheck::DerivedFrom<cedar::aux::MatData>());


	// Declare the output and set it to the output matrix defined above.
	this->declareOutput("cutout_region", mOutput);

	this->declareOutput("WTA activation", mWTAActivation);

/*
  auto input = this->declareInput("matrix");
  cedar::proc::typecheck::Matrix input_check;
  input_check.addAcceptedDimensionalityRange(1, 16);
  input->setCheck(input_check);

  this->declareOutput("slice", mOutput);
*/
 

  QObject::connect(this->_mWidth.get(), SIGNAL(valueChanged()), this, SLOT(rangeChanged()));
  QObject::connect(this->_mHeight.get(), SIGNAL(valueChanged()), this, SLOT(rangeChanged()));

}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

void cedar::proc::steps::AttentionSlice::compute(const cedar::proc::Arguments&)
{
  cedar::proc::ExternalDataPtr input_slot_image = this->getInputSlot("image");
  cedar::proc::ExternalDataPtr input_slot_X = this->getInputSlot("attentionCenterX");
  cedar::proc::ExternalDataPtr input_slot_Y = this->getInputSlot("attentionCenterY");

  if (input_slot_image->getDataCount() <= 0)
  {
    return;
  }
  if (input_slot_X->getDataCount() <= 0)
  {
    return;
  }
  if (input_slot_Y->getDataCount() <= 0)
  {
    return;
  }


  //calculate image borders
  int width = _mWidth->getValue();
  int height = _mHeight->getValue();
  int img_rows = input_slot_image->getData()->getData<cv::Mat>().rows;
  int img_cols = input_slot_image->getData()->getData<cv::Mat>().cols;
  
  this->mWTAActivation->getData().create(img_rows, img_cols, CV_32F);
  this->mWTAActivation->getData() = 0.0;

  float x = input_slot_X->getData()->getData<cv::Mat>().at<float>(0);
  float y = input_slot_Y->getData()->getData<cv::Mat>().at<float>(0);
  this->mLastAttentionCenterX = x;
  this->mLastAttentionCenterY = y;

  int pad_left = 0;
  int pad_right = 0;

  bool pad = this->_mPadInput->getValue();

  //left
  int left = std::round(x * static_cast<float>(img_cols) - static_cast<float>(width)/2.f);
  if (left < 0)
  {
    if (pad)
    {
      pad_left = -left;
    }
    left = 0;
  }
  if ((left + width) > img_cols) //check bounds
  {
    if (pad)
    {
      pad_right = (left + width) - img_cols;
    }
    else
    {
      left = img_cols-width;
    }

  	if (left < 0)
  	{  
  		CEDAR_THROW(cedar::aux::RangeException, "Error: width out of bounds");
			//return;
  	}
  }
  
	//top
  int pad_top = 0;
  int pad_bottom = 0;

  int top = std::round(y * static_cast<float>(img_rows) - static_cast<float>(height)/2.0f);
  if (top < 0)
  {
    if (pad)
    {
      pad_top = -top;
    }
    top = 0;
  }
  if ((top + height) > img_rows) //check bounds
  {
    if (pad)
    {
      pad_bottom = (top + height) - img_rows;
    }
    else
    {
      top = img_rows - height;
    }

  	if (top < 0)
  	{
			CEDAR_THROW(cedar::aux::RangeException, "Error: height out of bounds");
  	}
  }
  
  cv::Mat input;
  if (pad && (pad_left != 0 || pad_right != 0 || pad_top != 0 || pad_bottom != 0))
  {
    cv::copyMakeBorder(input_slot_image->getData()->getData<cv::Mat>(), input, pad_top, pad_bottom, pad_left, pad_right, cv::BORDER_REPLICATE);
  }
  else
  {
    input = input_slot_image->getData()->getData<cv::Mat>();
  }

  cv::Mat& cutout_region = mOutput->getData();
  cutout_region = input(cv::Rect(left, top, width, height)).clone();
  this->mWTAActivation->getData().at<float>(top + width/2, left + height/2) = 1.0;
}


void cedar::proc::steps::AttentionSlice::updateOutput()
{
  QReadLocker l(&this->mOutput->getLock());
  cv::Mat old_out = this->mOutput->getData().clone();
  l.unlock();

  QReadLocker wta_l(&this->mWTAActivation->getLock());
  cv::Mat old_wta = this->mWTAActivation->getData().clone();
  wta_l.unlock();

  this->callComputeWithoutTriggering();

  l.relock();
  const cv::Mat& out = this->mOutput->getData();
  bool changed = old_out.type() != out.type() || out.size != old_out.size;
  l.unlock();

  wta_l.relock();
  const cv::Mat& wta = this->mWTAActivation->getData();
  bool wta_changed = old_wta.type() != wta.type() || wta.size != old_wta.size;
  wta_l.unlock();

  if (changed)
  {
    this->emitOutputPropertiesChangedSignal("cutout_region");
  }

  if (wta_changed)
  {
    this->emitOutputPropertiesChangedSignal("WTA activation");
  }
}

void cedar::proc::steps::AttentionSlice::rangeChanged()
{
  this->updateOutput();
}

void cedar::proc::steps::AttentionSlice::inputConnectionChanged(const std::string&)
{
  this->updateOutput();
}

cv::Point cedar::proc::steps::AttentionSlice::getCorrectedAttention()
{
	cv::Point attention;
	attention.x = std::round(this->mLastAttentionCenterX);
	attention.y = std::round(this->mLastAttentionCenterY);
	return attention;
}

