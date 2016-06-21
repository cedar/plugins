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

    File:        ViewCombination.cpp

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2014 04 10

    Description: Source file for the class cedar::proc::steps::ViewCombination.

    Credits:

======================================================================================================================*/

// CEDAR CONFIGURATION
#include "cedar/configuration.h"

// PROJECT INCLUDES
#include "steps/object_recognition/ViewCombination.h"

// CEDAR INCLUDES
#include <cedar/processing/typecheck/Matrix.h>
#include <cedar/auxiliaries/annotation/DiscreteMetric.h>
#include <cedar/auxiliaries/MatData.h>

// SYSTEM INCLUDES

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::proc::steps::ViewCombination::ViewCombination()
:
mObjectActivation(new cedar::aux::MatData(cv::Mat())),
_mNumberOfViews(new cedar::aux::UIntParameter(this, "number of views per object", 1)),
_mReverse(new cedar::aux::BoolParameter(this, "reverse mapping", false))
{
  auto input_slot = this->declareInput("label activation");
  cedar::proc::typecheck::Matrix input_check;
  input_check.addAcceptedDimensionality(1);
  input_check.addAcceptedType(CV_32F);
  input_slot->setCheck(input_check);

  this->mObjectActivation->setAnnotation(cedar::aux::annotation::DiscreteMetricPtr(new cedar::aux::annotation::DiscreteMetric()));
  this->declareOutput("object activation sum", this->mObjectActivation);

  QObject::connect(this->_mNumberOfViews.get(), SIGNAL(valueChanged()), this, SLOT(numberOfViewsChanged()));
  QObject::connect(this->_mReverse.get(), SIGNAL(valueChanged()), this, SLOT(reverseChanged()));
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

void cedar::proc::steps::ViewCombination::setNumberOfViews(unsigned int numViews)
{
  this->_mNumberOfViews->setValue(numViews);
}

void cedar::proc::steps::ViewCombination::numberOfViewsChanged()
{
  this->reallocateOutputMatrix();
}

void cedar::proc::steps::ViewCombination::reverseChanged()
{
  this->reallocateOutputMatrix();
}

unsigned int cedar::proc::steps::ViewCombination::getNumberOfLabels() const
{
  return cedar::aux::math::get1DMatrixSize(this->mLabelActivation->getData());
}

unsigned int cedar::proc::steps::ViewCombination::getNumberOfObjects() const
{
  if (this->_mReverse->getValue())
  {
    return this->getNumberOfLabels() * this->getNumberOfViews();
  }
  else
  {
    return this->getNumberOfLabels() / this->getNumberOfViews();
  }
}

void cedar::proc::steps::ViewCombination::reallocateOutputMatrix()
{
  if (!mLabelActivation)
  {
    this->mObjectActivation->setData(cv::Mat());
  }
  else
  {
    this->mObjectActivation->setData(cv::Mat::zeros(this->getNumberOfObjects(), 1, CV_32F));
  }

  this->emitOutputPropertiesChangedSignal("object activation sum");
}

void cedar::proc::steps::ViewCombination::inputConnectionChanged(const std::string& inputName)
{
  this->mLabelActivation = boost::dynamic_pointer_cast<cedar::aux::ConstMatData>(this->getInput(inputName));

  this->reallocateOutputMatrix();
}

void cedar::proc::steps::ViewCombination::compute(const cedar::proc::Arguments&)
{
  const cv::Mat& label_activation = this->mLabelActivation->getData();
  cv::Mat& object_activation = this->mObjectActivation->getData();
  object_activation.setTo(0.0);

  bool reverse = this->_mReverse->getValue();

  if (reverse)
  {
    for (unsigned int object_id = 0; object_id < cedar::aux::math::get1DMatrixSize(object_activation); ++object_id)
    {
      unsigned int label = object_id / this->getNumberOfViews();
      object_activation.at<float>(object_id, 0) += label_activation.at<float>(label);
    }
  }
  else
  {
    for (unsigned int label = 0; label < cedar::aux::math::get1DMatrixSize(label_activation); ++label)
    {
      unsigned int object_id = label / this->getNumberOfViews();
      object_activation.at<float>(object_id, 0) += label_activation.at<float>(label);
    }
  }
}
