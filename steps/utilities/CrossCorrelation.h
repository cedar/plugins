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

    File:        CrossCorrelation.h

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2012 01 26

    Description:

    Credits:

======================================================================================================================*/

#ifndef CEDAR_PROC_STEPS_CROSS_CORRELATION_H
#define CEDAR_PROC_STEPS_CROSS_CORRELATION_H

// LOCAL INCLUDES
#include "steps/utilities/CrossCorrelation.fwd.h"

// PROJECT INCLUDES

// SYSTEM INCLUDES
#include <cedar/processing/Step.h>
#include <cedar/auxiliaries/convolution/BorderType.h>
#include <cedar/auxiliaries/BoolParameter.h>
#include <cedar/auxiliaries/BoolVectorParameter.h>
#include <cedar/auxiliaries/DoubleParameter.h>
#include <cedar/auxiliaries/EnumParameter.h>

// FORWARD DECLARATIONS
#include <cedar/auxiliaries/convolution/Convolution.h> // should be a forward declaration, but the header doesn't exist yet


/*!@brief A step that calculates cross correlation between two patterns.
 */
class cedar::proc::steps::CrossCorrelation : public cedar::proc::Step
{
  //--------------------------------------------------------------------------------------------------------------------
  // macros
  //--------------------------------------------------------------------------------------------------------------------
  Q_OBJECT

  //--------------------------------------------------------------------------------------------------------------------
  // constructors and destructor
  //--------------------------------------------------------------------------------------------------------------------
public:
  //!@brief The standard constructor.
  CrossCorrelation();

  //!@brief Destructor
  ~CrossCorrelation();

  //--------------------------------------------------------------------------------------------------------------------
  // public methods
  //--------------------------------------------------------------------------------------------------------------------
public:
  void inputConnectionChanged(const std::string& inputName);

  cedar::proc::DataSlot::VALIDITY determineInputValidity
  (
    cedar::proc::ConstDataSlotPtr slot,
    cedar::aux::ConstDataPtr data
  )
  const;

public slots:
  void recompute();
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

  void compute1D2D();

  void compute3D();

  void compute4D();

  inline bool padInput() const
  {
    return this->_mPadInput->getValue();
  }

  inline bool correlateAlong(unsigned int dimension) const
  {
    return this->_mCorrelatedDimensions->at(dimension);
  }

  inline int getCvBorderType() const
  {
    return cedar::aux::conv::BorderType::toCvConstant(this->_mBorderType->getValue());
  }

  void createConvolutionObject();

  cv::Mat normalizeMatrix(const cv::Mat& in, double& norm, cv::Mat mask = cv::Mat());

  void computeMasks(const cv::Mat& input, const cv::Mat& pattern, cv::Mat& inputMask, cv::Mat& outputMask) const;

private slots:
  void alternateKernelCenterChanged();

  void applySpatialFilteringChanged();

//---------------------------------------------------------------------------------------------------------------
  // members
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

private:
  // inputs
  cedar::aux::ConstMatDataPtr mInput;
  cedar::aux::ConstMatDataPtr mPattern;

  // buffers
  cedar::aux::MatDataPtr mNormalizedInput;
  cedar::aux::MatDataPtr mNormalizedPattern;
  cedar::aux::MatDataPtr mFullResult;

  // outputs
  cedar::aux::MatDataPtr mCorrelation;

  // convolution used for 3D cross-correlation
  cedar::aux::conv::ConvolutionPtr mConvolution;

  // used by some variants of the correlation to store a temporary result (part of which is then returned).
  cv::Mat mTempResultMatrix;

  //--------------------------------------------------------------------------------------------------------------------
  // parameters
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

private:
  cedar::aux::BoolParameterPtr _mPadInput;
  cedar::aux::BoolParameterPtr _mUnPadOutput;
  cedar::aux::BoolParameterPtr _mNormalize;
  cedar::aux::BoolVectorParameterPtr _mCorrelatedDimensions;
  cedar::aux::EnumParameterPtr _mBorderType;
  cedar::aux::BoolParameterPtr _mAlternateKernelCenter;

  // (4d corr 3d only) assumes that columns in space that are entirely zero are not part of the area to be matched
  // note that this method assumes that the spatial dimensions are the last ones in the matrix, i.e., that mat[0] and
  // mat[1] are some feature dimensions.
  cedar::aux::BoolParameterPtr _mApplySpatialFiltering;

  // spatial columns are considered empty if their mean value lies below this threshold
  cedar::aux::DoubleParameterPtr _mSpatialFilteringThreshold;

}; // class cedar::proc::steps::CrossCorrelation

#endif // CEDAR_PROC_STEPS_CROSS_CORRELATION_H
