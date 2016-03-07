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

    File:        PatternMemory.cpp

    Maintainer:  Oliver Lomp
    Email:       oliver.lomp@ini.ruhr-uni-bochum.de
    Date:        2012 01 26

    Description:

    Credits:

======================================================================================================================*/

// LOCAL INCLUDES
#include "steps/dynamics/PatternMemory.h"

// CEDAR INCLUDES
#include <cedar/processing/StepTime.h>
#include <cedar/auxiliaries/math/tools.h>
#include <cedar/auxiliaries/math/functions.h>
#include <cedar/auxiliaries/math/constants.h>
#include <cedar/auxiliaries/Log.h>
#include <cedar/auxiliaries/assert.h>
#include <cedar/auxiliaries/exceptions.h>

// SYSTEM INCLUDES
#include <QFileDialog>
#include <vector>

//----------------------------------------------------------------------------------------------------------------------
// static members
//----------------------------------------------------------------------------------------------------------------------

cedar::aux::EnumType<cedar::dyn::steps::PatternMemory::WeightInitialization>
  cedar::dyn::steps::PatternMemory::WeightInitialization::mType("cedar::dyn::steps::PatternMemory::WeightInitialization::");

#ifndef MSVC
const cedar::dyn::steps::PatternMemory::WeightInitialization::Id cedar::dyn::steps::PatternMemory::WeightInitialization::Legacy;
const cedar::dyn::steps::PatternMemory::WeightInitialization::Id cedar::dyn::steps::PatternMemory::WeightInitialization::Gaussians;
const cedar::dyn::steps::PatternMemory::WeightInitialization::Id cedar::dyn::steps::PatternMemory::WeightInitialization::RandomNormal;
#endif

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::dyn::steps::PatternMemory::PatternMemory()
:
mWeights(new cedar::aux::MatData(cv::Mat::zeros(10, 10, CV_32F))),
_mNumLabels(new cedar::aux::UIntParameter(this, "numLabels", 10, 1, 10000)),
_mLearningFactor(new cedar::aux::DoubleParameter(this, "learningFactor", 0.8, 0.0, 100000.0)),
_mDisableDecay(new cedar::aux::BoolParameter(this, "disable learning decay", false)),
_mWeightInitializationMethod(new cedar::aux::EnumParameter(this, "weight initialization", cedar::dyn::steps::PatternMemory::WeightInitialization::typePtr(), cedar::dyn::steps::PatternMemory::WeightInitialization::Legacy))
{
  this->declareInput("shifted_input");
  this->declareInput("labels");
  this->declareInput("learning modulation", false);

  auto weight_slot = this->declareOutput("weights", this->mWeights);
  weight_slot->setSerializable(true);

  this->registerFunction("learn current pattern", boost::bind(&cedar::dyn::steps::PatternMemory::learnCurrentPattern, this));
//  this->registerFunction("save weight matrix", boost::bind(&cedar::dyn::steps::PatternMemory::openSaveWeightMatrixDialog, this));
  this->registerFunction("load weight matrix (old format)", boost::bind(&cedar::dyn::steps::PatternMemory::openLoadWeightMatrixDialog, this));
  this->registerFunction("reset weight matrix", boost::bind(&cedar::dyn::steps::PatternMemory::initializeWeights, this));

  this->initializeWeights();

  QObject::connect(this->_mNumLabels.get(), SIGNAL(valueChanged()), this, SLOT(updateWeightMatrixSize()));
}

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------

void cedar::dyn::steps::PatternMemory::openSaveWeightMatrixDialog()
{
  QString path = QFileDialog::getSaveFileName
                 (
                   NULL,
                   "Chose a file to save to" // caption
                 );
  if (!path.isEmpty())
  {
    this->saveWeightMatrix(path.toStdString());
  }
}

void cedar::dyn::steps::PatternMemory::openLoadWeightMatrixDialog()
{
  QString path = QFileDialog::getOpenFileName
                 (
                   NULL,
                   "Chose a file to load weights from" // caption
                 );
  if (!path.isEmpty())
  {
    this->loadWeightMatrix(path.toStdString());
  }
}

void cedar::dyn::steps::PatternMemory::saveWeightMatrix(const std::string& path) const
{
  cv::FileStorage storage(path, cv::FileStorage::WRITE);
  storage << "weights" << this->mWeights->getData();
}

