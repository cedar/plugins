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

    File:        PadAndTranslate.cpp

    Maintainer:  Jan Tekülve
    Email:       jan.tekuelve@ini.rub.de
    Date:        2016 01 12

    Description: Source file for the class cedar::proc::steps::PadAndTranslate.

    Credits:

======================================================================================================================*/

// CEDAR CONFIGURATION
#include "cedar/configuration.h"

// CLASS HEADER
#include "steps/utilities/PadAndTranslate.h"
#include <cedar/auxiliaries/MatData.h>

// CEDAR INCLUDES

// SYSTEM INCLUDES

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::proc::steps::PadAndTranslate::PadAndTranslate():
mOutputSize(new cedar::aux::IntParameter(this,"OutputSize")),
mOutput(new cedar::aux::MatData(cv::Mat::zeros(mOutputSize->getValue(), 1, CV_32F)))
{
	this->declareInput("input", true);
	this->declareInput("translation", true);
	// Declare the output and set it to the output matrix defined above.
	this->declareOutput("result", mOutput);

	QObject::connect(mOutputSize.get(), SIGNAL(valueChanged()), this, SLOT(updateOutputSize()));

}

cedar::proc::steps::PadAndTranslate::~PadAndTranslate()
{
}

void cedar::proc::steps::PadAndTranslate::compute(const cedar::proc::Arguments&)
{
	cv::Mat outputMat = cv::Mat::zeros(mOutputSize->getValue(), 1, CV_32F);
	cv::Mat& output = mOutput->getData();
	cedar::aux::ConstDataPtr inputData = this->getInputSlot("input")->getData();
	cedar::aux::ConstDataPtr inputTranslation = this->getInputSlot("translation")->getData();
	if(boost::dynamic_pointer_cast<const cedar::aux::MatData>(inputData)&&boost::dynamic_pointer_cast<const cedar::aux::MatData>(inputTranslation))
	{
		cv::Mat inputMat = inputData->getData<cv::Mat>().clone();
		cv::Mat translationMat = inputTranslation->getData<cv::Mat>().clone();
		int inputRows = inputMat.rows;


		if(translationMat.rows ==1 && translationMat.cols==1)
		{
			int translation = boost::numeric_cast<int>(translationMat.at<float>(0,0));
				for(int i= 0;i<inputRows;i++)
				{
					int translatedPos = i+translation-(inputRows/2);

					if(translatedPos > mOutputSize->getValue()||translatedPos <0)
					{
                        //Todo: Make the cyclic Boundaries a Parameter Option
						translatedPos = (translatedPos+mOutputSize->getValue())%mOutputSize->getValue();
					}
					if(translatedPos>=0 && translatedPos<mOutputSize->getValue())
					{
					outputMat.at<float>(translatedPos,0) = inputMat.at<float>(i,0);
					}
				}
			output = outputMat;
		}
	}
}


void cedar::proc::steps::PadAndTranslate::updateOutputSize()
{
	int new_size = mOutputSize->getValue();

	mOutput->setData(cv::Mat::zeros(new_size,1,CV_32F));
}


void cedar::proc::steps::PadAndTranslate::reset()
{
	mOutput->setData(cv::Mat::zeros(mOutputSize->getValue(),1,CV_32F));
}

cedar::proc::DataSlot::VALIDITY cedar::proc::steps::PadAndTranslate::determineInputValidity(cedar::proc::ConstDataSlotPtr slot,cedar::aux::ConstDataPtr data) const
{
	if (cedar::aux::ConstMatDataPtr input = boost::dynamic_pointer_cast<const cedar::aux::MatData>(data))
	{
		if (input && input->getDimensionality() == 1 && slot->getName()=="input" &&( input->getData().rows < mOutputSize->getValue() ))
		{
			return cedar::proc::DataSlot::VALIDITY_VALID;
		}
		if (input && input->getDimensionality() == 0 && slot->getName()=="translation")
		{
			return cedar::proc::DataSlot::VALIDITY_VALID;
		}
	}
	return cedar::proc::DataSlot::VALIDITY_ERROR;
}
//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------
