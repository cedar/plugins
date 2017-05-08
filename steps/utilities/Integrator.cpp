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
      mOutput(new cedar::aux::MatData(cv::Mat::zeros(1, 1, CV_32F))),
      _mSize(new cedar::aux::UIntParameter(this, "dimensionality", 1, 1, 100)),
      _initValue(new cedar::aux::DoubleVectorParameter(this, "Initial Position", 1, 0)),
      _useRestrictions(new cedar::aux::BoolParameter(this, "use restrictions", false)),
      _lowerRestrictions(new cedar::aux::DoubleVectorParameter(this, "lower restrictions", 1, 0)),
      _upperRestrictions(new cedar::aux::DoubleVectorParameter(this, "upper restrictions", 1, 0)),
      _cyclicRestrictions(new cedar::aux::BoolParameter(this, "cyclic restrictions", false)),
      _mSum(cv::Mat(_mSize->getValue(), 1, CV_32F, _initValue->getDefaultValue()))
{
  this->declareInput("velocity");
  this->declareInput("overwriteInput", false);
  this->declareInput("overwritePeakDetector", false);
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

    cedar::aux::ConstDataPtr overWriteInput = this->getInputSlot("overwriteInput")->getData();
    cedar::aux::ConstDataPtr overWritePeakDetector = this->getInputSlot("overwritePeakDetector")->getData();
    if (boost::dynamic_pointer_cast<const cedar::aux::MatData>(overWriteInput) && boost::dynamic_pointer_cast<const cedar::aux::MatData>(overWriteInput) )
    {
      auto overWriteMat = overWriteInput->getData<cv::Mat>().clone();
      auto overWritePeak = overWritePeakDetector->getData<cv::Mat>().clone();
      if(overWritePeak.at<float>(0,0)> 0.5)
      {
        _mSum = overWriteMat;
//        std::cout<<"Current Input is overwritten to: "<< _mSum<< std::endl;
      }
    }

    for (unsigned int d = 0; d < _mSize->getValue(); d++)
    {
      _mSum.at<float>(d, 0) += time / cedar::unit::Time(1 * cedar::unit::seconds) * inputValueMat.at<float>(d, 0);
      if (_useRestrictions->getValue())
      {
        if (_mSum.at<float>(d, 0) > _upperRestrictions->getValue().at(d))
        {
          if (_cyclicRestrictions->getValue())
            _mSum.at<float>(d, 0) = _lowerRestrictions->getValue().at(d);
          else
            _mSum.at<float>(d, 0) = _upperRestrictions->getValue().at(d);
        }
        if (_mSum.at<float>(d, 0) < _lowerRestrictions->getValue().at(d))
        {
          if (_cyclicRestrictions->getValue())
            _mSum.at<float>(d, 0) = _upperRestrictions->getValue().at(d);
          else
            _mSum.at<float>(d, 0) = _lowerRestrictions->getValue().at(d);
        }
      }
//      std::cout<<"mSum after!: "<<_mSum<<std::endl;

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

  _upperRestrictions->resize(new_size, _upperRestrictions->getDefaultValue());
  _upperRestrictions->setDefaultSize(new_size);
  _lowerRestrictions->resize(new_size, _lowerRestrictions->getDefaultValue());
  _lowerRestrictions->setDefaultSize(new_size);

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

cedar::proc::DataSlot::VALIDITY cedar::proc::steps::Integrator::determineInputValidity(cedar::proc::ConstDataSlotPtr, cedar::aux::ConstDataPtr data) const
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

