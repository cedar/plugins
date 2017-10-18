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

    File:        EgoToBirdView.h

    Maintainer:  Tutorial Writer Person
    Email:       cedar@ini.rub.de
    Date:        2011 12 09

    Description:

    Credits:

======================================================================================================================*/

// CEDAR INCLUDES
#include "steps/utilities/EgoToBirdView.h"
#include <cedar/processing/ExternalData.h> // getInputSlot() returns ExternalData
#include <cedar/auxiliaries/MatData.h> // this is the class MatData, used internally in this step

// SYSTEM INCLUDES

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------
cedar::proc::steps::EgoToBirdView::EgoToBirdView()
: // <- the colon starts the member initialization list
mOutputSize(new cedar::aux::UIntVectorParameter(this,"OutputSize",2,100)),
mMeterToField(new cedar::aux::DoubleParameter(this,"1 meter in field",1)),
mAgentPosInFieldX(new cedar::aux::DoubleParameter(this,"Agent Pos X in field",mOutputSize->getValue().at(0)/2.0)),
mAgentPosInFieldY(new cedar::aux::DoubleParameter(this,"Agent Pos Y in field",mOutputSize->getValue().at(1)/2.0)),
mOutput(new cedar::aux::MatData(cv::Mat::zeros(mOutputSize->getValue().at(0), mOutputSize->getValue().at(1), CV_32F)))
{
	/* Declare both inputs; the "true" means that the inputs are mandatory, i.e.,
     the step will not run unless both of the inputs are connected to data.
	 */
	this->declareInput("degree_distance", true);
	// Declare the output and set it to the output matrix defined above.
	this->declareOutput("birdView", mOutput);

	QObject::connect(mOutputSize.get(), SIGNAL(valueChanged()), this, SLOT(updateOutputSize()));
}
//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------
void cedar::proc::steps::EgoToBirdView::compute(const cedar::proc::Arguments&)
{
	// Using data like this is more convenient.
//	cv::Mat& outPutMat = mOutput->getData();

	cv::Mat outPutMat = cv::Mat::zeros(mOutputSize->getValue().at(0),mOutputSize->getValue().at(1),CV_32F);
	// Get a pointer to the first input.
	cedar::aux::ConstDataPtr inputDegreeDist = this->getInputSlot("degree_distance")->getData();

//	std::cout<<"EgoToBirdViewCompute!"<<std::endl;

	if(boost::dynamic_pointer_cast<const cedar::aux::MatData>(inputDegreeDist))
	{
		cv::Mat degreeDist = inputDegreeDist->getData<cv::Mat>().clone();

		double maxDegrees = cedar::aux::math::get1DMatrixSize(degreeDist)/2.0;
		std::vector<int> blobPositions = retrieveBlobPositions(degreeDist);



		for(unsigned int i=0; i< blobPositions.size();i++)
		{
			int blobPos = blobPositions.at(i);
			double alphaDegree = 0;
//			if(blobPos > maxDegrees)
//			{
				alphaDegree = blobPos - maxDegrees;
//			}
//			else
//			{
//				alphaDegree = maxDegrees - blobPos;
//				aplhaDegree = alphaDegree*-1;
//			}

			float hypotenuse = degreeDist.at<float>(blobPos,0);
//			std::cout<<"Max Degrees: "<< maxDegrees << " Blobpos: "<< blobPos<<std::endl;
//			std::cout<<"Blob"<<i<<": Degree = "<<alphaDegree<<" Hypotenuse: "<< hypotenuse<<std::endl;

			float xPos = sin(alphaDegree*pi/180) * hypotenuse;
			float yPos = cos(alphaDegree*pi/180) * hypotenuse;

			double outPutCenterX = mAgentPosInFieldX->getValue();
			double outPutCenterY = mAgentPosInFieldY->getValue();



//			std::cout<<" CenterX = "<< outPutCenterX<< " Center Y= "<< outPutCenterY << std::endl;


			float xField = xPos *mMeterToField->getValue();
			float yField = yPos *mMeterToField->getValue();
//			std::cout<<"Xpos in fieldPos: "<< xField<< ". After Rounding: "<< (int) round(xField) << ".  Ypos in fieldPos: " << yField<< ". After Rounding: "<< (int) round(yField) <<std::endl;

			int newPosX = round(outPutCenterX + (int) (round(xField)));
			int newPosY= round(outPutCenterY - (int) (round(yField)));

//			std::cout<<"NewXpos : "<< newPosX<< "NewYpos: " << newPosY<<std::endl;
//			std::cout<<" "<<std::endl;

			if(newPosY >=0 && newPosY < outPutMat.rows && newPosX >=0 && newPosX < outPutMat.cols)
			{
			  outPutMat.at<float>(newPosY,newPosX) = 1;
			}
			else
			{
			  std::string errorString = "The transformed peak positions X: " +boost::lexical_cast<std::string>(newPosX) +" and Y: " +boost::lexical_cast<std::string>(newPosY) +" exceed the birds view field limits. Change some Parameters.";
			  cedar::aux::LogSingleton::getInstance()->warning
			      (
			        errorString,
			        CEDAR_CURRENT_FUNCTION_NAME
			      );
			}

		}

		mOutput->setData(outPutMat);

	}
}

std::vector<int> cedar::proc::steps::EgoToBirdView::retrieveBlobPositions(cv::Mat inputMat)
{
	std::vector<int> blobPosVector;

	bool isPeak = false;
	int lastStart = 0;
	for(int i=0;i<inputMat.rows;i++)
	{
		float curValue = inputMat.at<float>(i,0);

		if(curValue>0 && !isPeak)
		{
			isPeak = true;
			lastStart = i;
		}

		if((curValue==0 && isPeak) || ((i==inputMat.rows -1) && isPeak) )
		{
			isPeak = false;
			int peakCenter = (lastStart + i-1)/2;
			blobPosVector.push_back(peakCenter);
		}
	}

	return blobPosVector;
}

void cedar::proc::steps::EgoToBirdView::updateOutputSize()
{
	int new_sizeX = mOutputSize->getValue().at(0);
	int new_sizeY = mOutputSize->getValue().at(1);
	mOutput->setData(cv::Mat::zeros(new_sizeX,new_sizeY,CV_32F));
}

void cedar::proc::steps::EgoToBirdView::reset()
{
	mOutput->setData(cv::Mat::zeros(mOutputSize->getValue().at(0),mOutputSize->getValue().at(1),CV_32F));
}

cedar::proc::DataSlot::VALIDITY cedar::proc::steps::EgoToBirdView::determineInputValidity(cedar::proc::ConstDataSlotPtr,cedar::aux::ConstDataPtr data) const
{
	if (cedar::aux::ConstMatDataPtr input = boost::dynamic_pointer_cast<const cedar::aux::MatData>(data))
	{
		if (input && input->getDimensionality() == 1)
		{
			return cedar::proc::DataSlot::VALIDITY_VALID;
		}
	}
	return cedar::proc::DataSlot::VALIDITY_ERROR;
}
