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

    File:        ReceptiveFieldHistogram.h

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2013 12 11

    Description: Header file for the class ReceptiveFieldHistogram.

    Credits:

======================================================================================================================*/

#ifndef CEDAR_PROC_STEPS_RECEPTIVE_FIELD_HISTOGRAM_H
#define CEDAR_PROC_STEPS_RECEPTIVE_FIELD_HISTOGRAM_H

// CEDAR CONFIGURATION
#include "cedar/configuration.h"

// CEDAR INCLUDES
#include <cedar/processing/Step.h>
#include <cedar/auxiliaries/UIntParameter.h>
#include <cedar/auxiliaries/UIntVectorParameter.h>
#include <cedar/auxiliaries/DoubleParameter.h>

// FORWARD DECLARATIONS
#include "steps/object_recognition/ReceptiveFieldHistogram.fwd.h"
#include <cedar/auxiliaries/MatData.fwd.h>

// SYSTEM INCLUDES


class cedar::proc::steps::ReceptiveFieldHistogram : public cedar::proc::Step
{
  //--------------------------------------------------------------------------------------------------------------------
  // nested types
  //--------------------------------------------------------------------------------------------------------------------
  Q_OBJECT

  //--------------------------------------------------------------------------------------------------------------------
  // constructors and destructor
  //--------------------------------------------------------------------------------------------------------------------
public:
  //!@brief The standard constructor.
  ReceptiveFieldHistogram();

  //--------------------------------------------------------------------------------------------------------------------
  // public methods
  //--------------------------------------------------------------------------------------------------------------------
public:
  unsigned int getNumberOfFeatureBins() const;

  //--------------------------------------------------------------------------------------------------------------------
  // protected methods
  //--------------------------------------------------------------------------------------------------------------------
protected:
  void inputConnectionChanged(const std::string& inputName);

  //--------------------------------------------------------------------------------------------------------------------
  // private methods
  //--------------------------------------------------------------------------------------------------------------------
private:
  void compute(const cedar::proc::Arguments&);

  template <typename InputType>
  void updateBinImage
       (
           const cv::Mat& input,
           cv::Mat& output,
           int bins,
           float lower,
           float upper
       );

  void calulateWeightedHistogram
       (
         const cv::Mat& binImage,
         const cv::Mat& weights,
         const cv::Mat& mask,
         cv::Mat histogram,
         int* center, // 2d array {row, col}
         int* weight_size // {rows, cols}; ignored if weights is not empty
       );


private slots:
  void resizeOutput();

  void recompute();

  void refreshLookupTable();

  void numberOfBinsChanged();

  void rangeChanged();

  void numberOfRFsChanged();

  //--------------------------------------------------------------------------------------------------------------------
  // members
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet
private:
  cedar::aux::ConstMatDataPtr mInput;

  cedar::aux::ConstMatDataPtr mReceptiveField;

  cedar::aux::ConstMatDataPtr mMask;

  cedar::aux::MatDataPtr mBinImage;

  cedar::aux::MatDataPtr mOutput;

  std::vector<short> mBinLookupTable;

  //--------------------------------------------------------------------------------------------------------------------
  // parameters
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

private:
  cedar::aux::UIntParameterPtr _mFeatureBins;

  cedar::aux::UIntVectorParameterPtr _mNumberOfReceptiveFields;

  cedar::aux::DoubleParameterPtr _mValueRangeLower;

  cedar::aux::DoubleParameterPtr _mValueRangeUpper;

  cedar::aux::BoolParameterPtr _mCyclicBins;

}; // class cedar::proc::steps::ReceptiveFieldHistogram

#endif // CEDAR_PROC_STEPS_RECEPTIVE_FIELD_HISTOGRAM_H

