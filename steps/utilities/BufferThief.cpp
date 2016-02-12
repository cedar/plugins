/*======================================================================================================================

    Copyright 2011, 2012, 2013, 2014, 2015, 2016 Institut fuer Neuroinformatik, Ruhr-Universitaet Bochum, Germany
 
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

    File:        BufferThief.cpp

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2012 06 06

    Description:

    Credits:

======================================================================================================================*/

// CEDAR CONFIGURATION
#include "cedar/configuration.h"

// LOCAL INCLUDES
#include "steps/utilities/BufferThief.h"

// CEDAR INCLUDES
#include <cedar/processing/Group.h>
#include <cedar/processing/exceptions.h>
#include <cedar/auxiliaries/stringFunctions.h>
#include <cedar/auxiliaries/MatData.h>
#include <cedar/auxiliaries/Log.h>

// SYSTEM INCLUDES

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::proc::steps::BufferThief::BufferThief()
:
_mPath(new cedar::aux::StringParameter(this, "buffer path", ""))
{
  this->declareOutput("buffer", cedar::aux::DataPtr(new cedar::aux::Data()));
  QObject::connect(this->_mPath.get(), SIGNAL(valueChanged()), this, SLOT(bufferPathChanged()));

  this->connectToGroupChanged(boost::bind(&cedar::proc::steps::BufferThief::bufferPathChanged, this));
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

void cedar::proc::steps::BufferThief::compute(const cedar::proc::Arguments&)
{
}

void cedar::proc::steps::BufferThief::bufferPathChanged()
{
  try
  {
    std::string step_path;
    std::string buffer_path;
    const std::string& path = this->getPath();
    cedar::aux::splitLast(path, ".", step_path, buffer_path);

    if (!this->getGroup())
    {
      return;
    }

    cedar::proc::ConnectablePtr connectable = this->getGroup()->getElement<cedar::proc::Connectable>(step_path);

    cedar::aux::ConstDataPtr buffer = connectable->getBuffer(buffer_path);
    this->setOutput("buffer", boost::const_pointer_cast<cedar::aux::Data>(buffer));

    cedar::aux::LogSingleton::getInstance()->message
    (
      "Buffer successfully stolen.",
      "void utilities::BufferThief::bufferPathChanged()",
      this->getName()
    );
  }
  catch (const cedar::aux::InvalidNameException&)
  {
    // ok, we don't care about exceptions, we'll just tell the user if things worked out
  }
}

