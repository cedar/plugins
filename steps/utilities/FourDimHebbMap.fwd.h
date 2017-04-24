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

    File:        DummyBehavior.fwd.h

    Maintainer:  Jan Tekülve
    Email:       jan.tekuelve@ini.rub.de
    Date:        2015 10 30

    Description: Forward declaration file for the class promoplugin::DummyBehavior.

    Credits:

======================================================================================================================*/

#ifndef FOUR_DIM_HEBBMAP_FWD_H
#define FOUR_DIM_HEBBMAP_FWD_H

// CEDAR CONFIGURATION
#include "cedar/configuration.h"

// CEDAR INCLUDES

// SYSTEM INCLUDES
#ifndef Q_MOC_RUN

#include <boost/smart_ptr.hpp>

#endif // Q_MOC_RUN


namespace cedar
{
  namespace proc
  {
    namespace steps
    {
      //!@cond SKIPPED_DOCUMENTATION
      class FourDimHebbMap;
      CEDAR_GENERATE_POINTER_TYPES(FourDimHebbMap);
      //!@endcond
    }
  }
}
#endif // FOUR_DIM_HEBBMAP_FWD_H

