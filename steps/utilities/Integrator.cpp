/*
 * cedar::proc::steps::Integrator.cpp
 *
 *  Created on: Mar 10, 2014
 *      Author: Jan Tekuelve
 */

#include "steps/utilities/Integrator.h"
#include <cedar/auxiliaries/MatData.h>
#include "cedar/processing/Arguments.h"
#include "cedar/units/Time.h"
#include "cedar/units/prefixes.h"

cedar::proc::steps::Integrator::Integrator()
    :
      mOutput(new cedar::aux::MatData(cv::Mat::zeros(2, 1, CV_32F))),
      _mSize(new cedar::aux::UIntParameter(this, "dimensionality", 1, 1, 100)),
      _initValue(new cedar::aux::DoubleVectorParameter(this, "Initial Position", 1, 0)),
      _mSum(cv::Mat(_mSize->getValue(), 1, CV_32F, _initValue->getDefaultValue()))
{
  this->declareInput("velocity");
  this->declareOutput("lambda", mOutput);
  //TODO Rename Input, Output and Parameters

  QObject::connect(_mSize.get(), SIGNAL(valueChanged()), this, SLOT(updateSize()));
}

cedar::proc::steps::Integrator::~Integrator()
{
  // TODO Auto-generated destructor stub
}

void cedar::proc::steps::Integrator::eulerStep(const cedar::unit::Time& time)
{
  cv::Mat& output = mOutput->getData();
  cedar::aux::ConstDataPtr inputData = this->getInputSlot("velocity")->getData();

  if (boost::dynamic_pointer_cast<const cedar::aux::MatData>(inputData))
  {
    cv::Mat inputValueMat = inputData->getData<cv::Mat>().clone();

    for (unsigned int d = 0; d < _mSize->getValue(); d++)
    {
      _mSum.at<float>(d, 0) += time / cedar::unit::Time(1 * cedar::unit::seconds) * inputValueMat.at<float>(d, 0);
      output.at<float>(d, 0) = _mSum.at<float>(d, 0);
    }
  }
}

cv::Mat cedar::proc::steps::Integrator::getCurrentSum()
{
  return _mSum;
}

void cedar::proc::steps::Integrator::resetSum()
{
  for (unsigned int i = 0; i < _mSize->getValue(); i++)
  {
    _mSum.at<float>(i, 0) = _initValue->getValue().at(i);
  }

}

void cedar::proc::steps::Integrator::setInitValue(float dimension, float value)
{
  _initValue->setValue(dimension, value);
}

void cedar::proc::steps::Integrator::updateSize()
{
  int new_size = static_cast<int>(_mSize->getValue());
  _initValue->resize(new_size, _initValue->getDefaultValue());
  _initValue->setDefaultSize(new_size);
  _mSum = cv::Mat::zeros(new_size, 1, CV_32F);
  for (int i = 0; i < new_size; i++)
  {
    _mSum.at<float>(i, 0) = _initValue->getValue().at(i);
  }
  mOutput->setData(cv::Mat::zeros(new_size, 1, CV_32F));
  this->emitOutputPropertiesChangedSignal("lambda");
}

void cedar::proc::steps::Integrator::reset()
{
  this->resetSum();
  cv::Mat& output = mOutput->getData();

  for (unsigned int d = 0; d < _mSize->getValue(); d++)
  {
    float sum = _mSum.at<float>(d, 0);
    output.at<float>(d, 0) = sum;
  }
}

cedar::proc::DataSlot::VALIDITY cedar::proc::steps::Integrator::determineInputValidity(
    cedar::proc::ConstDataSlotPtr,
    cedar::aux::ConstDataPtr data) const
{
  if (cedar::aux::ConstMatDataPtr input = boost::dynamic_pointer_cast<const cedar::aux::MatData>(data))
  {
    unsigned int matrixRows = input->getData().rows;
    unsigned int matrixColumns = input->getData().cols;

    if (matrixRows == _mSize->getValue() && matrixColumns == 1)
    {
      return cedar::proc::DataSlot::VALIDITY_VALID;
    }
  }
  return cedar::proc::DataSlot::VALIDITY_ERROR;
}

