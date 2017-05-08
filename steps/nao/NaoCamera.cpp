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

    File:        NaoCamera.cpp

    Maintainer:  Mathis Richter
    Email:       mathis.richter@ini.rub.de
    Date:        2012 08 01

    Description:

    Credits:

======================================================================================================================*/

// LOCAL INCLUDES
#include "steps/nao/NaoCamera.h"
#include "cedar/auxiliaries/annotation/ColorSpace.h"

// SYSTEM INCLUDES
#include <cedar/auxiliaries/assert.h>
#include <cedar/auxiliaries/exceptions.h>
#include <cedar/auxiliaries/DataTemplate.h>
#include <cedar/auxiliaries/BoolParameter.h>
#include <cedar/auxiliaries/IntParameter.h>
#include <cedar/auxiliaries/UIntParameter.h>
#include <cedar/auxiliaries/StringParameter.h>
#include <cedar/auxiliaries/MatData.h>
#include <cedar/auxiliaries/EnumType.h>
#include <cedar/auxiliaries/EnumParameter.h>
#include <cedar/auxiliaries/Log.h>
#include <alerror/alnetworkerror.h>
#include <alproxies/alvideodeviceproxy.h>
#include <alvision/alvisiondefinitions.h>
#include <alvision/alimage.h>
#include <alvalue/alvalue.h>


//----------------------------------------------------------------------------------------------------------------------
// static members
//----------------------------------------------------------------------------------------------------------------------

cedar::aux::EnumType<cedar::proc::steps::NaoCamera::ColorSpace> cedar::proc::steps::NaoCamera::ColorSpace::mType("ColorSpace::");
cedar::aux::EnumType<cedar::proc::steps::NaoCamera::CameraSelect> cedar::proc::steps::NaoCamera::CameraSelect::mType("CameraSelect::");
cedar::aux::EnumType<cedar::proc::steps::NaoCamera::Resolution> cedar::proc::steps::NaoCamera::Resolution::mType("Resolution::");
cedar::aux::EnumType<cedar::proc::steps::NaoCamera::AutoExposureControlAlgorithm> cedar::proc::steps::NaoCamera::AutoExposureControlAlgorithm::mType("AutoExposureControlAlgorithm::");

#ifndef CEDAR_COMPILER_MSVC
const cedar::proc::steps::NaoCamera::ColorSpace::Id cedar::proc::steps::NaoCamera::ColorSpace::BGR;
const cedar::proc::steps::NaoCamera::ColorSpace::Id cedar::proc::steps::NaoCamera::ColorSpace::HSV;

const cedar::proc::steps::NaoCamera::CameraSelect::Id cedar::proc::steps::NaoCamera::CameraSelect::Forehead;
const cedar::proc::steps::NaoCamera::CameraSelect::Id cedar::proc::steps::NaoCamera::CameraSelect::Mouth;

const cedar::proc::steps::NaoCamera::AutoExposureControlAlgorithm::Id cedar::proc::steps::NaoCamera::AutoExposureControlAlgorithm::AverageSceneBrightness;
const cedar::proc::steps::NaoCamera::AutoExposureControlAlgorithm::Id cedar::proc::steps::NaoCamera::AutoExposureControlAlgorithm::WeightedAverageSceneBrightness;
const cedar::proc::steps::NaoCamera::AutoExposureControlAlgorithm::Id cedar::proc::steps::NaoCamera::AutoExposureControlAlgorithm::AdaptiveWeightedHighlights;
const cedar::proc::steps::NaoCamera::AutoExposureControlAlgorithm::Id cedar::proc::steps::NaoCamera::AutoExposureControlAlgorithm::AdaptiveWeightedLowlights;


