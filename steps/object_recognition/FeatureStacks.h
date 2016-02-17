/*======================================================================================================================

    Copyright 2011, 2012, 2013 Institut fuer Neuroinformatik, Ruhr-Universitaet Bochum, Germany
 
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

    File:        FeatureStacks.h

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2014 01 28

    Description: Header file for the class keypoints::FeatureStacks.

    Credits:

======================================================================================================================*/

#ifndef CEDAR_PROC_STEPS_FEATURE_STACKS_H
#define CEDAR_PROC_STEPS_FEATURE_STACKS_H

// CEDAR CONFIGURATION
#include "cedar/configuration.h"

// CEDAR INCLUDES
#include "cedar/processing/Step.h"
#include "cedar/auxiliaries/UIntParameter.h"
#include "cedar/auxiliaries/DoubleParameter.h"
#include "cedar/auxiliaries/FileParameter.h"

// FORWARD DECLARATIONS
#include "steps/object_recognition/FeatureStacks.fwd.h"
#include "cedar/auxiliaries/MatData.fwd.h"

// SYSTEM INCLUDES

/*!
 */
class cedar::proc::steps::FeatureStacks : public cedar::proc::Step
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
  FeatureStacks();

  //--------------------------------------------------------------------------------------------------------------------
  // public methods
  //--------------------------------------------------------------------------------------------------------------------
public:
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

  void inputConnectionChanged(const std::string& inputName);

  void allocateOutput();

  std::vector<cv::Mat>
    histograms
    (
      cv::Mat img,
      const std::vector<cv::KeyPoint>& points,
      cv::Mat centers,
      cv::Mat labels,
      int nrColors
    );

  std::vector<cv::Mat>
    siftprototypefeatures
    (
      cv::Mat img,
      std::vector<cv::KeyPoint> points,
      cv::Mat centers,
      cv::Mat labels,
      int nrprotos
    );

  cv::Mat floydSteinberg(cv::Mat& imgOrig, cv::Mat& centers, cv::Mat& labels, cv::Mat& labelimg);

  float vec3bDist(cv::Vec3b a, cv::Vec3b b);

  cv::Vec3b findClosestPaletteColor(cv::Vec3b color, cv::Mat palette);

  int findClosestPaletteIndex(cv::Vec3b color, cv::Mat palette);

  int getNumberOfColors() const;

private slots:
  void lambdasChanged();

  void paletteChanged();

  void subsamplingChanged();

  void recompute();

  //--------------------------------------------------------------------------------------------------------------------
  // members
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet
private:
  // inputs
  cedar::aux::ConstMatDataPtr mInput;
  cedar::aux::ConstMatDataPtr mMask;

  // buffers
  cedar::aux::MatDataPtr mColorLabels;
  cedar::aux::MatDataPtr mColorCenters;

  // outputs
  cedar::aux::MatDataPtr mOutput;

  // other members
  std::vector<double> mLambdas;

  //--------------------------------------------------------------------------------------------------------------------
  // parameters
  //--------------------------------------------------------------------------------------------------------------------
private:
  cedar::aux::UIntParameterPtr _mNumberOfScales;

  //!@todo LambdasStart/End should be a limits parameter, but we don't yet have a widget for that
  cedar::aux::DoubleParameterPtr _mLambdasStart;

  cedar::aux::DoubleParameterPtr _mLambdasEnd;

  cedar::aux::DoubleParameterPtr _mSubsamplingFactor;

  cedar::aux::FileParameterPtr _mPaletteFile;

  cedar::aux::BoolParameterPtr _mDither;

  cedar::aux::BoolParameterPtr _mUseSiftFeatures;

  cedar::aux::UIntParameterPtr _mLuminanceThreshold;

}; // class cedar::proc::steps::FeatureStacks

#endif // CEDAR_PROC_STEPS_FEATURE_STACKS_H

