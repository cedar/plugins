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

    File:        WeightedSumOfSlices.h

    Maintainer:  christian faubel
    Email:       christian.faubel@ini.ruhr-uni-bochum.de
    Date:        2012 01 27

    Description:

    Credits:

======================================================================================================================*/

#ifndef CEDAR_PROC_STEPS_WEIGHTED_SUM_OF_SLICES_H
#define CEDAR_PROC_STEPS_WEIGHTED_SUM_OF_SLICES_H

// LOCAL INCLUDES
#include "steps/utilities/WeightedSumOfSlices.fwd.h"

// PROJECT INCLUDES

// SYSTEM INCLUDES
#include <cedar/processing/Step.h>
#include <cedar/auxiliaries/UIntParameter.h>
#include <cedar/auxiliaries/MatData.h>

/*!@brief Processing step that extracts a slice of a matrix.
 */
class cedar::proc::steps::WeightedSumOfSlices : public cedar::proc::Step
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
  WeightedSumOfSlices();

  //!@brief Destructor
  ~WeightedSumOfSlices();

  //--------------------------------------------------------------------------------------------------------------------
  // public methods
  //--------------------------------------------------------------------------------------------------------------------
public:
  void inputConnectionChanged(const std::string& inputName);

  cedar::proc::DataSlot::VALIDITY determineInputValidity(cedar::proc::ConstDataSlotPtr, cedar::aux::ConstDataPtr) const;

  inline cedar::aux::ConstMatDataPtr getWeightedSum() const
  {
    return this->mOutput;
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
  void compute(const cedar::proc::Arguments&);

  bool checkIfMaticesFit(const cv::Mat& input, const cv::Mat& weights) const;


private slots:
  void allocateOutput();

  //--------------------------------------------------------------------------------------------------------------------
  // members
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

private:
  //input
  cedar::aux::ConstMatDataPtr mInput;
  cedar::aux::ConstMatDataPtr mWeightVector;

  //output
  cedar::aux::MatDataPtr mOutput;

  //--------------------------------------------------------------------------------------------------------------------
  // parameters
  //--------------------------------------------------------------------------------------------------------------------
private:
  cedar::aux::UIntParameterPtr _mWeightedDimension;
}; // class cedar::proc::steps::WeightedSumOfSlices

#endif // CEDAR_PROC_STEPS_WEIGHTED_SUM_OF_SLICES_H
