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

// CEDAR CONFIGURATION
#include "cedar/configuration.h"

// LOCAL INCLUDES 
#include "RGBAPointCloudData.h"

// CEDAR INCLUDES

#include <cedar/auxiliaries/stringFunctions.h>

// SYSTEM INCLUDES
#include <pcl/point_types.h>

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::aux::RGBAPointCloudData::RGBAPointCloudData() {}


cedar::aux::RGBAPointCloudData::RGBAPointCloudData(const cedar::aux::RGBAPointCloud& value) :
    cedar::aux::DataTemplate<cedar::aux::RGBAPointCloud>(value) {}

cedar::aux::RGBAPointCloudData::~RGBAPointCloudData() {}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

std::string cedar::aux::RGBAPointCloudData::getDescription() const
{
  std::string description;
  description += Super::getDescription();
  description += "<hr />";

  this->lockForRead();
  //const cedar::aux::RGBAPointCloud& pc = this->getData();
  //FIXME: fill description

  // if (mat.empty())
  // {
  //   description += "Empty matrix.";
  // }
  // else
  // {
  //   unsigned int dim = cedar::aux::math::getDimensionalityOf(mat);
  //   description += cedar::aux::toString(dim) + "-dimensional matrix";
  //   if (dim != 0)
  //   {
  //   description += "<br />size: ";
  //   if (dim == 1)
  //   {
  //     description += cedar::aux::toString(cedar::aux::math::get1DMatrixSize(mat));
  //   }
  //   else
  //   {
  //     for (unsigned int i = 0; i < dim; ++i)
  //     {
  //     if (i > 0)
  //     {
  //       description += " x ";
  //     }
  //     description += cedar::aux::toString(mat.size[i]);
  //     }
  //   }
  //   }
  //   description += "<br />";
  //   description += "type: " + cedar::aux::math::matrixTypeToString(mat) + "<br />";
  //   description += "channels: " + cedar::aux::toString(mat.channels());
  // }
  this->unlock();

  return description;
}

// unsigned int cedar::aux::MatData::getDimensionality() const
// {
//   return cedar::aux::math::getDimensionalityOf(this->mData);
// }
