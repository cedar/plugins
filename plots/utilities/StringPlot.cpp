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

    File:        StringPlot.cpp

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2015 06 09

    Description: Source file for the class utilities::StringPlot.

    Credits:

======================================================================================================================*/

// CEDAR CONFIGURATION
#include "cedar/configuration.h"

// CLASS HEADER
#include "plots/utilities/StringPlot.h"

// CEDAR INCLUDES

// SYSTEM INCLUDES
#include <QVBoxLayout>
#include <QFontMetrics>

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::aux::gui::StringPlot::StringPlot()
{
  auto layout = new QVBoxLayout();
  int margin = 2;
  layout->setContentsMargins(margin, margin, margin, margin);
  this->setLayout(layout);
  this->mpLabel = new QLabel();
  layout->addWidget(this->mpLabel);
  this->mpLabel->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

void cedar::aux::gui::StringPlot::plot(cedar::aux::ConstDataPtr data, const std::string& /* title */)
{
  this->mStringData = boost::dynamic_pointer_cast<cedar::aux::ConstStringData>(data);
  this->startTimer(100);
}

void cedar::aux::gui::StringPlot::resizeEvent(QResizeEvent* pEvent)
{
  this->cedar::aux::gui::PlotInterface::resizeEvent(pEvent);
  this->fitLabelTextToWidgetSize();
}

void cedar::aux::gui::StringPlot::fitLabelTextToWidgetSize()
{
  int increment = 1;
  QString str = this->mpLabel->text();
  QFont font = this->mpLabel->font();
  // starting size
  font.setPixelSize(5);

  // increment size until the text no longer fits
  while (true)
  {
    QFontMetrics font_metrics(font);
    QStringList lines = str.split('\n');

    int max_width = 0;
    for (auto line : lines)
    {
      max_width = std::max(max_width, font_metrics.width(line));
    }

    if (max_width >= this->mpLabel->width() || font_metrics.height() >= this->mpLabel->height())
    {
      break;
    }
    else
    {
      font.setPixelSize(font.pixelSize() + increment);
    }
  }
  this->mpLabel->setFont(font);
  this->mpLabel->setMinimumSize(0, 0);
}

void cedar::aux::gui::StringPlot::timerEvent(QTimerEvent*)
{
  QReadLocker l(&this->mStringData->getLock());
  std::string copy = this->mStringData->getData();
  l.unlock();

  this->mpLabel->setText(QString::fromStdString(copy));
  this->fitLabelTextToWidgetSize();
}
