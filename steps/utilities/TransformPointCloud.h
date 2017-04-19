#ifndef TRANSFORM_POINT_CLOUD_H
#define TRANSFORM_POINT_CLOUD_H

// WORKAROUND FOR QT_FOREACH Macro
#include <boost/foreach.hpp>

// LOCAL INCLUDES
#include "data_structures/RGBAPointCloudData.h"
// PROJECT INCLUDES

// SYSTEM INCLUDES
#include <pcl/point_types.h>
#include <cedar/processing/Step.h>
#include <cedar/auxiliaries/DoubleParameter.h>
#include <cedar/auxiliaries/MatData.h>

#include "steps/utilities/TransformPointCloud.fwd.h"

/*!@brief Abstract description of the class.
 *
 * More detailed description of the class.
 */
class cedar::proc::steps::TransformPointCloud : public cedar::proc::Step
{
  Q_OBJECT
  //--------------------------------------------------------------------------------------------------------------------
  // macros
  //--------------------------------------------------------------------------------------------------------------------

  //--------------------------------------------------------------------------------------------------------------------
  // constructors and destructor
  //--------------------------------------------------------------------------------------------------------------------
public:
  //!@brief The standard constructor.
  TransformPointCloud();

  //--------------------------------------------------------------------------------------------------------------------
  // methods
  //--------------------------------------------------------------------------------------------------------------------
public:
  void compute(const cedar::proc::Arguments&);
  void inputConnectionChanged(const std::string& inputName);

protected:
  cedar::proc::DataSlot::VALIDITY determineInputValidity(cedar::proc::ConstDataSlotPtr slot, cedar::aux::ConstDataPtr data) const;

private:
  // input
  cedar::aux::ConstRGBAPointCloudDataPtr mpInputPointCloud;

  //output
  cedar::aux::RGBAPointCloudDataPtr mpOutputPointCloud;
  cedar::aux::MatDataPtr mpCameraPosition;

  //param
  cedar::aux::DoubleParameterPtr mpXTranslation;
  cedar::aux::DoubleParameterPtr mpYTranslation;
  cedar::aux::DoubleParameterPtr mpZTranslation;
  cedar::aux::DoubleParameterPtr mpRoll;
  cedar::aux::DoubleParameterPtr mpPitch;
  cedar::aux::DoubleParameterPtr mpYaw;
};

#endif // TRANSFORM_POINT_CLOUD_H
