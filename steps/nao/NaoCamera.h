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

    File:        NaoCamera.h

    Maintainer:  Mathis Richter
    Email:       mathis.richter@ini.rub.de
    Date:        2012 07 31

    Description:

    Credits:

======================================================================================================================*/

#ifndef CEDAR_PROC_STEPS_NAO_CAMERA_H
#define CEDAR_PROC_STEPS_NAO_CAMERA_H

// LOCAL INCLUDES
#include "steps/nao/NaoCamera.fwd.h"

// PROJECT INCLUDES

// SYSTEM INCLUDES
#include <cedar/processing/Step.h>
#include <cedar/auxiliaries/MatData.fwd.h>
#include <cedar/auxiliaries/IntParameter.fwd.h>
#include <cedar/auxiliaries/UIntParameter.fwd.h>
#include <cedar/auxiliaries/BoolParameter.fwd.h>
#include <cedar/auxiliaries/EnumType.h>
#include <cedar/auxiliaries/EnumParameter.h>
#include <alproxies/alvideodeviceproxy.h>
#include <alvision/alvisiondefinitions.h>


/*!@brief Processing step that grabs an image from NAO's camera.
 */
class cedar::proc::steps::NaoCamera : public cedar::proc::Step
{
  //--------------------------------------------------------------------------------------------------------------------
  // macros
  //--------------------------------------------------------------------------------------------------------------------
  Q_OBJECT

  //--------------------------------------------------------------------------------------------------------------------
  // nested types
  //--------------------------------------------------------------------------------------------------------------------
  typedef boost::shared_ptr<AL::ALVideoDeviceProxy> ALVideoDeviceProxyPtr;

public:
  //!@brief Enum class for camera resolution.
  class Resolution
  {
    public:
      //! the id of an enum entry
      typedef cedar::aux::EnumId Id;

      //! constructs the enum for all ids
      static void construct()
      {
        mType.type()->def(cedar::aux::Enum(QQVGA, "QQVGA (160*120)"));
        mType.type()->def(cedar::aux::Enum(QVGA, "QVGA (320*240)"));
        mType.type()->def(cedar::aux::Enum(VGA, "VGA (640*480)"));
        mType.type()->def(cedar::aux::Enum(VGA4, "4VGA (1280*940)"));
      }

      //! @returns A const reference to the base enum object.
      static const cedar::aux::EnumBase& type()
      {
        return *(mType.type());
      }

      //! @returns A pointer to the base enum object.
      static const cedar::proc::DataRole::TypePtr& typePtr()
      {
        return mType.type();
      }

    public:
      static const Id QQVGA	= 0;	// 160*120px
      static const Id QVGA = 1; // 320*240px
      static const Id VGA	= 2; // 640*480px
      static const Id VGA4 = 3; // 1280*960px

    private:
      static cedar::aux::EnumType<Resolution> mType;
  };

  //!@brief Enum class for color spaces.
  class ColorSpace
  {
    public:
      //! the id of an enum entry
      typedef cedar::aux::EnumId Id;

      //! constructs the enum for all ids
      static void construct()
      {
        mType.type()->def(cedar::aux::Enum(BGR, "BGR"));
        mType.type()->def(cedar::aux::Enum(HSV, "HSV"));
      }

      //! @returns A const reference to the base enum object.
      static const cedar::aux::EnumBase& type()
      {
        return *(mType.type());
      }

      //! @returns A pointer to the base enum object.
      static const cedar::proc::DataRole::TypePtr& typePtr()
      {
        return mType.type();
      }

    public:
      //! flag for automatically determining the color space (using annotations)
      static const Id BGR = AL::kBGRColorSpace;
      //! flag for HSV color space (hue, saturation, value)
      static const Id HSV = AL::kHSYColorSpace;

    private:
      static cedar::aux::EnumType<ColorSpace> mType;
  };

  //!@brief Enum class for camera selection.
  class CameraSelect
  {
    public:
      //! the id of an enum entry
      typedef cedar::aux::EnumId Id;

      //! constructs the enum for all ids
      static void construct()
      {
        mType.type()->def(cedar::aux::Enum(Forehead, "Forehead"));
        mType.type()->def(cedar::aux::Enum(Mouth, "Mouth"));
      }

      //! @returns A const reference to the base enum object.
      static const cedar::aux::EnumBase& type()
      {
        return *(mType.type());
      }

      //! @returns A pointer to the base enum object.
      static const cedar::proc::DataRole::TypePtr& typePtr()
      {
        return mType.type();
      }

    public:
      static const Id Forehead = 0;
      static const Id Mouth = 1;

    private:
      static cedar::aux::EnumType<CameraSelect> mType;
  };

