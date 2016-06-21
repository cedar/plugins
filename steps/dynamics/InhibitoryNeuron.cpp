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

    File:        InhibitoryNeuron.cpp

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2014 10 01

    Description: Source file for the class cedar::dyn::InhibitoryNeuron.

    Credits:

======================================================================================================================*/

// CEDAR CONFIGURATION
#include "cedar/configuration.h"

// CEDAR INCLUDES
#include "steps/dynamics/InhibitoryNeuron.h"
#include "cedar/processing/typecheck/Matrix.h"
#include "cedar/auxiliaries/math/transferFunctions/AbsSigmoid.h"
#include "cedar/units/Time.h"
#include "cedar/units/prefixes.h"

// SYSTEM INCLUDES

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::dyn::InhibitoryNeuron::InhibitoryNeuron()
:
mSigmoidedActivity(this, "sigmoided field activity"),
mInhibition(new cedar::aux::MatData(cv::Mat::zeros(1, 1, CV_32F))),
mSum(new cedar::aux::MatData(cv::Mat::zeros(1, 1, CV_32F))),
_mSigmoid(new cedar::dyn::InhibitoryNeuron::SigmoidParameter(this, "sigmoid", cedar::aux::math::SigmoidPtr(new cedar::aux::math::AbsSigmoid(0.0, 100.0)))),
_mTanhSlope(new cedar::aux::DoubleParameter(this, "tanh slope", 1.0)),
_mTau(new cedar::aux::TimeParameter(this, "timescale", cedar::unit::Time(10.0 * cedar::unit::milli * cedar::unit::seconds)))
{
  cedar::proc::typecheck::Matrix check;
  check.addAcceptedType(CV_32F);
  this->mSigmoidedActivity.getSlot()->setCheck(check);

  this->declareBuffer("input sum", mSum);
  this->declareOutput("inhibition activation", mInhibition);
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

void cedar::dyn::InhibitoryNeuron::eulerStep(const cedar::unit::Time& time)
{
  const cv::Mat& input = this->mSigmoidedActivity.getData();
  float& v = this->mInhibition->getData().at<float>(0, 0);

  float& sum = this->mSum->getData().at<float>(0, 0);
  sum = static_cast<float>(cv::sum(input)[0]);
  float sig_sum = this->_mSigmoid->getValue()->compute(sum);
  float slope = static_cast<float>(this->_mTanhSlope->getValue());

  float dv = -sig_sum * std::tanh(slope * (v - sum)) - (1.0 - sig_sum) * v;

  v += (time / this->_mTau->getValue()) * dv;
}
