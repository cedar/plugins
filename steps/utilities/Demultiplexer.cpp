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

    File:        Demultiplexer.cpp

    Maintainer:  Guido Knips
    Email:       guido.knips@ini.rub.de
    Date:        2013 12 04

    Description:

    Credits:

======================================================================================================================*/

// LOCAL INCLUDES
#include "steps/utilities/Demultiplexer.h"

// PROJECT INCLUDES

// SYSTEM INCLUDES
#include <sstream>
#include <iostream>

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::proc::steps::Demultiplexer::Demultiplexer()
:
mOutputs(1, cedar::aux::MatDataPtr(new cedar::aux::MatData(cv::Mat(1,1, CV_32F)))),
mInput(new cedar::aux::MatData(cv::Mat::zeros(1,1, CV_32F))),
_mInputDimension (new cedar::aux::UIntParameter(this, "VectorDimension", 1,1,255))
{
  declareOutput("output_0", mOutputs[0]);
  declareInput("inputVector");

  QObject::connect(_mInputDimension.get(), SIGNAL(valueChanged()), this, SLOT(vectorDimensionChanged()));
}

cedar::proc::steps::Demultiplexer::~Demultiplexer()
{
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------
void cedar::proc::steps::Demultiplexer::compute(const cedar::proc::Arguments&)
{
  auto vec = mInput->getData();
  for (unsigned i=0; i<_mInputDimension->getValue(); i++)
  {
    vec.row(i).copyTo(mOutputs[i]->getData()); 
  }
}

void cedar::proc::steps::Demultiplexer::vectorDimensionChanged()
{
  unsigned newsize = _mInputDimension->getValue();
  unsigned oldsize = mOutputs.size();
  if (newsize == oldsize)
  {
    return;
  }
  //resize outputs vector
  mOutputs.resize(newsize);
  if (newsize < oldsize)
  {
    //delete unused slots
    for (unsigned i=newsize; i<oldsize; i++)
    {
      std::stringstream s;
      s << "output_" << i;
      removeOutputSlot(s.str());
    }
  }
  else if (newsize > oldsize)
  {
    for (unsigned i=oldsize; i<newsize; i++)
    {
      //initialize new output data
      mOutputs[i] = cedar::aux::MatDataPtr(new cedar::aux::MatData(cv::Mat::zeros(1,1,mOutputs[0]->getData().type())));
      //declare new input slot
      std::stringstream s;
      s << "output_" << i;
      declareOutput(s.str(), mOutputs[i]);
    }
  }
  //revalidate the input matrix
  getInputSlot("inputVector")->setValidity(cedar::proc::DataSlot::VALIDITY_UNKNOWN);
  
  onTrigger();
}

void cedar::proc::steps::Demultiplexer::inputConnectionChanged(const std::string& inputName)
{
  mInput = boost::dynamic_pointer_cast<const cedar::aux::MatData>(getInput(inputName));
  //change output type if it changed
  if (mInput && mInput->getData().type() != mOutputs[0]->getData().type())
  {
    for (unsigned i=0; i<mOutputs.size(); i++)
    {
      mOutputs[i]->getData().create(1,1,mInput->getData().type());

      //revalidate the outputs
      std::stringstream s;
      s << "output_" << i;
      emitOutputPropertiesChangedSignal(s.str());
    }
  }
  
  onTrigger();
}

cedar::proc::DataSlot::VALIDITY cedar::proc::steps::Demultiplexer::determineInputValidity
(
  cedar::proc::ConstDataSlotPtr,
  cedar::aux::ConstDataPtr data
)const
{
  cedar::aux::ConstMatDataPtr input = boost::dynamic_pointer_cast<const cedar::aux::MatData>(data);
  if (input)
  {
    //input must be one-dimensional and must have the correct size
    auto vec = input->getData();
    if (vec.cols == 1 && unsigned(vec.rows) == _mInputDimension->getValue())
    {
      return cedar::proc::DataSlot::VALIDITY_VALID;
    }
  }
  return cedar::proc::DataSlot::VALIDITY_ERROR;
}
