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

    File:        LabelString.cpp

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2015 06 09

    Description: Source file for the class utilities::LabelString.

    Credits:

======================================================================================================================*/

// CEDAR CONFIGURATION
#include "cedar/configuration.h"

// CLASS HEADER
#include <cedar/processing/typecheck/Matrix.h>
#include <cedar/auxiliaries/math/tools.h>
#include <steps/utilities/PointWiseNormalization.h>

// SYSTEM INCLUDES

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::proc::steps::PointWiseNormalization::PointWiseNormalization()
:
mInput(this, "pattern"),
mNormalized(new cedar::aux::MatData()),
_mCorrelatedDimension(new cedar::aux::UIntParameter(this, "normalized dimension", 0))
{
  cedar::proc::typecheck::Matrix input_check;
  input_check.addAcceptedDimensionality(3);
  input_check.addAcceptedNumberOfChannels(1);
  this->mInput.getSlot()->setCheck(input_check);

  auto output_slot = this->declareOutput("normalized", this->mNormalized);

  this->mInput.addOutputRelation(boost::make_shared<cedar::proc::CopyMatrixProperties>(output_slot));

  QObject::connect(this->_mCorrelatedDimension.get(), SIGNAL(valueChanged()), this, SLOT(correlatedDimensionChanged()));
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

void cedar::proc::steps::PointWiseNormalization::compute(const cedar::proc::Arguments&)
{
  const cv::Mat& input = this->mInput.getData();
  cv::Mat& output = this->mNormalized->getData();
  int correlated_dimension = this->_mCorrelatedDimension->getValue();

  std::vector<int> dims;
  for (int i = 0; i < input.dims; ++i)
  {
    if (i != correlated_dimension)
    {
      dims.push_back(i);
    }
  }

  std::vector<cv::Range> ranges(static_cast<size_t>(input.dims), cv::Range::all());
  for (int d0 = 0; d0 < input.size[dims[0]]; ++d0)
  {
    ranges[dims[0]].start = d0;
    ranges[dims[0]].end = d0 + 1;
    for (int d1 = 0; d1 < input.size[dims[1]]; ++d1)
    {
      ranges[dims[1]].start = d1;
      ranges[dims[1]].end = d1 + 1;

      cv::Mat vector = input(&ranges.front());

      double mean = cv::mean(vector).val[0];

      cv::Mat mean_free = vector - mean;

      double norm = cv::norm(mean_free, cv::NORM_L2);

      if (std::abs(norm) < std::numeric_limits<float>::epsilon())
      {
        norm = 1.0;
      }

      cv::Mat normalized = mean_free / norm;

      normalized.copyTo(output(&ranges.front()));
    }
  }
}

void cedar::proc::steps::PointWiseNormalization::correlatedDimensionChanged()
{
  this->onTrigger();
}
