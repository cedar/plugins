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
 
  Institute: Ruhr-Universitaet Bochum
  Institut fuer Neuroinformatik
 
  File: ConvertDepthToXyz.h
 
  Maintainer:  
  Email: 
  Date: 2015 11 June 
 
  Description: This step will detetect the border on value, i.e. show the delta matrix of the input

  Credits:
 
======================================================================================================================*/

#ifndef CEDAR_CONVERT_DEPTH_TO_XYZ_H
#define CEDAR_CONVERT_DEPTH_TO_XYZ_H

#include <QObject>

#include <cedar/processing/Step.h>
#include <cedar/processing/ExternalData.h>

#include <cedar/auxiliaries/MatData.h>
#include <cedar/auxiliaries/UIntVectorParameter.h>
#include <cedar/auxiliaries/DoubleVectorParameter.h>
#include <cedar/auxiliaries/DoubleParameter.h>

#include <cedar/auxiliaries/math/constants.h>

#include <boost/scoped_ptr.hpp>

#include "steps/utilities/ConvertDepthToXyz.fwd.h"

class cedar::proc::steps::ConvertDepthToXyz : public cedar::proc::Step
{
//--------------------------------------------------------------------------------------------------------------------
// macros
//--------------------------------------------------------------------------------------------------------------------
  Q_OBJECT

//----------------------------------------------------------------------------------------------------------------------
//    Public Methods
//----------------------------------------------------------------------------------------------------------------------
public:
  // Constructor
  ConvertDepthToXyz() ;
  // ~BorderDetection() ;

  //  Compute the delta of the input matrix
  void compute(const cedar::proc::Arguments& ) ;

  void setSizePoint(unsigned int dimension, unsigned int new_value);
  unsigned int getSizePoint(unsigned int dimension);
  void setSizeMeter(unsigned int dimension, double new_value);
  double getSizeMeter(unsigned int dimension);
  void setAlpha(double new_value);
  double getAlpha();
  void setMinDepth(double new_value);
  double getMinDepth();
  void setMaxDepth(double new_value);
  double getMaxDepth();

public slots:
  void updateMatrixSize();
  void updateKinectAngle();

//----------------------------------------------------------------------------------------------------------------------
//    Protected Methods
//----------------------------------------------------------------------------------------------------------------------
protected :
  //  none

//----------------------------------------------------------------------------------------------------------------------
//    Private Methods
//----------------------------------------------------------------------------------------------------------------------
private :
  //  none

//----------------------------------------------------------------------------------------------------------------------
//    Members
//----------------------------------------------------------------------------------------------------------------------
private :
  /*
  Output :
    mWorld3D   : Matrix 3D where are all the point with a constant value. They coordonate determine their position
          in the real world
    mSpanVector  :  Vector with the values of the different span (scale) axis in the matrix.

  Parameter :
    _mSizesPoint   : The sizes on point of the different dimensions.
    _mSizesMeter  :  ------------ meter ---------------------------.
    _mALpha     : The angle between the vertical and the camera plane. |/ (small one)


  Constant :
    focalLength : The focal length of the Depth camera kinect. (basicaly 580 pixels)
    conversion   : The coefficient to put the meter on the unit we want (100 is for cm)
  */

  // Output
  cedar::aux::MatDataPtr m3DObject;
  cedar::aux::MatDataPtr mSpanVector;
  cedar::aux::MatDataPtr mCenterMatrix;

  // Parameter
  cedar::aux::UIntVectorParameterPtr _mSizesPoint;
  cedar::aux::DoubleVectorParameterPtr _mSizesMeter;
  cedar::aux::DoubleParameterPtr _mAlpha;
  cedar::aux::DoubleParameterPtr _mMinDepth;
  cedar::aux::DoubleParameterPtr _mMaxDepth;

  // constant
  float focalLength = 525.0;  // vorher 525.0
  float conversion = 100.;
  float PI = cedar::aux::math::pi;
  float cameraHeight  = 53.5; // vorher 53.5

protected :
  //  none
} ;

#endif // CEDAR_CONVERT_DEPTH_TO_XYZ_H
