/*======================================================================================================================

    Copyright 2011, 2012 Institut fuer Neuroinformatik, Ruhr-Universitaet Bochum, Germany
 
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

    File:        MotionKernel.cpp

    Maintainer:  Michael Berger
    Email:       michael.berger@ini.rub.de
    Date:        2012 09 20

    Description:

    Credits:

======================================================================================================================*/

// LOCAL INCLUDES
#include "kernels/MotionKernel.h"

// CEDAR INCLUDES
#include "cedar/auxiliaries/DoubleParameter.h"
#include "cedar/auxiliaries/MatData.h"
#include "cedar/auxiliaries/math/functions.h"
#include "cedar/auxiliaries/DoubleVectorParameter.h"
#include "cedar/auxiliaries/FactoryManager.h"
#include "cedar/auxiliaries/stringFunctions.h"
#include "cedar/auxiliaries/assert.h"
#include "cedar/auxiliaries/exceptions.h"
#include "cedar/auxiliaries/Log.h"

// SYSTEM INCLUDES
#include <iostream>
#include <limits.h>

//----------------------------------------------------------------------------------------------------------------------
// register the class
//----------------------------------------------------------------------------------------------------------------------
/*
namespace
{
  bool declare()
  {
    cedar::aux::kernel::FactoryManagerSingleton::getInstance()->registerType<cedar::aux::kernel::MotionKernelPtr>();
    cedar::aux::kernel::FactoryManagerSingleton::getInstance()->addDeprecatedName<cedar::aux::kernel::MotionKernelPtr>("motionDetection.MotionKernel");
    return true;
  }

  bool declared
    = declare();

}
*/

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------
cedar::aux::kernel::MotionKernel::MotionKernel
(
  unsigned int dimensionality,
  double amplitudes,
  double sigmas,
  double shifts,
  double inhs,
  double limit
)
:
cedar::aux::kernel::Separable(),
_mAmplitudes(new cedar::aux::DoubleVectorParameter(this, "amplitude", dimensionality, amplitudes, -10000.0, 10000.0)),
_mSigmas(new cedar::aux::DoubleVectorParameter(this, "sigmas", dimensionality, sigmas, 0.01, 10000)),
_mShifts(new cedar::aux::DoubleVectorParameter(this, "shifts", dimensionality, shifts, -10000.0, 10000)),
_mInhs(new cedar::aux::DoubleVectorParameter(this, "inhibition", dimensionality, inhs, -10000.0, 0)),
_mLimit(new cedar::aux::DoubleParameter(this, "limit", limit, 0.01, 1000.0))
{
  cedar::aux::LogSingleton::getInstance()->allocating(this);
  this->setDimensionality(dimensionality);
  
  this->onInit();
}

cedar::aux::kernel::MotionKernel::MotionKernel(
                                  std::vector<double> amplitudes,
                                  std::vector<double> sigmas,
                                  std::vector<double> shifts,
                                  std::vector<double> inhs,
                                  double limit,
                                  unsigned int dimensionality
                                )
:
cedar::aux::kernel::Separable(dimensionality),
_mAmplitudes(new cedar::aux::DoubleVectorParameter(this, "amplitude", amplitudes, -10000.0, 10000.0)),
_mSigmas(new cedar::aux::DoubleVectorParameter(this, "sigmas", sigmas, 0.01, 10000)),
_mShifts(new cedar::aux::DoubleVectorParameter(this, "shifts", shifts, -10000.0, 10000)),
_mInhs(new cedar::aux::DoubleVectorParameter(this, "inhibition", inhs, -10000.0, 0)),
_mLimit(new cedar::aux::DoubleParameter(this, "limit", limit, 0.01, 1000.0))
{
  cedar::aux::LogSingleton::getInstance()->allocating(this);

  CEDAR_ASSERT(amplitudes.size() == dimensionality);
  CEDAR_ASSERT(sigmas.size() == dimensionality);
  CEDAR_ASSERT(shifts.size() == dimensionality);
  CEDAR_ASSERT(inhs.size() == dimensionality);

  this->mCenters.resize(dimensionality);
  this->mSizes.resize(dimensionality);
  this->onInit();
}


