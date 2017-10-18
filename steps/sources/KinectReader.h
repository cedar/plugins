/*
  This file is part of the action parsing project.

  Copyright (C) 2013 David Lobato <dav.lobato@gmail.com>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef UTILITIES_POINTCLOUD_OPENNIGRABBER_H
#define UTILITIES_POINTCLOUD_OPENNIGRABBER_H

// WORKAROUND FOR QT_FOREACH Macro
#include <boost/foreach.hpp>

// LOCAL INCLUDES
//#include "utilities_namespace.h"
#include "steps/sources/KinectReader.fwd.h"
#include "data_structures/RGBAPointCloudData.h"
// PROJECT INCLUDES
#include <iostream>

// SYSTEM INCLUDES
#include <QObject>
#include <boost/scoped_ptr.hpp>
#include <cedar/processing/Step.h>
#include <cedar/auxiliaries/StringParameter.h>
#include <cedar/auxiliaries/BoolParameter.h>
#include <cedar/auxiliaries/DoubleParameter.h>
#include <cedar/auxiliaries/MatData.h>
#include <cedar/auxiliaries/UIntParameter.fwd.h>
//#include <Eigen/Core>

/*!@brief Abstract description of the class.
 *
 * More detailed description of the class.
 */
class cedar::proc::steps::KinectReader : public cedar::proc::Step
{
  Q_OBJECT
  //--------------------------------------------------------------------------------------------------------------------
  // macros
  //--------------------------------------------------------------------------------------------------------------------

  //--------------------------------------------------------------------------------------------------------------------
  // constructors and destructor
  //--------------------------------------------------------------------------------------------------------------------
public:
  //!@brief The standard constructor.
  KinectReader();

  //!@brief Destructor
  ~KinectReader();

  //--------------------------------------------------------------------------------------------------------------------
  // public methods
  //--------------------------------------------------------------------------------------------------------------------
public:
  void compute(const cedar::proc::Arguments&);

public slots:
  void setKinectID();
  void setDownsamplingGrid();

  //--------------------------------------------------------------------------------------------------------------------
  // protected methods
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

  //--------------------------------------------------------------------------------------------------------------------
  // private methods
  //--------------------------------------------------------------------------------------------------------------------
private:
  void removeNanPointFromDepthImage();

  //--------------------------------------------------------------------------------------------------------------------
  // members
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // outputs
  cedar::aux::RGBAPointCloudDataPtr mPointCloud;
  cedar::aux::MatDataPtr mImage;
  cedar::aux::MatDataPtr mDepthImage;

private:
  struct PImpl;
  boost::scoped_ptr<PImpl> pImpl;

  //--------------------------------------------------------------------------------------------------------------------
  // parameters
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

private:
  cedar::aux::StringParameterPtr _mKinectID;

  cedar::aux::BoolParameterPtr _mGrabPC;
  cedar::aux::BoolParameterPtr _mGrabRGBImage;
  cedar::aux::BoolParameterPtr _mGrabDepthImage;
  cedar::aux::BoolParameterPtr _mRemoveNaNPoints;
  cedar::aux::BoolParameterPtr _mRemoveNaNDepth;

  //downsampling parameters
  cedar::aux::BoolParameterPtr _mDownsample;
  cedar::aux::DoubleParameterPtr _mDownsampleZMinLimit;
  cedar::aux::DoubleParameterPtr _mDownsampleZMaxLimit;
  cedar::aux::DoubleParameterPtr _mDownsampleLeafSize;
  cedar::aux::UIntParameterPtr _mImageWidth;
  cedar::aux::UIntParameterPtr _mImageHeight;

  cedar::aux::RGBAPointCloudPtr _mDownsampledCloud;

}; // class utilities::PointCloudOpenniGrabber

#endif // UTILITIES_POINTCLOUD_OPENNIGRABBER_H
