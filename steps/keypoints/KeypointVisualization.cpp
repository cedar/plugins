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

    File:        Visualization.cpp

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2015 02 20

    Description: Source file for the class keypoints::Visualization.

    Credits:

======================================================================================================================*/

// CEDAR CONFIGURATION
#include "cedar/configuration.h"

// CLASS HEADER
#include "steps/keypoints/KeypointVisualization.h"

// CEDAR INCLUDES
#include "cedar/processing/typecheck/Matrix.h"
#include "cedar/processing/typecheck/DerivedFrom.h"

// SYSTEM INCLUDES

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::proc::steps::KeypointVisualization::KeypointVisualization()
:
// inputs
mKeypoints(this, "keypoint list"),
mImage(this, "image"),
// outputs
mOutput(new cedar::aux::MatData(cv::Mat()))
{
  // declare all data
  this->declareOutput("output", mOutput);
  
  cedar::proc::typecheck::DerivedFrom<cedar::aux::KeypointListData> keypoint_check;
  mKeypoints.getSlot()->setCheck(keypoint_check);

  cedar::proc::typecheck::Matrix input_check;
  input_check.addAcceptedDimensionality(2);
  this->mImage.getSlot()->setCheck(input_check);
  this->mImage.addOutputRelation(cedar::proc::CopyMatrixPropertiesPtr(new cedar::proc::CopyMatrixProperties(this->getOutputSlot("output"))));
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

void cedar::proc::steps::KeypointVisualization::compute(const cedar::proc::Arguments&)
{
  cv::Mat& output = this->mOutput->getData();
  this->mImage.getData().copyTo(output);
  
  cv::Scalar color(255, 0, 0);

  cv::drawKeypoints(output, this->mKeypoints.getData(), output, color, cv::DrawMatchesFlags::DRAW_OVER_OUTIMG | cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
}