cedar::aux::kernel::MotionKernel::~MotionKernel()
{
  cedar::aux::LogSingleton::getInstance()->freeing(this);
}
//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

void cedar::aux::kernel::MotionKernel::onInit()
{
  updateDimensionality();
  QObject::connect(_mAmplitudes.get(), SIGNAL(valueChanged()), this, SLOT(updateKernel()));
  QObject::connect(_mLimit.get(), SIGNAL(valueChanged()), this, SLOT(updateKernel()));
  QObject::connect(_mSigmas.get(), SIGNAL(valueChanged()), this, SLOT(updateKernel()));
  QObject::connect(_mShifts.get(), SIGNAL(valueChanged()), this, SLOT(updateKernel()));
  QObject::connect(_mInhs.get(), SIGNAL(valueChanged()), this, SLOT(updateKernel()));
  QObject::connect(_mDimensionality.get(), SIGNAL(valueChanged()), this, SLOT(updateDimensionality()));
}

void cedar::aux::kernel::MotionKernel::calculateParts()
{
  mpReadWriteLockOutput->lockForWrite();
  unsigned int dimensionality = this->getDimensionality();
  //const double& amplitude = _mAmplitude->getValue(); //Gauss class: this is important if amplitude is not a vector
  // sanity check

  // assert the correct size of all parameters & lists
  CEDAR_DEBUG_ASSERT((dimensionality == 0 && _mAmplitudes->size() == 1) || _mAmplitudes->size() == dimensionality);
  CEDAR_DEBUG_ASSERT((dimensionality == 0 && _mSigmas->size() == 1) || _mSigmas->size() == dimensionality);
  CEDAR_DEBUG_ASSERT((dimensionality == 0 && _mShifts->size() == 1) || _mShifts->size() == dimensionality);
  CEDAR_DEBUG_ASSERT((dimensionality == 0 && _mInhs->size() == 1) || _mInhs->size() == dimensionality);
  CEDAR_DEBUG_ASSERT((dimensionality == 0 && mSizes.size() == 1)   || mSizes.size() == dimensionality);
  CEDAR_DEBUG_ASSERT((dimensionality == 0 && mCenters.size() == 1) || mCenters.size() == dimensionality);

  // calculate the kernel parts for every dimension
  if (dimensionality > 0)
  {
    for (unsigned int dim = 0; dim < dimensionality; dim++)
    {
      double sigma = _mSigmas->at(dim);
      // estimate width
      if (sigma != 0)
      {
        this->mSizes.at(dim) = this->estimateWidth(dim);
      }
      else
      {
        this->mSizes.at(dim) = 1;
      }
      this->mCenters.at(dim) = static_cast<int>(mSizes.at(dim) / 2) + _mShifts->at(dim);
      cv::Mat kernel_part = cv::Mat::zeros(mSizes.at(dim), 1, CV_32F);

      // calculate kernel part
      if (sigma != 0)
      {
        for (unsigned int j = 0; j < mSizes.at(dim); j++)
        {
          // init the Gauss kernel
          //!\todo move filling up of matrix to some tool function
          kernel_part.at<float>(j, 0)
            = cedar::aux::math::gauss(static_cast<int>(j) - mCenters.at(dim), sigma);
        }
      }
      else // discrete case
      {
        //kernel_part.at<float>(0, 0) = 1; // Gauss class: sigma = 0  initializes a discrete kernel
        //here: it is a 0-Matrix to enable global inhibition without excitation
      }
      // normalize, modification: scale the result with the amplitude
      kernel_part /= ( cv::sum(kernel_part).val[0] / _mAmplitudes->at(dim) );

      // Modification: (global inhibition per dimension)
      // after computing the normalized Gauss, the global inhibition term is added to the function
      // (more efficient implementation possible?)
      for (unsigned int j = 0; j < mSizes.at(dim); j++)
      {
        //!\todo move filling up of matrix to some tool function
        kernel_part.at<float>(j, 0) += _mInhs->at(dim);
      }

      this->setKernelPart(dim, kernel_part);
    }

    // Gauss class: set the amplitude (non-vector) to the first dimension
    // this->setKernelPart(0, amplitude * this->getKernelPart(0) );
  }
  else
  {
    this->setKernelPart(0, _mAmplitudes->at(0) * cv::Mat::ones(1, 1, CV_32F));
  }

  mpReadWriteLockOutput->unlock();
}