void cedar::dyn::steps::PatternMemory::loadWeightMatrix(const std::string& path)
{
  cv::FileStorage storage(path, cv::FileStorage::READ);
  cv::Mat new_weights;
  storage["weights"] >> new_weights;

  if (new_weights.size != this->mWeights->getData().size)
  {
    std::string weights_size, new_weights_size;

    for (int d = 0; d < this->mWeights->getData().dims; ++d)
    {
      if (d > 0)
      {
        weights_size += "x";
      }
      weights_size += cedar::aux::toString(this->mWeights->getData().size[d]);
    }

    for (int d = 0; d < new_weights.dims; ++d)
    {
      if (d > 0)
      {
        new_weights_size += "x";
      }
      new_weights_size += cedar::aux::toString(new_weights.size[d]);
    }

    cedar::aux::LogSingleton::getInstance()->error
    (
      "Could not load weights: weight matrix size (" + new_weights_size + ") doesn't fit the current size (" + weights_size + ").",
      "void cedar::dyn::steps::PatternMemory::loadWeightMatrix(const std::string&)"
    );
    return;
  }
  this->mWeights->setData(new_weights);
}

void cedar::dyn::steps::PatternMemory::learnCurrentPattern()
{
  //TODO this should use on trigger, but that is missing from the interface right now
  this->eulerStep(1.0 * cedar::unit::second);
}

void cedar::dyn::steps::PatternMemory::inputConnectionChanged(const std::string& inputName)
{
  if (inputName == "shifted_input")
  {
    this->mPattern = boost::dynamic_pointer_cast<cedar::aux::ConstMatData>(this->getInput(inputName));

    if (this->mPattern)
    {
      this->updateWeightMatrixSize();
    }
  }
  else if (inputName == "labels")
  {
    this->mLabels = boost::dynamic_pointer_cast<cedar::aux::ConstMatData>(this->getInput(inputName));
  }
}

cedar::proc::DataSlot::VALIDITY cedar::dyn::steps::PatternMemory::determineInputValidity
                                (
                                  cedar::proc::ConstDataSlotPtr slot,
                                  cedar::aux::ConstDataPtr data
                                )
                                const
{
  if (slot == this->getInputSlot("shifted_input"))
  {
    if (cedar::aux::ConstMatDataPtr mat_data = boost::dynamic_pointer_cast<const cedar::aux::MatData>(data))
    {
      const cv::Mat& mat = mat_data->getData();

      if (cedar::aux::math::getDimensionalityOf(mat) >= 1 && cedar::aux::math::getDimensionalityOf(mat) <= 3)
      {
        return cedar::proc::DataSlot::VALIDITY_VALID;
      }
      else
      {
        return cedar::proc::DataSlot::VALIDITY_ERROR;
      }
    }
  }
  else if (slot == this->getInputSlot("labels"))
  {
    if (cedar::aux::ConstMatDataPtr space_code = boost::dynamic_pointer_cast<cedar::aux::ConstMatData>(data))
    {
      const cv::Mat& mat = space_code->getData();

      if
      (
        cedar::aux::math::getDimensionalityOf(mat) == 1
        && cedar::aux::math::get1DMatrixSize(mat) == this->getNumLabels()
      )
      {
        return cedar::proc::DataSlot::VALIDITY_VALID;
      }
      else
      {
        return cedar::proc::DataSlot::VALIDITY_ERROR;
      }
    }
  }
  else if (slot == this->getInputSlot("learning modulation"))
  {
    if (cedar::aux::ConstMatDataPtr mat_data = boost::dynamic_pointer_cast<const cedar::aux::MatData>(data))
    {
      const cv::Mat& mat = mat_data->getData();

      if (cedar::aux::math::getDimensionalityOf(mat) == 0)
      {
        return cedar::proc::DataSlot::VALIDITY_VALID;
      }
    }
  }
  return cedar::proc::DataSlot::VALIDITY_ERROR;
}

unsigned int cedar::dyn::steps::PatternMemory::getDimensionality() const
{
  if (this->mPattern)
  {
    return cedar::aux::math::getDimensionalityOf(this->mPattern->getData());
  }
  else
  {
    return 1;
  }
}

int cedar::dyn::steps::PatternMemory::getPatternSize1D()
{
  if (this->mPattern)
  {
    return static_cast<int>(cedar::aux::math::get1DMatrixSize(this->mPattern->getData()));
  }
  else
  {
    return 10;
  }
}