  //!@brief Enum class for the auto exposure control algorithm.
  class AutoExposureControlAlgorithm
  {
    public:
      //! the id of an enum entry
      typedef cedar::aux::EnumId Id;

      //! constructs the enum for all ids
      static void construct()
      {
        mType.type()->def(cedar::aux::Enum(AverageSceneBrightness, "average scene brightness"));
        mType.type()->def(cedar::aux::Enum(WeightedAverageSceneBrightness, "weighted average scene brightness"));
        mType.type()->def(cedar::aux::Enum(AdaptiveWeightedHighlights, "adaptive weighted for highlights"));
        mType.type()->def(cedar::aux::Enum(AdaptiveWeightedLowlights, "adaptive weighted for lowlights"));
      }

      //! @returns A const reference to the base enum object.
      static const cedar::aux::EnumBase& type()
      {
        return *(mType.type());
      }

      //! @returns A pointer to the base enum object.
      static const cedar::proc::DataRole::TypePtr& typePtr()
      {
        return mType.type();
      }

    public:
      static const Id AverageSceneBrightness = 0;
      static const Id WeightedAverageSceneBrightness = 1;
      static const Id AdaptiveWeightedHighlights = 2;
      static const Id AdaptiveWeightedLowlights = 3;

    private:
      static cedar::aux::EnumType<AutoExposureControlAlgorithm> mType;
  };




  //--------------------------------------------------------------------------------------------------------------------
  // constructors and destructor
  //--------------------------------------------------------------------------------------------------------------------
public:
  //!@brief The standard constructor.
  NaoCamera();

  //!@brief Destructor
  ~NaoCamera();

  //--------------------------------------------------------------------------------------------------------------------
  // public methods
  //--------------------------------------------------------------------------------------------------------------------
public:
  //!@brief refreshes the internal matrix containing the camera image
  void compute(const cedar::proc::Arguments& arguments);

public slots:
  void recompute();
  void updateBrightness();
  void updateContrast();
  void updateSaturation();
  void updateHue();
  void updateGain();
  void updateHorizontalFlip();
  void updateVerticalFlip();
  void updateWhiteBalance();
  void updateAutoWhiteBalance();
  void updateResolution();
  void updateFrameRate();
  void updateExposure();
  void updateCameraSelect();
  void updateAutoExposureControlAlgorithm();
  void updateSharpness();
  void updateBacklightCompensation();

  //--------------------------------------------------------------------------------------------------------------------
  // protected methods
  //--------------------------------------------------------------------------------------------------------------------
protected:

  //--------------------------------------------------------------------------------------------------------------------
  // private methods
  //--------------------------------------------------------------------------------------------------------------------
private:
  virtual void onStart();
  virtual void onStop();
  virtual void reset();
  void setConstant(bool constant);
  void annotateImage();

  //--------------------------------------------------------------------------------------------------------------------
  // parameters
  //--------------------------------------------------------------------------------------------------------------------
private:
  cedar::aux::StringParameterPtr _mNaoIpAddress;
  cedar::aux::EnumParameterPtr _mCameraSelect;
  cedar::aux::EnumParameterPtr _mResolution;
  cedar::aux::UIntParameterPtr _mFrameRate;
  cedar::aux::EnumParameterPtr _mColorSpace;

  cedar::aux::UIntParameterPtr _mBrightness;
  cedar::aux::UIntParameterPtr _mContrast;
  cedar::aux::UIntParameterPtr _mSaturation;
  cedar::aux::IntParameterPtr _mHue;
  cedar::aux::UIntParameterPtr _mGain;
  cedar::aux::IntParameterPtr _mSharpness;
  cedar::aux::UIntParameterPtr _mBacklightCompensation;

  cedar::aux::UIntParameterPtr _mExposure;
  cedar::aux::EnumParameterPtr _mAutoExposureControlAlgorithm;

  cedar::aux::UIntParameterPtr _mWhiteBalance;
  cedar::aux::BoolParameterPtr _mAutoWhiteBalance;

  cedar::aux::BoolParameterPtr _mHorizontalFlip;
  cedar::aux::BoolParameterPtr _mVerticalFlip;

  //--------------------------------------------------------------------------------------------------------------------
  // members
  //--------------------------------------------------------------------------------------------------------------------
protected:
  //!@brief the buffer containing the output
  cedar::aux::MatDataPtr mOutput;
private:
  std::string mSubscriberId;
  ALVideoDeviceProxyPtr mNaoVisionProxy;
}; // class cedar::proc::steps::NaoCamera

#endif // CEDAR_PROC_STEPS_NAO_CAMERA_H
