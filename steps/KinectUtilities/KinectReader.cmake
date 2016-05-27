DECLARE_STEP(cedar::proc::steps::KinectReader
             MOC
             CATEGORY         "Kinect Utilities"
             DESCRIPTION      "A source that reads out images and depth images, as well as a RGBA point cloud from a Kinect camera."
             MAINTAINER       "Nico Kuerschner"
             DEPRECATED_NAME  KinectUtilities::KinectReader
             REQUIRES_DATA_STRUCTURES cedar::aux::RGBAPointCloudData pcl::AsyncGrabber
             REQUIRES_LIBRARY "pcl"
            )
