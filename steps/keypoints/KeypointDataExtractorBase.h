/*======================================================================================================================

    Copyright 2011, 2012, 2013, 2014 Institut fuer Neuroinformatik, Ruhr-Universitaet Bochum, Germany
 
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

    File:        KeyPointDataExtractorBase.h

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2014 12 09

    Description: Header file for the class keypoints::KeyPointDataExtractorBase.

    Credits:

======================================================================================================================*/

#ifndef CEDAR_PROC_STEPS_KEY_POINT_DATA_EXTRACTOR_BASE_H
#define CEDAR_PROC_STEPS_KEY_POINT_DATA_EXTRACTOR_BASE_H

// CEDAR CONFIGURATION
#include "cedar/configuration.h"

// PROJECT INCLUDES
#include "data_structures/KeypointData.h"

// CEDAR INCLUDES
#include <cedar/processing/Step.h>
#include <cedar/processing/InputSlotHelper.h>
#include <cedar/auxiliaries/MatData.h>

// FORWARD DECLARATIONS
#include "steps/keypoints/KeypointDataExtractorBase.fwd.h"

// SYSTEM INCLUDES


/*!@brief A base class for extracting maps from keypoint data.
 */
class cedar::proc::steps::KeypointDataExtractorBase : public cedar::proc::Step
{
  //--------------------------------------------------------------------------------------------------------------------
  // nested types
  //--------------------------------------------------------------------------------------------------------------------

  //--------------------------------------------------------------------------------------------------------------------
  // constructors and destructor
  //--------------------------------------------------------------------------------------------------------------------
public:
  //!@brief A constructor.
  KeypointDataExtractorBase
  (
    const std::string& outputName,
    const boost::function<cv::Mat(const vislab::keypoints::KPData&)>& extractor
  )
  :
  mInput(this, "keypoint data")
  {
    this->construct(outputName, extractor, extractor);
  }

  //!@brief A constructor.
  KeypointDataExtractorBase
  (
    const std::string& outputName,
    const boost::function<cv::Mat(const vislab::keypoints::KPData&)>& extractor,
    const boost::function<cv::Mat(const vislab::keypoints::KPData&)>& sizeExtractor
  )
  :
  mInput(this, "keypoint data")
  {
    this->construct(outputName, extractor, sizeExtractor);
  }

private:
  void construct
  (
    const std::string& outputName,
    const boost::function<cv::Mat(const vislab::keypoints::KPData&)>& extractor,
    const boost::function<cv::Mat(const vislab::keypoints::KPData&)>& sizeExtractor
  );

  //--------------------------------------------------------------------------------------------------------------------
  // public methods
  //--------------------------------------------------------------------------------------------------------------------
public:

  //--------------------------------------------------------------------------------------------------------------------
  // protected methods
  //--------------------------------------------------------------------------------------------------------------------
protected:
  void compute(const cedar::proc::Arguments&);

  void compute(const cedar::proc::Arguments& args, const cedar::aux::KeypointData::StoredType& data);

  void inputConnectionChanged(const std::string& slotName);

  //--------------------------------------------------------------------------------------------------------------------
  // private methods
  //--------------------------------------------------------------------------------------------------------------------
private:
  std::string mOutputName;

  //--------------------------------------------------------------------------------------------------------------------
  // members
  //--------------------------------------------------------------------------------------------------------------------
protected:
  //! The keypoint data input
  cedar::proc::InputSlotHelper<cedar::aux::KeypointData> mInput;

private:
  //! The output map
  cedar::aux::MatDataPtr mOutput;

  //! Functor that defines which map is extracted from the keypoint data.
  boost::function<cv::Mat(const vislab::keypoints::KPData&)> mExtractor;

  //--------------------------------------------------------------------------------------------------------------------
  // parameters
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

private:
  // none yet

}; // class cedar::proc::steps::KeypointDataExtractorBase

#endif // CEDAR_PROC_STEPS_KEY_POINT_DATA_EXTRACTOR_BASE_H