const cedar::proc::steps::NaoCamera::Resolution::Id cedar::proc::steps::NaoCamera::Resolution::QQVGA;
const cedar::proc::steps::NaoCamera::Resolution::Id cedar::proc::steps::NaoCamera::Resolution::QVGA;
const cedar::proc::steps::NaoCamera::Resolution::Id cedar::proc::steps::NaoCamera::Resolution::VGA;
const cedar::proc::steps::NaoCamera::Resolution::Id cedar::proc::steps::NaoCamera::Resolution::VGA4;
#endif // CEDAR_COMPILER_MSVC

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::proc::steps::NaoCamera::NaoCamera()
:
cedar::proc::Step(true),
_mNaoIpAddress(new cedar::aux::StringParameter(this, "nao ip address", "nao.local")),
_mCameraSelect(new cedar::aux::EnumParameter(this, "camera", CameraSelect::typePtr(), CameraSelect::Forehead)),
_mResolution(new cedar::aux::EnumParameter(this, "resolution", Resolution::typePtr(), Resolution::QQVGA)),
_mFrameRate(new cedar::aux::UIntParameter(this, "frame rate (fps)", 30, 1, 30)),
_mColorSpace(new cedar::aux::EnumParameter(this, "color space", ColorSpace::typePtr(), ColorSpace::BGR)),
_mBrightness(new cedar::aux::UIntParameter(this, "brightness", 55, 0, 255)),
_mContrast(new cedar::aux::UIntParameter(this, "contrast", 32, 16, 32)),
_mSaturation(new cedar::aux::UIntParameter(this, "saturation", 128, 0, 255)),
_mHue(new cedar::aux::IntParameter(this, "hue", 0, -180, 180)),
_mGain(new cedar::aux::UIntParameter(this, "gain", 32, 32, 255)),
_mSharpness(new cedar::aux::IntParameter(this, "sharpness", 0, -1, 7)),
_mBacklightCompensation(new cedar::aux::UIntParameter(this, "backlight compensation", 1, 0, 4)),
_mExposure(new cedar::aux::UIntParameter(this, "exposure", 0, 0, 2500)),
_mAutoExposureControlAlgorithm(new cedar::aux::EnumParameter(this, "auto exposure control algorithm", AutoExposureControlAlgorithm::typePtr(), AutoExposureControlAlgorithm::WeightedAverageSceneBrightness)),
_mWhiteBalance(new cedar::aux::UIntParameter(this, "white balance", 2700, 2700, 6500)),
_mAutoWhiteBalance(new cedar::aux::BoolParameter(this, "auto white balance", true)),
_mHorizontalFlip(new cedar::aux::BoolParameter(this, "horizontal flip", false)),
_mVerticalFlip(new cedar::aux::BoolParameter(this, "vertical flip", false)),
// --  
mOutput(new cedar::aux::MatData(cv::Mat(cv::Size(160, 120), CV_8UC3))),
mSubscriberId("subscriber id"),
mNaoVisionProxy()
{
  // output
  this->declareOutput("output", mOutput);
  this->annotateImage();

  QObject::connect(this->_mBrightness.get(), SIGNAL(valueChanged()), this, SLOT(updateBrightness()));
  QObject::connect(this->_mContrast.get(), SIGNAL(valueChanged()), this, SLOT(updateContrast()));
  QObject::connect(this->_mSaturation.get(), SIGNAL(valueChanged()), this, SLOT(updateSaturation()));
  QObject::connect(this->_mHue.get(), SIGNAL(valueChanged()), this, SLOT(updateHue()));
  QObject::connect(this->_mGain.get(), SIGNAL(valueChanged()), this, SLOT(updateGain()));
  QObject::connect(this->_mHorizontalFlip.get(), SIGNAL(valueChanged()), this, SLOT(updateHorizontalFlip()));
  QObject::connect(this->_mVerticalFlip.get(), SIGNAL(valueChanged()), this, SLOT(updateVerticalFlip()));
  QObject::connect(this->_mAutoWhiteBalance.get(), SIGNAL(valueChanged()), this, SLOT(updateAutoWhiteBalance()));
  QObject::connect(this->_mResolution.get(), SIGNAL(valueChanged()), this, SLOT(updateResolution()));
  QObject::connect(this->_mFrameRate.get(), SIGNAL(valueChanged()), this, SLOT(updateFrameRate()));
  QObject::connect(this->_mExposure.get(), SIGNAL(valueChanged()), this, SLOT(updateExposure()));
  QObject::connect(this->_mCameraSelect.get(), SIGNAL(valueChanged()), this, SLOT(updateCameraSelect()));
  QObject::connect(this->_mAutoExposureControlAlgorithm.get(), SIGNAL(valueChanged()), this, SLOT(updateAutoExposureControlAlgorithm()));
  QObject::connect(this->_mSharpness.get(), SIGNAL(valueChanged()), this, SLOT(updateSharpness()));
  QObject::connect(this->_mBacklightCompensation.get(), SIGNAL(valueChanged()), this, SLOT(updateBacklightCompensation()));
}

