DECLARE_STEP(cedar::proc::steps::ViewCombination
             MOC
             CATEGORY                 "Object Recognition"
             DESCRIPTION              "Subsamples discrete activation fields by summing up neighboring sampling points (1D only). This is intended for object recognition, where multiple nodes representing multiple object views may be summed to form a representation of a single object."
             MAINTAINER               "Oliver Lomp"
             DEPRECATED_NAME          recognition::ViewCombination
            )
