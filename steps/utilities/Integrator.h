/*
 * cedar::proc::steps::Integrator.h
 *
 *  Created on: Mar 10, 2014
 *      Author: Jan Tekuelve
 */

#ifndef INTEGRATOR_H
#define INTEGRATOR_H

#include "steps/utilities/Integrator.fwd.h"

#include <cedar/processing/Step.h> 
#include "cedar/dynamics/Dynamics.h"
#include <cedar/processing/ExternalData.h> // getInputSlot() returns ExternalData
#include <cedar/processing/typecheck/DerivedFrom.h>
#include "cedar/auxiliaries/BoolParameter.h"
#include "cedar/auxiliaries/UIntParameter.h"
#include "cedar/auxiliaries/UIntVectorParameter.h"
#include "cedar/auxiliaries/DoubleParameter.h"
#include "cedar/auxiliaries/DoubleVectorParameter.h"
#include <vector>
#include <iostream>
#include <fstream>

//forward includes
#include "cedar/auxiliaries/MatData.fwd.h"

class cedar::proc::steps::Integrator : public cedar::dyn::Dynamics
{

  //	//--------------------------------------------------------------------------------------------------------------------
  //	// macros
  //	//--------------------------------------------------------------------------------------------------------------------
  Q_OBJECT

public:
  Integrator();
  virtual ~Integrator();
  void eulerStep(const cedar::unit::Time& time);
  cv::Mat getCurrentSum();
  void setInitValue(float dimension, float value);
  void resetSum();

public slots:
  //!@brief a slot that is triggered if the matrix size is changed
  void updateSize();
private:
  cedar::proc::DataSlot::VALIDITY determineInputValidity(
      cedar::proc::ConstDataSlotPtr slot,
      cedar::aux::ConstDataPtr data) const;
  //!@brief Resets the field.
  void reset();
private:
  cedar::aux::MatDataPtr mOutput;
  cedar::aux::UIntParameterPtr _mSize;
  cedar::aux::DoubleVectorParameterPtr _initValue;
  cedar::aux::BoolParameterPtr _useRestrictions;
  cedar::aux::DoubleVectorParameterPtr _lowerRestrictions;
  cedar::aux::DoubleVectorParameterPtr _upperRestrictions;
  cedar::aux::BoolParameterPtr _cyclicRestrictions;

  cv::Mat _mSum;
  std::vector<cedar::aux::DoubleVectorParameterPtr> mRestrictions;

};

#endif /* INTEGRATOR_H */