void cedar::dyn::steps::PatternMemory::updateWeightMatrixSize()
{
  int num_labels = static_cast<int>(this->_mNumLabels->getValue());
  int dim = static_cast<int>(this->getDimensionality());

  CEDAR_DEBUG_ASSERT(dim >= 1 && dim <= 3);

  bool changed = false;
  switch (dim)
  {
    case 1:
    {
      cv::Mat& weights = this->mWeights->getData();
      if (cedar::aux::math::getDimensionalityOf(weights) != static_cast<unsigned int>(dim) || weights.cols != num_labels)
      {
        weights = cv::Mat::zeros(this->getPatternSize1D(), num_labels, CV_32F);
        changed = true;
      }
      break;
    }
    case 2:
    case 3:
    {
      std::vector<int> pattern_size;
      if (this->mPattern)
      {
        if (this->mPattern->getDimensionality() <= 2)
        {
          pattern_size.push_back(this->mPattern->getData().rows);
          pattern_size.push_back(this->mPattern->getData().cols);
        }
        else
        {
          for (int d = 0; d < static_cast<int>(this->mPattern->getDimensionality()); ++d)
          {
            pattern_size.push_back(this->mPattern->getData().size[d]);
          }
        }
      }
      else
      {
        pattern_size.resize(2);
        pattern_size[0] = pattern_size[1] = 10;
      }

      std::vector<int> sizes(pattern_size.size() + 1);
      sizes[0] = num_labels;

      for (size_t d = 0; d < pattern_size.size(); ++d)
      {
        sizes[d + 1] = pattern_size.at(d);

        if (this->mWeights->getDimensionality() < d || sizes.at(d) != this->mWeights->getData().size[d])
        {
          changed = true;
        }
      }

      if (changed)
      {
        this->mWeights->setData(cv::Mat(static_cast<int>(sizes.size()), &sizes.front(), CV_32F, 0.0));
      }
      break;
    }
  }

  if (changed)
  {
//    this->initializeWeights();

    this->emitOutputPropertiesChangedSignal("weights");
  }
}

void cedar::dyn::steps::PatternMemory::initializeWeights()
{
  switch (this->_mWeightInitializationMethod->getValue())
  {
    case WeightInitialization::Gaussians:
      this->initializeWeightsGaussians();
      break;

    case WeightInitialization::RandomNormal:
      this->initializeWeightsRandomNormal();
      break;

    default:
    case WeightInitialization::Legacy:
      this->initializeWeightsLegacy();
      break;
  }
}

void cedar::dyn::steps::PatternMemory::initializeWeightsRandomNormal()
{
  cv::Mat& weights = this->mWeights->getData();
  cv::theRNG().fill(weights, cv::RNG::NORMAL, cv::Scalar(0), cv::Scalar(1));
}

void cedar::dyn::steps::PatternMemory::initializeWeightsGaussians()
{
  cv::Mat& weights = this->mWeights->getData();

  switch (cedar::aux::math::getDimensionalityOf(weights))
  {
    case 1:
    {
      CEDAR_ASSERT(false && "This should not happen!");
      break;
    }

    case 2:
    {
      double pattern_size = static_cast<double>(weights.rows);
      double sigma = pattern_size / 10.0;
      double norm = 1.0 / (sigma * sqrt(2.0 * cedar::aux::math::pi));
      double center = 0.0;

      CEDAR_DEBUG_ASSERT(weights.type() == CV_32F);

      std::vector<unsigned int> sizes;
      sizes.push_back(static_cast<size_t>(weights.rows));
      std::vector<double> sigmas;
      sigmas.push_back(sigma);
      std::vector<double> centers;
      centers.push_back(center);
      cv::Mat gauss = cedar::aux::math::gaussMatrix(1, sizes, norm, sigmas, centers, true);
      for (int col = 0; col < weights.cols; ++col)
      {
        for (int row = 0; row < weights.rows; ++row)
        {
          weights.at<float>(row, col) = gauss.at<float>(row, 0);
        }
      }
      break;
    }

    case 3:
      this->initializeWeightsLegacy();
  }
}

