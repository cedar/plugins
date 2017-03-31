/*======================================================================================================================

    Copyright 2011, 2012 Institut fuer Neuroinformatik, Ruhr-Universitaet Bochum, Germany
 
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

    File:        MotionKernel.h

    Maintainer:  Michael Berger
    Email:       michael.berger@ini.rub.de
    Date:        2012 09 20

    Description:

    Credits:

======================================================================================================================*/

#ifndef CEDAR_AUX_KERNEL_MOTION_KERNEL_H
#define CEDAR_AUX_KERNEL_MOTION_KERNEL_H

// LOCAL INCLUDES
#include "kernels/MotionKernel.fwd.h"

// CEDAR INCLUDES
#include "cedar/auxiliaries/kernel/Separable.h"
#include "cedar/auxiliaries/DoubleVectorParameter.h"
#include "cedar/auxiliaries/DoubleParameter.h"

// SYSTEM INCLUDES


/*!@brief Motion kernel class.
 * This is a general version of the gauss kernel class.
 * - A global inhibition term can be added to each dimension separately.
 * - There is an amplitude parameter which scales the gaussian local excitation of each dimension but not the global inhibition term of this dimension.
 *
 * The benefit of this class in contrast to the usual Gauss kernel is that it is possible to introduce global excitation along only one dimension.
 * This is, e.g., important for motion detection where you want to detect a single direction selective motion percept (global inhibition for motion direction)
 * but for multiple objects (no global inhibition in the space dimension)
 */
class cedar::aux::kernel::MotionKernel : public cedar::aux::kernel::Separable
{
  //--------------------------------------------------------------------------------------------------------------------
  // macros
  //--------------------------------------------------------------------------------------------------------------------
  Q_OBJECT
  //--------------------------------------------------------------------------------------------------------------------
  // constructors and destructor
  //--------------------------------------------------------------------------------------------------------------------
public:
  //!@brief The standard constructor.
  MotionKernel
  (
    unsigned int dimensionality = 2,
    double amplitude = 1.0,
    double sigmas = 3.0,
    double shifts = 0.0,
    double inh = 0.0,
    double limit = 5.0
  );

  //!@brief Constructor to create an instance of Gauss directly from a set of parameters (without configuration file).
  MotionKernel(
         std::vector<double> amplitude,
         std::vector<double> sigmas,
         std::vector<double> shifts,
         std::vector<double> inhs,
         double limit,
         unsigned int dimensionality
       );
  //!@brief Destructor
  virtual ~MotionKernel();
  //--------------------------------------------------------------------------------------------------------------------
  // public methods
  //--------------------------------------------------------------------------------------------------------------------
public:
  //!\brief get the sigma of a given dimension
  double getSigma(unsigned int dimension) const;

  /*!\brief set the sigma of a chosen dimension and Gaussian
   * \param dimension the dimension
   * \param sigma the new \f$\sigma\f$
   */
  void setSigma(unsigned int dimension, double sigma);

  //!@brief get the shift of the kernel for a given dimension
  double getShift(unsigned int dimension) const;

  //!@brief set the shift for a given dimension
  void setShift(unsigned int dimension, double shift);

  //!@brief get the inhibition of the kernel for a given dimension
  double getInh(unsigned int dimension) const;

  //!@brief set the inhibition for a given dimension
  void setInh(unsigned int dimension, double inh);

  //!@brief get the amplitude of the kernel
  double getAmplitude(unsigned int dimesnion) const;

  //!@brief set the amplitude of the kernel
  void setAmplitude(unsigned int dimension, double amplitude);

  //!@brief set the limit of the kernel
  void setLimit(double limit);

  //--------------------------------------------------------------------------------------------------------------------
  // protected methods
  //--------------------------------------------------------------------------------------------------------------------
protected slots:
  //!@brief update the dimensionality of the kernel matrices, triggered by a signal (e.g. a changed parameter value)
  void updateDimensionality();

  //--------------------------------------------------------------------------------------------------------------------
  // private methods
  //--------------------------------------------------------------------------------------------------------------------
private:
  /*! virtual function that in the derived classes actually does the job of initializing
   * the kernel from file
   * @todo deal with boost PropertyTree here
   */
  virtual void onInit();

  /*!@brief virtual function to calculate the kernel matrix
   */
  void calculateParts();

  //!@brief A function that heuristically determines width of the kernel in pixels.
  unsigned int estimateWidth(unsigned int dim) const;

  //--------------------------------------------------------------------------------------------------------------------
  // members
  //--------------------------------------------------------------------------------------------------------------------
protected:
  //!@brief vector of pixel sizes of the kernel matrices
  std::vector<unsigned int> mSizes;
  //!@brief matrix indices for kernel centers
  std::vector<int> mCenters;
private:
  // none yet

  //--------------------------------------------------------------------------------------------------------------------
  // parameters
  //--------------------------------------------------------------------------------------------------------------------
protected:
  //!@brief amplitude of the kernel, modification: _mAmplitude is a vector that contains the amplitudes for each dimension
  cedar::aux::DoubleVectorParameterPtr _mAmplitudes;
  //!@brief sigmas of the Gauss function for each dimension
  cedar::aux::DoubleVectorParameterPtr _mSigmas;
  //!@brief shift of the Gauss function from the center for each dimension
  cedar::aux::DoubleVectorParameterPtr _mShifts;
  //!@brief global inhibition term can be separately applied to each dimension (this is the difference to the usual Gauss kernel)
  cedar::aux::DoubleVectorParameterPtr _mInhs;
  //!@brief scalar value, which is multiplied by the dimensions' sigmas to determine the pixel size
  cedar::aux::DoubleParameterPtr _mLimit;
private:
  // none yet

}; // class cedar::aux::kernel::MotionKernel

#endif // CEDAR_AUX_KERNEL_MOTION_KERNEL_H
