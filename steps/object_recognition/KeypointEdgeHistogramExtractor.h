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

    File:        KeypointEdgeHistogramExtractor.h

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2015 10 20

    Description: Header file for the class shapeBasedRecognition::KeypointEdgeHistogramExtractor.

    Credits:

======================================================================================================================*/

#ifndef SHAPE_BASED_RECOGNITION_KEYPOINT_EDGE_HISTOGRAM_EXTRACTOR_H
#define SHAPE_BASED_RECOGNITION_KEYPOINT_EDGE_HISTOGRAM_EXTRACTOR_H

// CEDAR CONFIGURATION
#include "cedar/configuration.h"

// PLUGIN INCLUDES
#include <data_structures/KeypointData.h>
#include <data_structures/KeypointListData.h>

// CEDAR INCLUDES
#include <cedar/processing/Step.h>
#include <cedar/processing/InputSlotHelper.h>
#include <cedar/auxiliaries/BoolParameter.h>
#include <cedar/auxiliaries/DoubleParameter.h>
#include <cedar/auxiliaries/UIntParameter.h>
#include <cedar/auxiliaries/MatData.h>
#include <cedar/auxiliaries/convolution/Convolution.h>

// FORWARD DECLARATIONS
#include "steps/object_recognition/KeypointEdgeHistogramExtractor.fwd.h"

// SYSTEM INCLUDES


/*!@todo describe.
 *
 * @todo describe more.
 */
class cedar::proc::steps::KeypointEdgeHistogramExtractor : public cedar::proc::Step
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
  KeypointEdgeHistogramExtractor();

  //--------------------------------------------------------------------------------------------------------------------
  // public methods
  //--------------------------------------------------------------------------------------------------------------------
public:
  inline unsigned int getSubsamplingFactor() const
  {
    return this->_mSubsampling->getValue();
  }

  inline void setSubsamplingFactor(unsigned int factor)
  {
    this->_mSubsampling->setValue(factor);
  }

  //--------------------------------------------------------------------------------------------------------------------
  // protected methods
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

  //--------------------------------------------------------------------------------------------------------------------
  // private methods
  //--------------------------------------------------------------------------------------------------------------------
private:
  void inputConnectionChanged(const std::string& inputName);

  void compute(const cedar::proc::Arguments& arguments);

  void updateOutputSize();

private slots:
  void samplingFactorChanged();
  void recompute();

  //--------------------------------------------------------------------------------------------------------------------
  // members
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet
private:
  // inputs
  cedar::proc::InputSlotHelper<cedar::aux::KeypointData> mKeypointData;

  cedar::proc::InputSlotHelper<cedar::aux::KeypointListData> mKeypoints;

  cedar::proc::InputSlotHelper<cedar::aux::MatData> mEdgeLocations;

  cedar::proc::InputSlotHelper<cedar::aux::MatData> mEdgeOrientations;

  cedar::proc::InputSlotHelper<cedar::aux::MatData> mGradientMagnitudes;

  cedar::proc::InputSlotHelper<cedar::aux::MatData> mGradientOrientations;

  // buffers
  cedar::aux::MatDataPtr mEdgeLocationsResampled;

  cedar::aux::MatDataPtr mEdgeOrientationsResampled;

  cedar::aux::MatDataPtr mGradientOrientationsResampled;

  cedar::aux::MatDataPtr mGradientOrientationAcuities;

  cedar::aux::MatDataPtr mGradientMagnitudesResampled;

  cedar::aux::MatDataPtr mHistogramBuffer;

  cedar::aux::MatDataPtr mKeypointDistanceBuffer;

  // outputs
  cedar::aux::MatDataPtr mKeypointHistograms;

  cedar::aux::MatDataPtr mOrientationMap;

  //--------------------------------------------------------------------------------------------------------------------
  // parameters
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

private:
  cedar::aux::DoubleParameterPtr _mEdgeStrengthThreshold;

  cedar::aux::DoubleParameterPtr _mGradientStrengthThreshold;

  cedar::aux::DoubleParameterPtr _mKeypointRadiusMultiplier;

  cedar::aux::UIntParameterPtr _mSubsampling;

  cedar::aux::BoolParameterPtr _mUseGradient;

  cedar::aux::BoolParameterPtr _mNormalize;

  cedar::aux::BoolParameterPtr _mRemoveKeypointsFromEdges;

  cedar::aux::BoolParameterPtr _mApplyBlurring;

  // convolution object used for the final convolution
  cedar::aux::conv::ConvolutionPtr _mConvolution;

}; // class cedar::proc::steps::KeypointEdgeHistogramExtractor

#endif // SHAPE_BASED_RECOGNITION_KEYPOINT_EDGE_HISTOGRAM_EXTRACTOR_H

