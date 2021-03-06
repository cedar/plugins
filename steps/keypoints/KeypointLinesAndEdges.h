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

    File:        LinesAndEdges.h

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2011 12 22

    Description:

    Credits:

======================================================================================================================*/

#ifndef CEDAR_PROC_STEPS_KEYPOINTS_LINES_AND_EDGES_H
#define CEDAR_PROC_STEPS_KEYPOINTS_LINES_AND_EDGES_H

// LOCAL INCLUDES
#include "steps/keypoints/KeypointDataExtractorBase.h"

// FORWARD DECLARATIONS
#include "steps/keypoints/KeypointLinesAndEdges.fwd.h"

// SYSTEM INCLUDES


/*!@brief A class that multiplies two matrices .
 */
class cedar::proc::steps::KeypointLinesAndEdges : public cedar::proc::steps::KeypointDataExtractorBase
{
  //--------------------------------------------------------------------------------------------------------------------
  // macros
  //--------------------------------------------------------------------------------------------------------------------
  //--------------------------------------------------------------------------------------------------------------------
  // constructors and destructor
  //--------------------------------------------------------------------------------------------------------------------
public:
  //!@brief The standard constructor.
  KeypointLinesAndEdges();

  //--------------------------------------------------------------------------------------------------------------------
  // public methods
  //--------------------------------------------------------------------------------------------------------------------
public:
  // none yet

  //--------------------------------------------------------------------------------------------------------------------
  // protected methods
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

  //--------------------------------------------------------------------------------------------------------------------
  // private methods
  //--------------------------------------------------------------------------------------------------------------------
private:
  void compute(const cedar::proc::Arguments& args);

  //--------------------------------------------------------------------------------------------------------------------
  // members
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

private:
  //! The output map
  cedar::aux::MatDataPtr mLinesEdgesLocations;

  //! The output map
  cedar::aux::MatDataPtr mLinesEdgesOrientations;

  //--------------------------------------------------------------------------------------------------------------------
  // parameters
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

private:

}; // class cedar::proc::steps::KeypointLinesAndEdges

#endif // CEDAR_PROC_STEPS_KEYPOINTS_LINES_AND_EDGES_H
