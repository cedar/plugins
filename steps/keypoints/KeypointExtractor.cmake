DECLARE_STEP(cedar::proc::steps::KeypointExtractor
             MOC
             CATEGORY                 "Keypoints"
             DESCRIPTION              "A step that extracts keypoint information."
             MAINTAINER               "Oliver Lomp"
             DEPRECATED_NAME          keypoints::KeypointExtractor
             REQUIRES_DATA_STRUCTURES cedar::aux::KeypointListData cedar::aux::KeypointData
             REQUIRES_LIBRARY "vislab_toolbox"
            )
