// LOCAL INCLUDES
#include "TransformPointCloud.h"

// PROJECT INCLUDES
#include <boost/math/constants/constants.hpp>
#include <pcl/common/transforms.h>  

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::proc::steps::TransformPointCloud::TransformPointCloud()
  :
  mpInputPointCloud(),
  mpOutputPointCloud(new cedar::aux::RGBAPointCloudData(cedar::aux::RGBAPointCloud())),
  mpCameraPosition(new cedar::aux::MatData(cv::Mat::zeros(3, 1, CV_64F))),
  mpXTranslation(new cedar::aux::DoubleParameter(this,"X Translation", 0.0, -2.0, 2.0)),
  mpYTranslation(new cedar::aux::DoubleParameter(this,"Y Translation", 0.0, -2.0, 2.0)),
  mpZTranslation(new cedar::aux::DoubleParameter(this,"Z Translation", 0.0, -2.0, 2.0)),
  mpRoll(new cedar::aux::DoubleParameter(this,"Roll",-boost::math::constants::pi<double>()/2., -3.1416, 3.1416)),
  mpPitch(new cedar::aux::DoubleParameter(this,"Pitch",0.0, -3.1416, 3.1416)),
  mpYaw(new cedar::aux::DoubleParameter(this,"Yaw", -boost::math::constants::pi<double>()/2., -3.1416, 3.1416))
{
  //declare
  this->declareInput("input pointcloud");
  this->declareOutput("transformed pointcloud", mpOutputPointCloud);
  this->declareOutput("camera position", mpCameraPosition);
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

void cedar::proc::steps::TransformPointCloud::compute(const cedar::proc::Arguments&)
{
  //cedar::aux::RGBAPointCloudPtr pc_copy(new cedar::aux::RGBAPointCloud(mpInputPointCloud->getData() )  );
  //TODO: Compute transformation matrix only when paramerer changed
  float x = static_cast<float>(mpXTranslation->getValue());
  float y = static_cast<float>(mpYTranslation->getValue());
  float z = static_cast<float>(mpZTranslation->getValue());
  float roll = static_cast<float>(mpRoll->getValue());
  float pitch = static_cast<float>(mpPitch->getValue());
  float yaw = static_cast<float>(mpYaw->getValue());

  Eigen::Affine3f transform;
  pcl::getTransformation (x, y, z, roll, pitch, yaw, transform);
  Eigen::Vector3f origin(0,0,0);
  Eigen::Vector3f camera_pos = transform * origin;

  mpCameraPosition->getData().at<double>(0,0) = camera_pos(0);
  mpCameraPosition->getData().at<double>(1,0) = camera_pos(1);
  mpCameraPosition->getData().at<double>(2,0) = camera_pos(2);

  if(mpInputPointCloud)
  {
    cedar::aux::RGBAPointCloudPtr pc_copy(this->mpInputPointCloud->getData().makeShared());
    cedar::aux::RGBAPointCloudPtr output_cloud(new cedar::aux::RGBAPointCloud);
    pcl::transformPointCloud(*pc_copy,*output_cloud,transform);
    mpOutputPointCloud->setData(*output_cloud);
  }
}

//// validity check
cedar::proc::DataSlot::VALIDITY cedar::proc::steps::TransformPointCloud::determineInputValidity(cedar::proc::ConstDataSlotPtr slot, cedar::aux::ConstDataPtr data) const
{
  if(slot->getName() == "input pointcloud")
  {
    cedar::aux::ConstRGBAPointCloudDataPtr _input = boost::dynamic_pointer_cast<cedar::aux::ConstRGBAPointCloudData>(data);
    if(_input)
    {
      return cedar::proc::DataSlot::VALIDITY_VALID;
    }
  }

  // else
  return cedar::proc::DataSlot::VALIDITY_ERROR;
}

void cedar::proc::steps::TransformPointCloud::inputConnectionChanged(const std::string& inputName)
{
  if (inputName == "input pointcloud")
  {
    this->mpInputPointCloud = boost::dynamic_pointer_cast<const cedar::aux::RGBAPointCloudData>(this->getInput(inputName));
  }
}