cedar::proc::steps::NaoCamera::~NaoCamera()
{
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

void cedar::proc::steps::NaoCamera::annotateImage()
{
  cedar::aux::annotation::ColorSpacePtr color_space;
  switch (_mColorSpace->getValue())
  {
    case AL::kBGRColorSpace:
      color_space = cedar::aux::annotation::ColorSpace::bgr();
      break;

    case AL::kHSYColorSpace:
      color_space = cedar::aux::annotation::ColorSpacePtr
                    (
		      new cedar::aux::annotation::ColorSpace
		          (
			    cedar::aux::annotation::ColorSpace::Hue,
			    cedar::aux::annotation::ColorSpace::Saturation,
			    cedar::aux::annotation::ColorSpace::Value
			  )
		      );
      break;

    default:
      // this should not happen.
      CEDAR_ASSERT(false);
  } // switch

  this->mOutput->setAnnotation(color_space);
}

void cedar::proc::steps::NaoCamera::onStart()
{
  this->annotateImage();

  if (mNaoVisionProxy == NULL)
  {
    setConstant(true);
    try
    {
      mNaoVisionProxy = cedar::proc::steps::NaoCamera::ALVideoDeviceProxyPtr(new AL::ALVideoDeviceProxy(_mNaoIpAddress->getValue()));
      mSubscriberId = mNaoVisionProxy->subscribeCamera
                      (
                        mSubscriberId,
                        _mCameraSelect->getValue(),
                        _mResolution->getValue(),
                        _mColorSpace->getValue(),
                        _mFrameRate->getValue()
                      );
    }
    catch (const AL::ALNetworkError& ex)
    {
      cedar::aux::LogSingleton::getInstance()->error // you can also use "warning", if this is not severe
      (
        "Unable to connect to the Nao robot!", // this is the message
        "cedar::proc::steps::NaoCamera::onStart()"
      );
    }
  }
  else
  {
    cedar::aux::LogSingleton::getInstance()->warning // you can also use "warning", if this is not severe
    (
      "Already subscribed to the Nao camera!", // this is the message
      "cedar::proc::steps::NaoCamera::onStart()"
    );
  }
}

void cedar::proc::steps::NaoCamera::onStop()
{
  if (mNaoVisionProxy != NULL)
  {
    mNaoVisionProxy->unsubscribe(mSubscriberId);
    mNaoVisionProxy.reset();

    setConstant(false);
  }
}

void cedar::proc::steps::NaoCamera::setConstant(bool constant)
{
  _mColorSpace->setConstant(constant);
  _mCameraSelect->setConstant(constant);
  _mResolution->setConstant(constant);
}

void cedar::proc::steps::NaoCamera::reset() {
  onStop();
  onStart();
}

void cedar::proc::steps::NaoCamera::recompute()
{
  // this triggers all connected steps.
  this->onTrigger();
}

void cedar::proc::steps::NaoCamera::updateBrightness()
{ 
  if (mNaoVisionProxy != NULL)
  {
    mNaoVisionProxy->setParam(AL::kCameraBrightnessID, _mBrightness->getValue());
  }
}

void cedar::proc::steps::NaoCamera::updateContrast()
{ 
  if (mNaoVisionProxy != NULL)
  {
    mNaoVisionProxy->setParam(AL::kCameraContrastID, _mContrast->getValue());
  }
}

void cedar::proc::steps::NaoCamera::updateSaturation()
{ 
  if (mNaoVisionProxy != NULL)
  {
    mNaoVisionProxy->setParam(AL::kCameraSaturationID, _mSaturation->getValue());
  }
}

void cedar::proc::steps::NaoCamera::updateHue()
{ 
  if (mNaoVisionProxy != NULL)
  {
    mNaoVisionProxy->setParam(AL::kCameraHueID, _mHue->getValue());
  }
}

void cedar::proc::steps::NaoCamera::updateGain()
{ 
  if (mNaoVisionProxy != NULL)
  {
    mNaoVisionProxy->setParam(AL::kCameraGainID, _mGain->getValue());
  }
}

void cedar::proc::steps::NaoCamera::updateHorizontalFlip()
{ 
  if (mNaoVisionProxy != NULL)
  {
    mNaoVisionProxy->setParam(AL::kCameraHFlipID, _mHorizontalFlip->getValue());
  }
}

void cedar::proc::steps::NaoCamera::updateVerticalFlip()
{ 
  if (mNaoVisionProxy != NULL)
  {
    mNaoVisionProxy->setParam(AL::kCameraVFlipID, _mVerticalFlip->getValue());
  }
}

void cedar::proc::steps::NaoCamera::updateWhiteBalance()
{ 
  if (mNaoVisionProxy != NULL)
  {
    mNaoVisionProxy->setParam(AL::kCameraWhiteBalanceID, _mWhiteBalance->getValue());
  }
}

void cedar::proc::steps::NaoCamera::updateAutoWhiteBalance()
{ 
  if (mNaoVisionProxy != NULL)
  {
    mNaoVisionProxy->setParam(AL::kCameraAutoWhiteBalanceID, _mAutoWhiteBalance->getValue());
  }
}

// todo
void cedar::proc::steps::NaoCamera::updateResolution()
{ 
  if (mNaoVisionProxy != NULL)
  {
    mNaoVisionProxy->setParam(AL::kCameraResolutionID, _mResolution->getValue());
  }
}

void cedar::proc::steps::NaoCamera::updateFrameRate()
{ 
  if (mNaoVisionProxy != NULL)
  {
    mNaoVisionProxy->setParam(AL::kCameraFrameRateID, _mFrameRate->getValue());
  }
}

void cedar::proc::steps::NaoCamera::updateExposure()
{ 
  if (mNaoVisionProxy != NULL)
  {
    mNaoVisionProxy->setParam(AL::kCameraExposureID, _mExposure->getValue());
  }
}

// todo
void cedar::proc::steps::NaoCamera::updateCameraSelect()
{ 
  if (mNaoVisionProxy != NULL)
  {
    mNaoVisionProxy->setParam(AL::kCameraSelectID, _mCameraSelect->getValue());
  }
}

// todo
void cedar::proc::steps::NaoCamera::updateAutoExposureControlAlgorithm()
{ 
  if (mNaoVisionProxy != NULL)
  {
    mNaoVisionProxy->setParam(AL::kCameraAecAlgorithmID, _mAutoExposureControlAlgorithm->getValue());
  }
}

void cedar::proc::steps::NaoCamera::updateSharpness()
{ 
  if (mNaoVisionProxy != NULL)
  {
    mNaoVisionProxy->setParam(AL::kCameraSharpnessID, _mSharpness->getValue());
  }
}

void cedar::proc::steps::NaoCamera::updateBacklightCompensation()
{ 
  if (mNaoVisionProxy != NULL)
  {
    mNaoVisionProxy->setParam(AL::kCameraBacklightCompensationID, _mBacklightCompensation->getValue());
  }
}


void cedar::proc::steps::NaoCamera::compute(const cedar::proc::Arguments&)
{
  if (mNaoVisionProxy != NULL)
  {
    AL::ALValue results = mNaoVisionProxy->getImageRemote(mSubscriberId);
    const int &width = static_cast<const int>(results[0]);
    const int &height = static_cast<const int>(results[1]);
  //  std::cout << width << std::endl;
  //  std::cout << height << std::endl;

    cv::Mat image = cv::Mat(cv::Size(width, height), CV_8UC3);
    const unsigned char* image_data = static_cast<const unsigned char*>(results[6].GetBinary());

    if (image_data == NULL)
    {
    	cedar::aux::LogSingleton::getInstance()->warning
			(
				"Could not retrieve image from Nao camera!",
				"cedar::proc::steps::NaoCamera::onStart()"
			);
    }
    else
    {
    	image.data = (uchar*) image_data;
    }

    mOutput->setData(image.clone());
  }
}
