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

  File:    ConvertDepthToXyz.cpp

  Maintainer:
  Email:
  Date:    2015 11 June

  Description:   We convert a 2 dimensionnal matrix with the value of depth of an object to a 3 dimensionnal matrix
                  that correspond to the XYZ of the world coordinate with the origin at the camera's base.

  Credits:

======================================================================================================================*/


#include "ConvertDepthToXyz.h"
#include <cmath>
#include <iostream>
#include <boost/scoped_ptr.hpp>
#include <fstream>

//CEDAR INCLUDES
#include <cedar/processing/ExternalData.h>
#include <cedar/auxiliaries/MatData.h>
#include <cedar/processing/typecheck/DerivedFrom.h>
#include <cedar/processing/typecheck/SameTypeCollection.h>
#include "cedar/processing/Arguments.h"

// CV 
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

cedar::proc::steps::ConvertDepthToXyz::ConvertDepthToXyz()
  : m3DObject(new cedar::aux::MatData( cv::Mat::zeros(1,1,CV_32FC1))),
    mSpanVector(new cedar::aux::MatData( cv::Mat::zeros(3,1,CV_32FC1))),
    mCenterMatrix(new cedar::aux::MatData( cv::Mat::zeros(3,1,CV_32FC1))),
    _mSizesPoint(new cedar::aux::UIntVectorParameter(this, "sizes of the matrix", 3, 30, 1, 1000.0)),
    _mSizesMeter(new cedar::aux::DoubleVectorParameter(this, "sizes on the world (meter)", 3, 1., 0.25, 10.0))  ,
    _mAlpha(new cedar::aux::DoubleParameter(this, "angle between horizontal and the kinect plane (°)",0.0,-90.,90.)),
    _mMinDepth(new cedar::aux::DoubleParameter(this, "Minimum Depth in cm",20.,0.0,500.)),
    _mMaxDepth(new cedar::aux::DoubleParameter(this, "Maximum Depth in cm",300.,0.0,500.))
{
  cedar::proc::DataSlotPtr mInputDepthObject = this->declareInput("inputDepthObject");
  mInputDepthObject->setCheck( cedar::proc::typecheck::DerivedFrom<cedar::aux::MatData>());

  this->declareOutput("3DObject", m3DObject);
  this->declareOutput("Scale", mSpanVector);
  this->declareOutput("Matrix Center", mCenterMatrix);

  QObject::connect(_mSizesPoint.get(), SIGNAL(valueChanged()), this, SLOT(updateMatrixSize()));
  QObject::connect(_mSizesMeter.get(), SIGNAL(valueChanged()), this, SLOT(updateMatrixSize()));
  QObject::connect(_mAlpha.get(), SIGNAL(valueChanged()), this, SLOT(updateKinectAngle()));
}