void cedar::dyn::steps::PatternMemory::initializeWeightsLegacy()
{
  cv::Mat& weights = this->mWeights->getData();
  weights.setTo(cv::Scalar(0.0));

  switch (cedar::aux::math::getDimensionalityOf(weights))
  {
    case 1:
    {
      CEDAR_ASSERT(false && "This should not happen!");
      break;
    }

    case 2:
    {
      double pattern_size = static_cast<double>(weights.rows);
      double sigma = pattern_size / 10.0;
      double norm = 1.0 / (sigma * sqrt(2.0 * cedar::aux::math::pi));

      CEDAR_DEBUG_ASSERT(weights.type() == CV_32F);

      for (int row = 0; row < weights.rows; ++row)
      {
        double center = static_cast<double>(weights.rows) - (pattern_size - 1.0) / 2.0;
        weights.at<float>(row, 0) = norm * cedar::aux::math::gauss(center, sigma);
      }
      break;
    }

    case 3:
    {
      std::vector<unsigned int> sizes(2);
      sizes.at(0) = weights.size[1];
      sizes.at(1) = weights.size[2];
      std::vector<double> sigmas(2, static_cast<double>(sizes.at(0)) / 2.0);
      std::vector<double> centers(2, static_cast<double>(sizes.at(0)) / 2.0);
      cv::Mat gauss = cedar::aux::math::gaussMatrix
                      (
                        2,
                        sizes,
                        1.0,
                        sigmas,
                        centers,
                        false // cyclic
                      );

      std::vector<cv::Range> ranges(static_cast<size_t>(weights.dims), cv::Range::all());
      for (int d = 0; d < weights.size[0]; ++d)
      {
        ranges.at(0) = cv::Range(d, d + 1);
        cv::Mat slice(gauss.rows, gauss.cols, weights.type(), weights(&ranges.front()).data);
        gauss.copyTo(slice);
      }

      break;
    }

    default:
    {
      double pattern_size = static_cast<double>(weights.size[0]);
      double sigma = pattern_size / 10.0;
      double norm = 1.0 / (sigma * sqrt(2.0 * cedar::aux::math::pi));

      CEDAR_DEBUG_ASSERT(weights.type() == CV_32F);

      std::vector<int> index;
      index.resize(static_cast<unsigned int>(weights.dims), 0);

      for (index[0] = 0; index[0] < weights.size[0]; ++index[0])
      {
        double center = static_cast<double>(index[0]) - (pattern_size - 1.0) / 2.0;
        weights.at<float>(&index.front()) = norm * cedar::aux::math::gauss(center, sigma);
      }

      break;
    }
  }
}


void cedar::dyn::steps::PatternMemory::eulerStep(const cedar::unit::Time& time)
{
  double learning_modulation = 1.0;

  if (cedar::aux::ConstDataPtr data = this->getInput("learning modulation"))
  {
    if (cedar::aux::ConstMatDataPtr mat_data = boost::dynamic_pointer_cast<cedar::aux::ConstMatData>(data))
    {
      learning_modulation = cedar::aux::math::getMatrixEntry<double>(mat_data->getData(), 0, 0);
    }
  }

  // matlab: W_l=W_l+ (u_tp*u_tp>0) * 0.02*w_l_teach*(-W_l + input_shifted*theta_u_l2')*theta_u_l2*theta_u_l2';
  cv::Mat& weights = this->mWeights->getData();
  double learning_factor = this->getLearningFactor();
  const cv::Mat& shifted_input = this->mPattern->getData();
  const cv::Mat& label_output = this->mLabels->getData();

  double dt = time/(1.0 * cedar::unit::second);

  if (this->getDimensionality() == 1)
  {
    cv::Mat modulation = cv::Mat(weights.rows, weights.cols, weights.type(), learning_modulation);

    if (this->_mDisableDecay->getValue())
    {
      // weights = Mat(pattern size, num labels)
      int nrows = this->getPatternSize1D();
      int ncols = 1;

      CEDAR_DEBUG_ASSERT(label_output.cols == 1);

      auto expanded_labels = cv::repeat(label_output.t(), nrows, ncols);
      CEDAR_DEBUG_ASSERT(expanded_labels.rows == modulation.rows);
      CEDAR_DEBUG_ASSERT(expanded_labels.cols == modulation.cols);
      modulation = modulation.mul(expanded_labels);
    }

    // fast version for 1d
    weights
      += (modulation * dt * learning_factor).mul((-weights + shifted_input * label_output.t()))
         * label_output * label_output.t();
  }
  else
  {
    //!@todo This isn't equivalent to the 1d case

    // weights = labels x pattern size 0 x pattern size 1 x ...
    CEDAR_DEBUG_ASSERT(label_output.type() == CV_32F);

    cv::Mat modulated = weights * 0.0;
    cv::Mat tmp = shifted_input.clone();
    for (int label = 0; label < modulated.size[0]; ++label)
    {
      tmp = shifted_input * label_output.at<float>(label);
      memcpy(modulated.data + modulated.step[0] * label, tmp.data, tmp.total() * sizeof(float));
    }

    cv::Mat gate;

    if (this->_mDisableDecay->getValue())
    {
      gate = weights * 0.0;
      std::vector<cv::Range> ranges(static_cast<size_t>(weights.dims), cv::Range::all());
      for (int i = 0; i < gate.size[0]; ++i)
      {
        ranges.at(0) = cv::Range(i, i + 1);
        gate(&ranges.front()) = label_output.at<float>(i);
      }
    }

    cv::Mat d_weights = learning_factor * (-weights + modulated);

    if (this->_mDisableDecay->getValue())
    {
      d_weights = d_weights.mul(gate);
    }
    weights += learning_modulation * dt * d_weights;
  }
}
