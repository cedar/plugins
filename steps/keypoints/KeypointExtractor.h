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

    File:        KeypointExtractor.h

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2014 12 08

    Description: Header file for the class keypoints::KeypointExtractor.

    Credits:

======================================================================================================================*/

#ifndef CEDAR_PROC_STEPS_KEYPOINT_EXTRACTOR_H
#define CEDAR_PROC_STEPS_KEYPOINT_EXTRACTOR_H

// CEDAR CONFIGURATION
#include "cedar/configuration.h"

// PROJECT INCLUDES
#include "data_structures/KeypointData.h"
#include "data_structures/KeypointListData.h"

// CEDAR INCLUDES
#include <cedar/processing/Step.h>
#include <cedar/processing/InputSlotHelper.h>
#include <cedar/auxiliaries/MatData.h>
#include <cedar/auxiliaries/UIntParameter.h>
#include <cedar/auxiliaries/BoolParameter.h>
#include <cedar/auxiliaries/DoubleParameter.h>
#include <cedar/auxiliaries/EnumParameter.h>
#include <cedar/auxiliaries/EnumType.h>

// FORWARD DECLARATIONS
#include "steps/keypoints/KeypointExtractor.fwd.h"

// SYSTEM INCLUDES


/*!@brief A step that encapsulates the keypoint extraction function of the vision lab toolbox.
 */
class cedar::proc::steps::KeypointExtractor : public cedar::proc::Step
{
  Q_OBJECT

  //--------------------------------------------------------------------------------------------------------------------
  // nested types
  //--------------------------------------------------------------------------------------------------------------------
public:
  class ScaleType
  {
  public:
    //! Type of the enum.
    typedef cedar::aux::EnumId Id;

    //! Pointer to the enumeration type.
    typedef boost::shared_ptr<cedar::aux::EnumBase> TypePtr;

    //! Constructs the enumeration values.
    static void construct();

    //! Returns the enum base class.
    static const cedar::aux::EnumBase& type();

    //! Returns a pointer to the enum base class.
    static const cedar::proc::steps::KeypointExtractor::ScaleType::TypePtr& typePtr();

    //! Linear scale.
    static const Id Linear = 0;

    //! Logarithmic scale.
    static const Id Logarithmic = 1;

  private:
    static cedar::aux::EnumType<cedar::proc::steps::KeypointExtractor::ScaleType> mType;
  };

  //--------------------------------------------------------------------------------------------------------------------
  // constructors and destructor
  //--------------------------------------------------------------------------------------------------------------------
public:
  //!@brief The standard constructor.
  KeypointExtractor();

  //--------------------------------------------------------------------------------------------------------------------
  // public methods
  //--------------------------------------------------------------------------------------------------------------------
public:
  size_t getScaleIndex(double scale) const;

  //--------------------------------------------------------------------------------------------------------------------
  // protected methods
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

  //--------------------------------------------------------------------------------------------------------------------
  // private methods
  //--------------------------------------------------------------------------------------------------------------------
private:
  void compute(const cedar::proc::Arguments& arguments);

  void inputConnectionChanged(const std::string& slotName);

private slots:
  void recompute();

  void updateAndRecompute();

  void updateLambdas();

  //--------------------------------------------------------------------------------------------------------------------
  // members
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet
private:
  //!@brief The input image on which keypoints are extracted.
  cedar::proc::InputSlotHelper<cedar::aux::MatData> mInput;

  //!@brief The extracted keypoints.
  cedar::aux::KeypointDataPtr mKeypointData;
  
  cedar::aux::KeypointListDataPtr mKeypoints;

  //! Lambdas used by the keypoint extraction.
  std::vector<double> mLambdas;

  //--------------------------------------------------------------------------------------------------------------------
  // parameters
  //--------------------------------------------------------------------------------------------------------------------
private:
  cedar::aux::UIntParameterPtr _mOrientations;

  cedar::aux::BoolParameterPtr _mScale;

  cedar::aux::DoubleParameterPtr _mLambdasStart;

  cedar::aux::DoubleParameterPtr _mLambdasEnd;

  cedar::aux::UIntParameterPtr _mNumLambdas;

  cedar::aux::EnumParameterPtr _mScaleType;

}; // class cedar::proc::steps::KeypointExtractor

#endif // CEDAR_PROC_STEPS_KEYPOINT_EXTRACTOR_H

