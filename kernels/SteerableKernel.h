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

    File:        SteerableKernel.h

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2011 10 19

    Description:

    Credits:

======================================================================================================================*/

#ifndef CEDAR_AUX_KERNEL_STEERABLE_KERNEL_H
#define CEDAR_AUX_KERNEL_STEERABLE_KERNEL_H

// LOCAL INCLUDES
#include "kernels/SteerableKernel.fwd.h"

// PROJECT INCLUDES

// SYSTEM INCLUDES
#include <cedar/auxiliaries/EnumType.h>
#include <cedar/auxiliaries/EnumParameter.h>
#include <cedar/auxiliaries/DoubleParameter.h>
#include <cedar/auxiliaries/UIntParameter.h>
#include <cedar/auxiliaries/kernel/Separable.h>
#include <QReadWriteLock>
#include <boost/function.hpp>


/*!@brief Abstract description of the class.
 *
 * More detailed description of the class.
 */
class cedar::aux::kernel::SteerableKernel : public cedar::aux::kernel::Separable
{
  //--------------------------------------------------------------------------------------------------------------------
  // macros
  //--------------------------------------------------------------------------------------------------------------------
  Q_OBJECT

  //--------------------------------------------------------------------------------------------------------------------
  // nested types
  //--------------------------------------------------------------------------------------------------------------------
  class Type
  {
    public:
      typedef cedar::aux::EnumId Id;
      typedef boost::shared_ptr<cedar::aux::EnumBase> TypePtr;

      static void construct()
      {
        mType.type()->def(cedar::aux::Enum(G2a, "G2a"));
        mType.type()->def(cedar::aux::Enum(G2b, "G2b"));
        mType.type()->def(cedar::aux::Enum(G2c, "G2c"));

        mType.type()->def(cedar::aux::Enum(H2a, "H2a"));
        mType.type()->def(cedar::aux::Enum(H2b, "H2b"));
        mType.type()->def(cedar::aux::Enum(H2c, "H2c"));
        mType.type()->def(cedar::aux::Enum(H2d, "H2d"));
      }

      static const cedar::aux::EnumBase& type()
      {
        return *mType.type();
      }

      static const TypePtr& typePtr()
      {
        return mType.type();
      }

      static const Id G2a = 0;
      static const Id G2b = 1;
      static const Id G2c = 2;

      static const Id H2a = 3;
      static const Id H2b = 4;
      static const Id H2c = 5;
      static const Id H2d = 6;

    private:
      static cedar::aux::EnumType<Type> mType;
  };

  //--------------------------------------------------------------------------------------------------------------------
  // constructors and destructor
  //--------------------------------------------------------------------------------------------------------------------
public:
  //!@brief The standard constructor.
  SteerableKernel();

public slots:
  void recalculate();

  //--------------------------------------------------------------------------------------------------------------------
  // public methods
  //--------------------------------------------------------------------------------------------------------------------
public:
  void calculateParts();

  //--------------------------------------------------------------------------------------------------------------------
  // protected methods
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

  //--------------------------------------------------------------------------------------------------------------------
  // private methods
  //--------------------------------------------------------------------------------------------------------------------
private:
  double getG2F1(double x);
  double getG2F2(double x);
  double getG2F3(double x);

  double getH2F1(double x);
  double getH2F2(double x);
  double getH2F3(double x);
  double getH2F4(double x);

  cv::Mat buildKernelPart
          (
            boost::function<double(cedar::aux::kernel::SteerableKernel*, double)> valueFunction,
            bool negativeSymmetry
          );

  //--------------------------------------------------------------------------------------------------------------------
  // members
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

private:
  QReadWriteLock *mpLock;

  //--------------------------------------------------------------------------------------------------------------------
  // parameters
  //--------------------------------------------------------------------------------------------------------------------
private:
  cedar::aux::EnumParameterPtr _mType;
  cedar::aux::UIntParameterPtr _mTaps;
  cedar::aux::DoubleParameterPtr _mSpacing;

}; // class cedar::aux::kernel::SteerableKernel

#endif // CEDAR_AUX_KERNEL_STEERABLE_KERNEL_H
