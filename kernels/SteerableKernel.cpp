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

    File:        SteeringKernel.cpp

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2011 07 27

    Description:

    Credits:

======================================================================================================================*/

// LOCAL INCLUDES
#include "kernels/SteerableKernel.h"

// PROJECT INCLUDES

// SYSTEM INCLUDES
#include <cedar/auxiliaries/MatData.h>
#include <cedar/auxiliaries/assert.h>

cedar::aux::EnumType<cedar::aux::kernel::SteerableKernel::Type> cedar::aux::kernel::SteerableKernel::Type::mType;

#ifndef MSVC
const cedar::aux::kernel::SteerableKernel::Type::Id cedar::aux::kernel::SteerableKernel::Type::G2a;
#endif

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::aux::kernel::SteerableKernel::SteerableKernel()
:
cedar::aux::kernel::Separable(2),
_mType(new cedar::aux::EnumParameter(this, "type", Type::typePtr(), Type::G2a)),
_mTaps(new cedar::aux::UIntParameter(this, "taps", 3, 1, 1000)),
_mSpacing(new cedar::aux::DoubleParameter(this, "spacing", 1.0, 0.0, 100.0))
{
  this->updateKernel();

  QObject::connect(_mType.get(), SIGNAL(valueChanged()), this, SLOT(recalculate()), Qt::DirectConnection);
  QObject::connect(_mTaps.get(), SIGNAL(valueChanged()), this, SLOT(recalculate()), Qt::DirectConnection);
  QObject::connect(_mSpacing.get(), SIGNAL(valueChanged()), this, SLOT(recalculate()), Qt::DirectConnection);
  QObject::connect(this->_mDimensionality.get(), SIGNAL(valueChanged()), this, SLOT(recalculate()), Qt::DirectConnection);
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

//!@todo Provide this as a standard slot in Kernel
void cedar::aux::kernel::SteerableKernel::recalculate()
{
  this->updateKernel();
}

void cedar::aux::kernel::SteerableKernel::calculateParts()
{
//  cv::Mat new_kernel;
  cv::Mat x, y;
  switch (this->_mType->getValue())
  {
    case Type::G2a:
      x = this->buildKernelPart(&cedar::aux::kernel::SteerableKernel::getG2F1, false);
      y = this->buildKernelPart(&cedar::aux::kernel::SteerableKernel::getG2F2, false);
      break;

    case Type::G2b:
      x = this->buildKernelPart(&cedar::aux::kernel::SteerableKernel::getG2F3, true);
      y = this->buildKernelPart(&cedar::aux::kernel::SteerableKernel::getG2F3, true);
      break;

    case Type::G2c:
      x = this->buildKernelPart(&cedar::aux::kernel::SteerableKernel::getG2F2, false);
      y = this->buildKernelPart(&cedar::aux::kernel::SteerableKernel::getG2F1, false);
      break;

    case Type::H2a:
      x = this->buildKernelPart(&cedar::aux::kernel::SteerableKernel::getH2F1, true);
      y = this->buildKernelPart(&cedar::aux::kernel::SteerableKernel::getH2F2, false);
      break;

    case Type::H2b:
      x = this->buildKernelPart(&cedar::aux::kernel::SteerableKernel::getH2F4, false);
      y = this->buildKernelPart(&cedar::aux::kernel::SteerableKernel::getH2F3, true);
      break;

    case Type::H2c:
      x = this->buildKernelPart(&cedar::aux::kernel::SteerableKernel::getH2F3, true);
      y = this->buildKernelPart(&cedar::aux::kernel::SteerableKernel::getH2F4, false);
      break;

    case Type::H2d:
      x = this->buildKernelPart(&cedar::aux::kernel::SteerableKernel::getH2F2, false);
      y = this->buildKernelPart(&cedar::aux::kernel::SteerableKernel::getH2F1, true);
      break;

    default:
      CEDAR_THROW(cedar::aux::UnhandledValueException, "Unhandled enum value in SteeringKernel::calculate.");
  }

//  new_kernel = dim2 * x.t();
//  mKernel->lockForWrite();
  //0 dim are rows ->y dimension
  if (this->kernelPartCount() > 0)
    this->setKernelPart(0, y);
  //1 dim are columns ->x dimension
  if (this->kernelPartCount() > 1)
    this->setKernelPart(1, x);
//  mKernel->getData() = new_kernel;
//  mKernel->unlock();
}

cv::Mat cedar::aux::kernel::SteerableKernel::buildKernelPart
                                   (
                                     boost::function<double(cedar::aux::kernel::SteerableKernel*, double)> valueFunction,
                                     bool negativeSymmetry
                                   )
{
  const double& spacing = _mSpacing->getValue();
  int taps =  2 * _mTaps->getValue() - 1;
  cv::Mat kernel_matrix(taps, 1, CV_32F);

  int middle = floor(taps/2.0);

  // set the middle element
  kernel_matrix.at<float>(middle, 0) = valueFunction(this, 0);

  double sym_factor = 1.0;
  if (negativeSymmetry)
  {
    sym_factor = -1.0;
  }

  // fill all other elements symmetrically around the middle element
  for (int i = 1; i < taps - middle; i++)
  {
    double x = static_cast<double>(i) * spacing;
    double value = valueFunction(this, x);
    kernel_matrix.at<float>(middle + i, 0) = value;
    kernel_matrix.at<float>(middle - i, 0) = sym_factor * value;
  }
  return kernel_matrix;
}

double cedar::aux::kernel::SteerableKernel::getG2F1(double x)
{
  return 0.9213 * (2 * pow(x, 2.0) - 1) * exp(-(pow(x, 2.0)));
}

double cedar::aux::kernel::SteerableKernel::getG2F2(double x)
{
  return exp(-(pow(x, 2.0)));
}

double cedar::aux::kernel::SteerableKernel::getG2F3(double x)
{
  return sqrt(1.843) * x * exp(-(pow(x, 2.0)));
}

double cedar::aux::kernel::SteerableKernel::getH2F1(double x)
{
  return  0.9780 * (-2.254 * x + pow(x, 3.0)) * exp(-(pow(x, 2.0)));
}

double cedar::aux::kernel::SteerableKernel::getH2F2(double x)
{
  return exp(-(pow(x, 2.0)));
}

double cedar::aux::kernel::SteerableKernel::getH2F3(double x)
{
  return  x * exp(-(pow(x, 2.0)));
}

double cedar::aux::kernel::SteerableKernel::getH2F4(double x)
{
  return 0.9780 * (-0.7515 + pow(x, 2.0)) * exp(-(pow(x, 2.0)));
}
