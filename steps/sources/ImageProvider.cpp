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

    File:        ImageProvider.cpp

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2011 07 22

    Description:

    Credits:

======================================================================================================================*/

// LOCAL INCLUDES
#include "steps/sources/ImageProvider.h"

// CEDAR INCLUDES
#include <cedar/auxiliaries/DirectoryParameter.h>
#include <cedar/auxiliaries/NumericParameter.h>
#include <cedar/auxiliaries/MatData.h>
#include <cedar/auxiliaries/annotation/ColorSpace.h>
#include <cedar/processing/Trigger.h>

// SYSTEM INCLUDES
#include <QDir>
#include <opencv2/opencv.hpp>

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::proc::steps::ImageProvider::ImageProvider()
:
mDirectory(new cedar::aux::DirectoryParameter(this, "sourcesDirectory", "")),
mImageIndex(new cedar::aux::UIntParameter(this, "imageIndex", 0, 0, 0)),
mImage(new cedar::aux::MatData(cv::Mat::zeros(2, 2, CV_32F)))
{
  QObject::connect(this->mDirectory.get(), SIGNAL(valueChanged()), this, SLOT(directoryChanged()));
  QObject::connect(this->mImageIndex.get(), SIGNAL(valueChanged()), this, SLOT(selectionChanged()));

  this->declareOutput("image", mImage);

  this->mImage->setAnnotation
  (
    cedar::aux::annotation::ColorSpace::bgr()
  );
}

cedar::proc::steps::ImageProvider::~ImageProvider()
{
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

void cedar::proc::steps::ImageProvider::selectionChanged()
{
  if (this->mImages.size() <= this->mImageIndex->getValue())
    return;

  const std::string& image_path = this->mDirectory->getValue().absolutePath().toStdString()
                                + "/"
                                + this->mImages.at(this->mImageIndex->getValue());
  this->loadImage(image_path);
}

void cedar::proc::steps::ImageProvider::directoryChanged()
{
  this->mImages.clear();

  QStringList extensions;
  extensions << "*.png" << "*.jpg";
  QStringList image_list = this->mDirectory->getValue().entryList(extensions);
  for (int i = 0; i < image_list.size(); ++i)
  {
    this->mImages.push_back(image_list.at(i).toStdString());
  }
  this->mImageIndex->setMaximum(this->mImages.size() - 1);
  this->selectionChanged();
}

void cedar::proc::steps::ImageProvider::compute(const cedar::proc::Arguments&)
{
}

void cedar::proc::steps::ImageProvider::loadImage(const std::string& path)
{
  this->mImage->getData() = cv::imread(path);
  this->getFinishedTrigger()->trigger();
}
