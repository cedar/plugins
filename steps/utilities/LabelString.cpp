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
#include "steps/utilities/LabelString.h"

// CEDAR INCLUDES
#include <cedar/processing/typecheck/Matrix.h>
#include <cedar/auxiliaries/math/tools.h>

// SYSTEM INCLUDES

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::proc::steps::LabelString::LabelString()
:
mInput(this, "label activation"),
mOutput(new cedar::aux::StringData("")),
mRankedList(new cedar::aux::StringData("")),
_mNumberOfStrings(new cedar::aux::UIntParameter(this, "number of strings", 0)),
_mStrings(new cedar::aux::StringVectorParameter(this, "strings", (std::vector<std::string>()))),
_mThreshold(new cedar::aux::DoubleParameter(this, "threshold", 0.5))
{
  cedar::proc::typecheck::Matrix input_check;
  input_check.addAcceptedDimensionality(1);
  input_check.addAcceptedNumberOfChannels(1);
  this->mInput.getSlot()->setCheck(input_check);

  this->declareOutput("output", this->mOutput);
  this->declareOutput("rank list", this->mRankedList);

  QObject::connect(this->_mNumberOfStrings.get(), SIGNAL(valueChanged()), this, SLOT(numberOfStringsChanged()));
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

void cedar::proc::steps::LabelString::setNumberOfStrings(unsigned int numberOfStrings)
{
  this->_mNumberOfStrings->setValue(numberOfStrings);
}

void cedar::proc::steps::LabelString::setString(unsigned int stringNumber, const std::string& string)
{
  this->_mStrings->setValue(stringNumber, string);
}

std::string cedar::proc::steps::LabelString::getLabelForIndex(unsigned int index) const
{
  if (index < this->_mStrings->size())
  {
    return this->_mStrings->at(index);
  }
  else
  {
    return "[unlabeled index " + cedar::aux::toString(index) + "]";
  }
}

void cedar::proc::steps::LabelString::compute(const cedar::proc::Arguments&)
{
  cv::Point max_loc;
  double max_val;
  const auto& input = this->mInput.getData();

  std::multimap<float, unsigned int> label_rankings;
  for (unsigned int i = 0; i < cedar::aux::math::get1DMatrixSize(input); ++i)
  {
    float activation = cedar::aux::math::getMatrixEntry<float>(input, i);
    label_rankings.insert(std::pair<float, unsigned int>(activation, i));
  }

  std::string ranking = "";
  unsigned int count = 0;
  for (auto iter = label_rankings.rbegin(); iter != label_rankings.rend(); ++iter)
  {
    ++count;
    if (iter != label_rankings.rbegin())
    {
      ranking += "\n";
    }

    ranking += cedar::aux::toString(count) + ": " + this->getLabelForIndex(iter->second) + "(" + cedar::aux::toString(iter->first) + ")";
  }

  this->mRankedList->setData(ranking);

  cv::minMaxLoc(input, nullptr, &max_val, nullptr, &max_loc);
  if (max_val > this->_mThreshold->getValue())
  {
    unsigned int index = static_cast<unsigned int>(max_loc.x);
    // if the index is zero, it might just be the dimension in which the matrix is of size 1; thus, swap
    if (index == 0)
    {
      index = static_cast<unsigned int>(max_loc.y);
    }

    this->mOutput->setData(this->getLabelForIndex(index));
  }
  else
  {
    this->mOutput->setData("(none)");
  }
}

void cedar::proc::steps::LabelString::numberOfStringsChanged()
{
  unsigned int size = this->_mNumberOfStrings->getValue();
  this->_mStrings->resize(size);
}
