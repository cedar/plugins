DECLARE_STEP(cedar::proc::steps::KinectReader
             MOC
             CATEGORY         "Sources"
             DESCRIPTION      "A source that reads a Kinect camera, yielding images and depth images, as well as a RGBA point cloud."
             MAINTAINER       "Nico Kuerschner"
             DEPRECATED_NAME  KinectUtilities::KinectReader
             REQUIRES_DATA_STRUCTURES cedar::aux::RGBAPointCloudData pcl::AsyncGrabber
             REQUIRES_LIBRARIES "pcl"
            )
