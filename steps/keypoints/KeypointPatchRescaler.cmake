DECLARE_STEP(cedar::proc::steps::KeypointPatchRescaler
             MOC
             CATEGORY                 "Keypoints"
             DESCRIPTION              "Rescales a local image patch according to a chosen keypoint size."
             MAINTAINER               "Oliver Lomp"
             DEPRECATED_NAME          keypoints::Rescaler
             REQUIRES_STEP            cedar::proc::steps::KeypointExtractor
            )
