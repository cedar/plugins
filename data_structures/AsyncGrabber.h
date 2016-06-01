/*
  This file is part of the action parsing project.

  Copyright (C) 2013 David Lobato <dav.lobato@gmail.com>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PCL_ASYNC_GRABBER_H
#define PCL_ASYNC_GRABBER_H

// FORWARD DECLARATIONS
#include "data_structures/AsyncGrabber.fwd.h"

#include <boost/scoped_ptr.hpp>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/io/openni_camera/openni_image.h>
#include <pcl/io/openni_camera/openni_depth_image.h>

namespace pcl
{
class AsyncGrabber
{
protected:
  typedef pcl::PointCloud<pcl::PointXYZRGBA> PointCloud;
  typedef typename PointCloud::Ptr PointCloudPtr;
  typedef typename PointCloud::ConstPtr PointCloudConstPtr;

public:
  AsyncGrabber(const std::string device_id);
  ~AsyncGrabber();

  void start();
  void stop();
  bool isRunning() const;

  PointCloudConstPtr getLatestCloud(); //if already read, returned cloud is null
  boost::shared_ptr<openni_wrapper::Image> getLatestImage();//idem
  boost::shared_ptr<openni_wrapper::DepthImage> getLatestDepthImage();//idem

  float getCloudFps() const { return cloud_fps_; }
  float getImageFps() const { return image_fps_; }

private:
  void cloud_cb_(const PointCloudConstPtr& cloud);
  void image_cb_(const boost::shared_ptr<openni_wrapper::Image>& image);
  void depth_image_cb_(const boost::shared_ptr<openni_wrapper::DepthImage>& depthImage);
  // add from 2015 : depth image grabb

  std::string device_id_;
  float cloud_fps_;
  float image_fps_;
  struct PImpl;
  boost::scoped_ptr<PImpl> mImpl;

  boost::signals2::scoped_connection mc1;
  boost::signals2::scoped_connection mc2;
  boost::signals2::scoped_connection mc3;
};
}//namespace pcl

#endif //PCL_ASYNC_GRABBER_H
