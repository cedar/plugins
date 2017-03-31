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

    File:        LabelWMAnnotator.cpp

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2015 06 29

    Description: Source file for the class cedar::proc::steps::LabelWMAnnotator.

    Credits:

======================================================================================================================*/

// CEDAR CONFIGURATION
#include "cedar/configuration.h"

// CLASS HEADER
#include "steps/utilities/LabelWMAnnotator.h"

// CEDAR INCLUDES
#include "cedar/processing/typecheck/Matrix.h"
#include "cedar/processing/typecheck/SameSize.h"
#include "cedar/processing/typecheck/And.h"
#include "cedar/auxiliaries/math/tools.h"

// SYSTEM INCLUDES

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::proc::steps::LabelWMAnnotator::LabelWMAnnotator()
:
// inputs
mImage(this, "image"),
mWMToSpace(this, "WM projected to space"),
mRateCodedLabels(this, "rate coded label WM"),
// outputs
mOutput(new cedar::aux::MatData(cv::Mat())),
_mNumberOfStrings(new cedar::aux::UIntParameter(this, "number of strings", 0)),
_mStrings(new cedar::aux::StringVectorParameter(this, "strings", (std::vector<std::string>()))),
_mThreshold(new cedar::aux::DoubleParameter(this, "threshold", 0.5)),
_mFontScale(new cedar::aux::DoubleParameter(this, "font scale", 1.0)),
_mFontThickness(new cedar::aux::IntParameter(this, "font thickness", 2)),
_mCrossRadius(new cedar::aux::DoubleParameter(this, "cross radius", 5.0))
{
  // declare all data
  this->declareOutput("output", mOutput);

  cedar::proc::typecheck::Matrix image_check;
  image_check.addAcceptedDimensionality(2);
  image_check.addAcceptedType(CV_8UC3);
  mImage.getSlot()->setCheck(image_check);

  cedar::proc::typecheck::SameSize wm_size_check;
  wm_size_check.addSlot(mWMToSpace.getSlot());
  wm_size_check.addSlot(mRateCodedLabels.getSlot());

  cedar::proc::typecheck::Matrix wm_mat_check;
  wm_mat_check.addAcceptedDimensionality(2);
  wm_mat_check.addAcceptedType(CV_32F);

  cedar::proc::typecheck::And wm_check;
  wm_check.addCheck(wm_mat_check);
  wm_check.addCheck(wm_size_check);

  mWMToSpace.getSlot()->setCheck(wm_check);
  mRateCodedLabels.getSlot()->setCheck(wm_check);

  QObject::connect(this->_mNumberOfStrings.get(), SIGNAL(valueChanged()), this, SLOT(numberOfStringsChanged()));
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

void cedar::proc::steps::LabelWMAnnotator::compute(const cedar::proc::Arguments&)
{
  this->mOutput->setData(this->mImage.getData().clone());
  auto& output = this->mOutput->getData();
  const auto& wm_to_space = this->mWMToSpace.getData();
  const auto& rate_coded_label_wm = this->mRateCodedLabels.getData();

  std::vector<cv::Point> peak_centers;
  cedar::aux::math::findPeaks(wm_to_space, peak_centers, this->_mThreshold->getValue());

  double cross_radius = this->_mCrossRadius->getValue();
  double font_scale = this->_mFontScale->getValue();
  cv::Scalar color(0.0, 0.0, 255.0);
  int cross_thickness = 2;
  int font_thickness = this->_mFontThickness->getValue();
  for (const auto& center : peak_centers)
  {
    double x = static_cast<double>(center.x) / static_cast<double>(wm_to_space.cols) * static_cast<double>(output.cols);
    double y = static_cast<double>(center.y) / static_cast<double>(wm_to_space.rows) * static_cast<double>(output.rows);

    cv::Point scaled_center(x, y);

    cv::line(output, scaled_center + cv::Point(-cross_radius, 0), scaled_center + cv::Point(cross_radius, 0), color, cross_thickness);
    cv::line(output, scaled_center + cv::Point(0, -cross_radius), scaled_center + cv::Point(0, cross_radius), color, cross_thickness);


    if (center.x >= rate_coded_label_wm.cols || center.y >= rate_coded_label_wm.rows || center.x < 0 || center.y < 0)
    {
      // peak is not within the rate code matrix
      continue;
    }
    unsigned int recognized_label = static_cast<unsigned int>(rate_coded_label_wm.at<float>(center.y, center.x));

    std::string label = "?";
    if (recognized_label < this->_mStrings->size())
    {
      label = this->_mStrings->at(recognized_label);
    }
    const auto cv_aa =
#if CV_MAJOR_VERSION >= 3
    cv::LINE_AA
#else
    CV_AA
#endif
    ;
    cv::putText(output, label.c_str(), scaled_center, cv::FONT_HERSHEY_SIMPLEX, font_scale, cv::Scalar::all(255.0), font_thickness + 1, cv_aa);
    cv::putText(output, label.c_str(), scaled_center, cv::FONT_HERSHEY_SIMPLEX, font_scale, color, font_thickness, cv_aa);
  }
}

void cedar::proc::steps::LabelWMAnnotator::numberOfStringsChanged()
{
  unsigned int size = this->_mNumberOfStrings->getValue();
  this->_mStrings->resize(size);
}
