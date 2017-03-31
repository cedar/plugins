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

    File:        ComponentMultiply.cpp

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2011 12 22

    Description:

    Credits:

======================================================================================================================*/

// LOCAL INCLUDES
#include "steps/keypoints/KeypointLinesAndEdges.h"

// PROJECT INCLUDES

// SYSTEM INCLUDES
#include <keypoints/linesedges.h>

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::proc::steps::KeypointLinesAndEdges::KeypointLinesAndEdges()
:
cedar::proc::steps::KeypointDataExtractorBase
(
  "lines and edges",
  [](const vislab::keypoints::KPData& data)
  {
    return data.Ch;
  },
  [](const vislab::keypoints::KPData& data)
  {
    return data.KD;
  }
)
{
  this->mLinesEdgesLocations = cedar::aux::MatDataPtr(new cedar::aux::MatData());
  this->declareOutput("line and edge locations", this->mLinesEdgesLocations);

  this->mLinesEdgesOrientations = cedar::aux::MatDataPtr(new cedar::aux::MatData());
  this->declareOutput("line and edge orientations", this->mLinesEdgesOrientations);
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

void cedar::proc::steps::KeypointLinesAndEdges::compute(const cedar::proc::Arguments& args)
{
  // this can happen during revalidation
  //!@todo Fix this!
  if (!this->mInput.get())
    return;

  cedar::aux::KeypointData::StoredType copy = mInput.getData();

  // we need a copy here because this call modifies the data
  vislab::keypoints::linesedges_sc(copy);

  // pass the copy to the standard processing
  this->KeypointDataExtractorBase::compute(args, copy);

  cv::Mat& le_loc = this->mLinesEdgesLocations->getData();
  cv::Mat& le_ori = this->mLinesEdgesOrientations->getData();

  CEDAR_DEBUG_ASSERT(copy.size() > 0);
  std::vector<int> sizes(3);
  sizes[0] = copy.size();
  sizes[1] = copy[0].LEloc.rows;
  sizes[2] = copy[0].LEloc.cols;

  le_loc.create(3, &sizes.front(), CV_64F);
  le_ori.create(3, &sizes.front(), CV_64F);

  auto interpolation_method = cv::INTER_NEAREST;

  for (int i = 0; i < static_cast<int>(copy.size()); ++i)
  {
    const cv::Mat& src_loc_mat = copy[i].LEloc;
    CEDAR_DEBUG_ASSERT(src_loc_mat.type() == CV_64F);
    cv::Mat src_loc_scaled;
    cv::resize(src_loc_mat, src_loc_scaled, cv::Size(sizes[2], sizes[1]), 0.0, 0.0, interpolation_method);
    CEDAR_DEBUG_ASSERT(src_loc_scaled.isContinuous());
    CEDAR_DEBUG_ASSERT(le_loc.isContinuous());
    auto* src_loc = src_loc_scaled.ptr(0);
    auto* dst_loc = le_loc.ptr(i);
    memcpy(dst_loc, src_loc, src_loc_scaled.total() * src_loc_scaled.elemSize());

    const cv::Mat& src_ori_mat = copy[i].LEori;
    CEDAR_DEBUG_ASSERT(src_ori_mat.type() == CV_64F);
    cv::Mat src_ori_scaled;
    cv::resize(src_ori_mat, src_ori_scaled, cv::Size(sizes[2], sizes[1]), 0.0, 0.0, interpolation_method);
    CEDAR_DEBUG_ASSERT(src_ori_scaled.isContinuous());
    CEDAR_DEBUG_ASSERT(le_ori.isContinuous());
    auto* src_ori = src_ori_scaled.ptr(0);
    auto* dst_ori = le_ori.ptr(i);
    memcpy(dst_ori, src_ori, src_ori_scaled.total() * src_ori_scaled.elemSize());
  }
}
