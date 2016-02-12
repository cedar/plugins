/*======================================================================================================================

    Copyright 2011, 2012, 2013, 2014 Institut fuer Neuroinformatik, Ruhr-Universitaet Bochum, Germany
 
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

    File:        KeyPointData.cpp

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2014 12 08

    Description: Source file for the class keypoints::KeyPointData.

    Credits:

======================================================================================================================*/

// CEDAR CONFIGURATION
#include "cedar/configuration.h"

// CLASS HEADER
#include "data_structures/KeyPointData.h"

// CEDAR INCLUDES
#include <cedar/auxiliaries/stringFunctions.h>

// SYSTEM INCLUDES

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::aux::KeyPointData::KeyPointData(const std::vector<vislab::keypoints::KPData>& data)
:
Super(data)
{
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

std::string cedar::aux::KeyPointData::getDescription() const
{
  auto size = this->getData().size();
  std::string description;
  description += "Keypoint data with ";
  description += cedar::aux::toString(size);
  description += " scale";
  if (size == 0 || size > 1)
  {
    description += "s";
  }
  description += "<br />";
  description += "The scales are: ";
  bool first = true;
  for (const auto kpdata : this->getData())
  {
    if (first)
      first = false;
    else
      description += ", ";
      
    description += cedar::aux::toString(kpdata.lambda);
  }
  return description;
}