void cedar::aux::kernel::MotionKernel::setSigma(unsigned int dimension, double sigma)
{
  if (dimension < _mSigmas->size())
  {
    _mSigmas->setValue(dimension, sigma);
  }
  else
  {
    CEDAR_THROW(cedar::aux::IndexOutOfRangeException, "Error in motionDetection::MotionKernel::setSigma: vector out of bounds");
  }
}

double cedar::aux::kernel::MotionKernel::getSigma(unsigned int dimension) const
{
  return _mSigmas->at(dimension);
}

void cedar::aux::kernel::MotionKernel::setShift(unsigned int dimension, double shift)
{
  if (dimension < this->_mShifts->size())
  {
    _mShifts->setValue(dimension, shift);
  }
  else
  {
    CEDAR_THROW(cedar::aux::IndexOutOfRangeException, "Error in motionDetection::MotionKernel::setShift: vector out of bounds");
  }
}

double cedar::aux::kernel::MotionKernel::getShift(unsigned int dimension) const
{
  return _mShifts->at(dimension);
}

void cedar::aux::kernel::MotionKernel::setInh(unsigned int dimension, double inh)
{
  if (dimension < this->_mInhs->size())
  {
    _mInhs->setValue(dimension, inh);
  }
  else
  {
    CEDAR_THROW(cedar::aux::IndexOutOfRangeException, "Error in motionDetection::MotionKernel::setInh: vector out of bounds");
  }
}

double cedar::aux::kernel::MotionKernel::getInh(unsigned int dimension) const
{
  return _mInhs->at(dimension);
}

void cedar::aux::kernel::MotionKernel::setAmplitude(unsigned int dimension, double amplitude)
{
  if (dimension < this->_mAmplitudes->size())
  {
    _mAmplitudes->setValue(dimension, amplitude);
  }
  else
  {
    CEDAR_THROW(cedar::aux::IndexOutOfRangeException, "Error in motionDetection::MotionKernel::setAmplitude: vector out of bounds");
  }
}

double cedar::aux::kernel::MotionKernel::getAmplitude(unsigned int dimension) const
{
  return _mAmplitudes->at(dimension);
}

void cedar::aux::kernel::MotionKernel::setLimit(double limit)
{
  _mLimit->setValue(limit);
}

unsigned int cedar::aux::kernel::MotionKernel::estimateWidth(unsigned int dim) const
{
  unsigned int tmp;
  /* size of kernel is determined by limit * sigma
   */
  tmp = static_cast<unsigned int>(ceil(_mLimit->getValue() * _mSigmas->at(dim)));
  // check if kernel size is even and if so, make it odd
  if (tmp % 2 == 0)
  {
    tmp++;
  }
  return tmp;
}

void cedar::aux::kernel::MotionKernel::updateDimensionality()
{
  mpReadWriteLockOutput->lockForWrite();
  unsigned int new_dimensionality = this->getDimensionality();
  unsigned int new_size = new_dimensionality;
  if (new_dimensionality == 0)
  {
    new_size = 1;
  }

  _mAmplitudes->resize(new_size);
  _mAmplitudes->setDefaultSize(new_size);
  _mSigmas->resize(new_size);
  _mSigmas->setDefaultSize(new_size);
  _mShifts->resize(new_size);
  _mShifts->setDefaultSize(new_size);
  _mInhs->resize(new_size);
  _mInhs->setDefaultSize(new_size);
  this->mCenters.resize(new_size);
  this->mSizes.resize(new_size);
  mpReadWriteLockOutput->unlock();

  this->updateKernel();
}
