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

#include <iostream>

#include "AsyncGrabber.h"
#include <boost/thread/mutex.hpp>
#include <pcl/io/openni_grabber.h>
#include <pcl/common/time.h>
// all the depth image are add during the summer 2015
namespace pcl
{

struct AsyncGrabber::PImpl
{
  boost::shared_ptr<pcl::Grabber> grabber_;
  boost::mutex cloud_mutex_;
  PointCloudConstPtr cloud_;
  boost::mutex image_mutex_;
  boost::mutex depth_image_mutex_;
  boost::shared_ptr<openni_wrapper::Image> image_;
  boost::shared_ptr<openni_wrapper::DepthImage> depth_image_;
};

AsyncGrabber::AsyncGrabber(const std::string device_id)
  :
device_id_(device_id),
mImpl( new PImpl())
{}

AsyncGrabber::~AsyncGrabber()
{
  stop();
}

void AsyncGrabber::start()
{    
  mImpl->grabber_.reset(new pcl::OpenNIGrabber (device_id_));

  boost::function<void (const PointCloudConstPtr&)> f1 = boost::bind(&AsyncGrabber::cloud_cb_, this, _1);
  boost::function<void (const boost::shared_ptr<openni_wrapper::Image>&)> f2 = boost::bind (&AsyncGrabber::image_cb_, this, _1);
  boost::function<void (const boost::shared_ptr<openni_wrapper::DepthImage>&)> f3 = boost::bind (&AsyncGrabber::depth_image_cb_, this, _1);

  boost::signals2::connection c1 = mImpl->grabber_->registerCallback (f1);
  boost::signals2::connection c2 = mImpl->grabber_->registerCallback (f2);
  boost::signals2::connection c3 = mImpl->grabber_->registerCallback (f3);

  mImpl->grabber_->start();  
}

void AsyncGrabber::stop()
{
  unsigned int count=0;

  while( (mImpl->grabber_ && mImpl->grabber_->isRunning()) && ++count < 10)
  {
    mImpl->grabber_->stop();
  }
}

bool AsyncGrabber::isRunning() const
{
  return (mImpl->grabber_ && mImpl->grabber_->isRunning());
}

void AsyncGrabber::cloud_cb_(const PointCloudConstPtr& cloud)
{
  static double duration = 0;
  static unsigned count = 0;
  static double prev_time = pcl::getTime();

  double now = pcl::getTime();

  if (++count == 10)
  {
    cloud_fps_ = (double)count/duration;
    count = 0;
    duration = 0.0;
  }
  else
  {
    duration += now-prev_time;
    prev_time = now;
  }

  boost::mutex::scoped_lock lock (mImpl->cloud_mutex_);
  mImpl->cloud_ = cloud;
}

void AsyncGrabber::image_cb_(const boost::shared_ptr<openni_wrapper::Image>& image)
{
  static double duration = 0;
  static unsigned count = 0;
  static double prev_time = pcl::getTime();
  double now = pcl::getTime();

  if (++count == 10)
  {
    image_fps_ = (double)count/duration;
    count = 0;
    duration = 0.0;
  }
  else
  {
    duration += now-prev_time;
    prev_time = now;
  }

  boost::mutex::scoped_lock lock (mImpl->image_mutex_);
  mImpl->image_ = image;
}

void AsyncGrabber::depth_image_cb_(const boost::shared_ptr<openni_wrapper::DepthImage>& depthImage)
{
  static double duration = 0;
  static unsigned count = 0;
  static double prev_time = pcl::getTime();
  double now = pcl::getTime();

  if (++count == 10)
  {
    image_fps_ = (double)count/duration;
    count = 0;
    duration = 0.0;
  }
  else
  {
    duration += now-prev_time;
    prev_time = now;
  }

  boost::mutex::scoped_lock lock(mImpl->depth_image_mutex_);
  mImpl->depth_image_ = depthImage;
}

AsyncGrabber::PointCloudConstPtr AsyncGrabber::getLatestCloud()
{
  boost::mutex::scoped_lock lock (mImpl->cloud_mutex_);
  PointCloudConstPtr tmp_cloud;
  tmp_cloud.swap (mImpl->cloud_);

  return tmp_cloud;
}

boost::shared_ptr<openni_wrapper::Image> AsyncGrabber::getLatestImage()
{
  boost::mutex::scoped_lock lock (mImpl->image_mutex_);
  boost::shared_ptr<openni_wrapper::Image> tmp_image;
  tmp_image.swap(mImpl->image_);

  return tmp_image;
}


boost::shared_ptr<openni_wrapper::DepthImage> AsyncGrabber::getLatestDepthImage()
{
  boost::mutex::scoped_lock lock (mImpl->depth_image_mutex_);
  boost::shared_ptr<openni_wrapper::DepthImage> tmp_depthImage;
  tmp_depthImage.swap(mImpl->depth_image_);
  return tmp_depthImage;
}
}//namespace pcl
