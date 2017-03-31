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

#ifndef CEDAR_AUX_RGBAPOINTCLOUD_DATA_H
#define CEDAR_AUX_RGBAPOINTCLOUD_DATA_H

// FORWARD DECLARATIONS
#include "data_structures/RGBAPointCloudData.fwd.h"

// CEDAR INCLUDES
#include <cedar/auxiliaries/DataTemplate.h>

// SYSTEM INCLUDES
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <QReadWriteLock>

namespace pcl
{ 
class PointXYZRGBA;
}

namespace cedar{
namespace aux{
//CEDAR_DECLARE_AUX_CLASS(RGBAPointCloud);
typedef pcl::PointCloud<pcl::PointXYZRGBA> RGBAPointCloud;
CEDAR_GENERATE_POINTER_TYPES(RGBAPointCloud);

//!@brief A concretization of DataTemplate for RGBAPointCloud
//typedef DataTemplate<RGBAPointCloud> RGBAPointCloudData;

CEDAR_DECLARE_AUX_CLASS(RGBAPointCloudData);
} //namespace aux
} //namespace cedar

/*!@brief Data containing matrices.
 */
class cedar::aux::RGBAPointCloudData : public cedar::aux::DataTemplate<cedar::aux::RGBAPointCloud>
{
  //--------------------------------------------------------------------------------------------------------------------
  // macros
  //--------------------------------------------------------------------------------------------------------------------

  //--------------------------------------------------------------------------------------------------------------------
  // nested types
  //--------------------------------------------------------------------------------------------------------------------
private:
  typedef cedar::aux::DataTemplate<cedar::aux::RGBAPointCloud> Super;

  //--------------------------------------------------------------------------------------------------------------------
  // constructors and destructor
  //--------------------------------------------------------------------------------------------------------------------
public:
  //!@brief The standard constructor.
  RGBAPointCloudData();

  //!@brief This constructor initializes the internal data to a value.
  RGBAPointCloudData(const cedar::aux::RGBAPointCloud& value);

  ~RGBAPointCloudData();
  //--------------------------------------------------------------------------------------------------------------------
  // public methods
  //--------------------------------------------------------------------------------------------------------------------
public:
  std::string getDescription() const;

public:
  //untested:
  static void addPointToCloud(RGBAPointCloudPtr &cloud, float x, float y, float z, unsigned r, unsigned g, unsigned b)
  {
    pcl::PointXYZRGBA p;
    p.x = x;
    p.y = y;
    p.z = z;
    p.rgb = rgbPCLTransform(r,g,b);
    cloud->points.push_back(p);
    cloud->width = (int) cloud->points.size();
    cloud->height = 1;
  }

  //untested:
  static unsigned rgbPCLTransform(unsigned r, unsigned g, unsigned b)
  {
    return r << 16 | g << 8 | b;
  }

  /*!@brief Returns the dimensionality of the matrix stored in this data.
   *
   * @remarks Calls cedar::aux::math::getDimensionalityOf(this->getData()) to determine the dimensionality.
   */
  //unsigned int getDimensionality() const;

  //--------------------------------------------------------------------------------------------------------------------
  // protected methods
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

  //--------------------------------------------------------------------------------------------------------------------
  // private methods
  //--------------------------------------------------------------------------------------------------------------------
private:
  // none yet

  //--------------------------------------------------------------------------------------------------------------------
  // members
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

private:
  // none yet

}; // class cedar::aux::RGBAPointcloudData

#endif // CEDAR_AUX_RGBAPOINTCLOUD_DATA_H
