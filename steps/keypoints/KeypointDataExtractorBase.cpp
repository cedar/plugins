/*======================================================================================================================

    Copyright 2011, 2012, 2013, 2014 Institut fuer Neuroinformatik, Ruhr-Universitaet Bochum, Germany
 
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

    File:        KeyPointDataExtractorBase.cpp

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2014 12 09

    Description: Source file for the class keypoints::KeyPointDataExtractorBase.

    Credits:

======================================================================================================================*/

// CEDAR CONFIGURATION
#include "cedar/configuration.h"

// CLASS HEADER
#include "steps/keypoints/KeypointDataExtractorBase.h"

// CEDAR INCLUDES
#include <cedar/processing/Arguments.h>

// SYSTEM INCLUDES

//----------------------------------------------------------------------------------------------------------------------
// hidden class
//----------------------------------------------------------------------------------------------------------------------

class CopyKeypointProperties : public cedar::proc::OutputRelation<cedar::aux::KeypointData>
{
public:
  CopyKeypointProperties
  (
    const boost::function<cv::Mat(const vislab::keypoints::KPData&)>& extractor,
    cedar::proc::DataSlotPtr target
  )
  :
  cedar::proc::OutputRelation<cedar::aux::KeypointData>(target),
  mExtractor(extractor)
  {
  }

protected:
  bool assign(cedar::aux::ConstKeypointDataPtr source, cedar::aux::DataPtr target)
  {
    auto mat_target = boost::dynamic_pointer_cast<cedar::aux::MatData>(target);
    CEDAR_DEBUG_ASSERT(mat_target);

    bool changed = false;
    if (!source)
    {
      mat_target->setData(cv::Mat());
      changed = !mat_target->getData().empty();
    }
    else
    {
      const cv::Mat& target_mat = mat_target->getData();
      const auto& kp_data = source->getData();

      cv::Mat init;
      if (source && !kp_data.empty())
      {
        auto source_mat = mExtractor(kp_data.at(0));
        int type = source_mat.type();

        if (source->getData().size() > 1)
        {
          int size[] = {static_cast<int>(kp_data.size()), source_mat.rows, source_mat.cols};
          init = cv::Mat(3, size, type, cv::Scalar(0));
        }
        else
        {
          init = cv::Mat(source_mat.dims, source_mat.size, type, cv::Scalar(0));
        }
      }

      if (target_mat.type() != init.type() || target_mat.size != init.size)
      {
        changed = true;
      }

      mat_target->setData(init);
    }

    return changed;
  }

private:
  boost::function<cv::Mat(const vislab::keypoints::KPData&)> mExtractor;
};
CEDAR_GENERATE_POINTER_TYPES(CopyKeypointProperties);

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

void cedar::proc::steps::KeypointDataExtractorBase::construct
(
  const std::string& outputName,
  const boost::function<cv::Mat(const vislab::keypoints::KPData&)>& extractor,
  const boost::function<cv::Mat(const vislab::keypoints::KPData&)>& sizeExtractor
)
{
  mOutputName = outputName;
  mOutput = cedar::aux::MatDataPtr(new cedar::aux::MatData(cv::Mat()));
  mExtractor = extractor;

//  cedar::proc::typecheck::DerivedFrom<keypoints::KeyPointData> input_check;
  auto input_check = [](cedar::proc::ConstDataSlotPtr, cedar::aux::ConstDataPtr data, std::string& info)
  {
    auto kp_data = boost::dynamic_pointer_cast<cedar::aux::ConstKeypointData>(data);

    if (!kp_data)
    {
      info = "This step expects keypoint data as input.";
      return cedar::proc::DataSlot::VALIDITY_ERROR;
    }

    if (kp_data->getData().empty())
    {
      info = "The given keypoint data is empty.";
      return cedar::proc::DataSlot::VALIDITY_ERROR;
    }

    return cedar::proc::DataSlot::VALIDITY_VALID;
  };

  this->mInput.getSlot()->setCheck(input_check);

  auto output_slot = this->declareOutput(outputName, this->mOutput);

  this->mInput.addOutputRelation
  (
    CopyKeypointPropertiesPtr
    (
      new CopyKeypointProperties
      (
        sizeExtractor,
        output_slot
      )
    )
  );
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

void cedar::proc::steps::KeypointDataExtractorBase::inputConnectionChanged(const std::string&)
{
  cv::Mat old_output = this->mOutput->getData().clone();

  this->callComputeWithoutTriggering();

  if (old_output.size != this->mOutput->getData().size)
  {
    this->emitOutputPropertiesChangedSignal(this->mOutputName);
  }
}

void cedar::proc::steps::KeypointDataExtractorBase::compute(const cedar::proc::Arguments& args)
{
  this->compute(args, this->mInput.getData());
}

void cedar::proc::steps::KeypointDataExtractorBase::compute
(
  const cedar::proc::Arguments&,
  const cedar::aux::KeypointData::StoredType& data
)
{
  if (data.empty())
  {
    return;
  }

  auto mat_zero = mExtractor(data[0]);

  if (mat_zero.empty())
  {
    return;
  }

  if (data.size() == 1)
  {
    this->mOutput->setData(mat_zero.clone());
  }
  else
  {
    cv::Mat& output = this->mOutput->getData();

    // make sure the output has the right size
    int sizes[3];
    sizes[0] = data.size();
    sizes[1] = static_cast<int>(mat_zero.rows);
    sizes[2] = static_cast<int>(mat_zero.cols);
    output.create(3, sizes, mat_zero.type());

    for (size_t d = 0; d < data.size(); ++d)
    {
      cv::Mat mat = mExtractor(data[d]);
      cv::Mat resized;

      // resize all matrices on the larger scales
      cv::resize(mat, resized, mat_zero.size());

      CEDAR_DEBUG_ASSERT(resized.rows == static_cast<int>(mat_zero.rows));
      CEDAR_DEBUG_ASSERT(resized.cols == static_cast<int>(mat_zero.cols));

      cv::Range ranges[] = {cv::Range(d, d+1), cv::Range::all(), cv::Range::all()};

      int sizes[] = {1, resized.rows, resized.cols};
      cv::Mat mat3d(3, sizes, resized.type(), resized.data);

      mat3d.copyTo(output(ranges));
    }
  }
}