void cedar::proc::steps::ConvertDepthToXyz::compute(const cedar::proc::Arguments& )
{
  /*
  Variable :
    input       : can check if we have something in input
    inputData     : give the depth of the object
    outputMatrix   : the matrix 3D that go in the output at the end (constant for value)
    rowDepth     : row in the depth image
    colDepth     : col ------------------
    cX        : center of the depth image on x axis
    cY        : ---------------------------- y ----
    sizes      : size of the different dimension
    coefX      : coefficient to put the x screen in x of the kinect world coordinate
    coefY      : ---------------------- y ------------------------------------------
    depth       : value on a specific point of the input matrix
    xKinect      :  the x of the kinect world coordinate
    yKinect      :  the y ------------------------------
    xSpan       : span on the x axis, calculate by the size of the axis and the gap of value
    ySpan       : ----------- y ------------------------------------------------------------
    zSpan       : ----------- z ------------------------------------------------------------

  */
  // check if input are here

  cedar::proc::ExternalDataPtr input_slot = this->getInputSlot("inputDepthObject");
  if(input_slot->getDataCount() <= 0)
  {
    return ;
  }

  cedar::aux::ConstDataPtr input = input_slot->getData();

  cv::Mat input_data = input->getData<cv::Mat>();

  cv::Mat& output_matrix = this->m3DObject->getData();
  cv::Mat& output_span_vector = this->mSpanVector->getData();
  cv::Mat& output_center_matrix = this->mCenterMatrix->getData();

  float rad_alpha = _mAlpha->getValue()*this->PI/180.;

  int row_depth = input_data.rows;
  int col_depth = input_data.cols;
  int cX = row_depth/2;
  int cY = col_depth/2;

  int sizes[3] ;

  std::vector<float> coef_x;
  std::vector<float> coef_y;

  float x_span, y_span, z_span;
  float depth;
  float kinect_x, kinect_y;

  cv::Point3_<float> world_point;
  cv::Point3_<int> matrix_point;

  //Initialisation for easy variable
  sizes[0] = this->getSizePoint(0);
  sizes[1] = this->getSizePoint(1);
  sizes[2] = this->getSizePoint(2);

  output_matrix = cv::Mat(3, sizes, CV_32FC1, 0.0);

  //initialisation of the coefficient for the transformation of x and y screen to x and y kinect world

  for(int ind = 0; ind < row_depth; ind++)
  {
    coef_x.push_back(static_cast<float>(cX - ind)/focalLength);
  }


  for(int k = 0; k < col_depth; k++)
  {
    coef_y.push_back( static_cast<float>(k - cY)/focalLength);
  }

  //determine span
  output_span_vector.at<float>(0,0) = static_cast<float>(this->getSizeMeter(0))/static_cast<float>(sizes[0]);
  output_span_vector.at<float>(1,0) = static_cast<float>(this->getSizeMeter(1))/static_cast<float>(sizes[1]);
  output_span_vector.at<float>(2,0) = static_cast<float>(this->getSizeMeter(2))/static_cast<float>(sizes[2]);

  x_span = output_span_vector.at<float>(0,0)*conversion;
  y_span = output_span_vector.at<float>(1,0)*conversion;
  z_span = output_span_vector.at<float>(2,0)*conversion;

  output_center_matrix.at<float>(0,0) = static_cast<float>(sizes[0]/2);
  output_center_matrix.at<float>(1,0) = 0;
  output_center_matrix.at<float>(2,0) = 0;

  //go to fill the array of 3D points, and determine the min and max !
  for(int i = 0 ; i < row_depth ; i++)
  {
    for(int j = 0 ; j < col_depth ; j++)
    {
      depth = input_data.at<float>(i,j)*conversion;

      if(_mMinDepth->getValue() < depth && depth < _mMaxDepth->getValue())
      {
        // change the screen to kinect coordinate
        kinect_y = depth*coef_x[i];
        kinect_x = depth*coef_y[j];
        // change the kinect coordinate to the world coordinate

        world_point.x = kinect_x ;  //koordinates swap to match world
        world_point.z = depth*cos(rad_alpha) + kinect_y*sin(rad_alpha);
        world_point.y = -depth*sin(rad_alpha) + kinect_y*cos(rad_alpha) + cameraHeight;

        matrix_point.x = static_cast<int>((world_point.x/x_span) + output_center_matrix.at<float>(0,0));
        matrix_point.y = static_cast<int>((world_point.y/y_span) + output_center_matrix.at<float>(1,0));
        matrix_point.z = static_cast<int>((world_point.z/z_span) + output_center_matrix.at<float>(2,0));

        if
        (
          0 <= matrix_point.x && matrix_point.x < sizes[0]
          && 0 <= matrix_point.y && matrix_point.y < sizes[1]
          && 0 <= matrix_point.z && matrix_point.z < sizes[2]
        )
        {
          output_matrix.at<float>(sizes[0]-matrix_point.y,matrix_point.x,matrix_point.z) = 1.0 ;
        }
      }
    }
  }

  this->mCenterMatrix->setData(output_center_matrix);
  this->mSpanVector->setData(output_span_vector);
  this->m3DObject->setData(output_matrix);

  input->unlock() ;
}

void cedar::proc::steps::ConvertDepthToXyz::setSizePoint(unsigned int dimension, unsigned int new_value)
{
  this->_mSizesPoint->setValue(dimension, new_value);
}

unsigned int cedar::proc::steps::ConvertDepthToXyz::getSizePoint(unsigned int dimension)
{
  return this->_mSizesPoint->at(dimension);
}

void cedar::proc::steps::ConvertDepthToXyz::setSizeMeter(unsigned int dimension, double new_value)
{
  this->_mSizesMeter->setValue(dimension, new_value);
}

double cedar::proc::steps::ConvertDepthToXyz::getSizeMeter(unsigned int dimension)
{
  return this->_mSizesMeter->at(dimension);
}

void cedar::proc::steps::ConvertDepthToXyz::setAlpha(double new_value)
{
  this->_mAlpha->setValue(new_value);
}

double cedar::proc::steps::ConvertDepthToXyz::getAlpha()
{
  return this->_mAlpha->getValue();
}

void cedar::proc::steps::ConvertDepthToXyz::setMinDepth(double new_value)
{
  this->_mMinDepth->setValue(new_value);
}

double cedar::proc::steps::ConvertDepthToXyz::getMinDepth()
{
  return this->_mMinDepth->getValue();
}

void cedar::proc::steps::ConvertDepthToXyz::setMaxDepth(double new_value)
{
  this->_mMaxDepth->setValue(new_value);
}

double cedar::proc::steps::ConvertDepthToXyz::getMaxDepth()
{
  return this->_mMaxDepth->getValue();
}

void cedar::proc::steps::ConvertDepthToXyz::updateMatrixSize()
{
  this->lock( cedar::aux::LOCK_TYPE_READ );
  this->compute( cedar::proc::Arguments());
  this->unlock();
  this->emitOutputPropertiesChangedSignal("3DObject");
  this->emitOutputPropertiesChangedSignal("Scale");
  this->onTrigger();
}

void cedar::proc::steps::ConvertDepthToXyz::updateKinectAngle()
{
  this->lock( cedar::aux::LOCK_TYPE_READ );
  this->compute( cedar::proc::Arguments());
  this->unlock();
  this->emitOutputPropertiesChangedSignal("3DObject");
  this->onTrigger();
}
