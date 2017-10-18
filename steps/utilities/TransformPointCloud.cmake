DECLARE_STEP(cedar::proc::steps::TransformPointCloud
             MOC
             CATEGORY         "Image Processing"
             DESCRIPTION      "A step that performs geometric operations on a point cloud, yielding the transformed point cloud and camera position."
             MAINTAINER       "Nico Kuerschner"
             DEPRECATED_NAME  KinectUtilities::TransformPointCloud
             REQUIRES_DATA_STRUCTURES cedar::aux::RGBAPointCloudData
             REQUIRES_LIBRARY "pcl"
            )
