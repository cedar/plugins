DECLARE_STEP_BASE_CLASS(cedar::proc::steps::KeypointDataExtractorBase
                        DESCRIPTION              "A base class for further processing keypoint data extracted by the KeypointExtractor step."
                        MAINTAINER               "Oliver Lomp"
                        REQUIRES_DATA_STRUCTURES cedar::aux::KeypointListData cedar::aux::KeypointData
                        REQUIRES_LIBRARY "vislab_toolbox"
                       )
