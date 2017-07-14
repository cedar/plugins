DECLARE_STEP(cedar::proc::steps::WeightedSum
             CATEGORY         "Arrays"
             MOC
             DESCRIPTION      "A looped step that multiplies each input value with a given weight and sums them up across each input dimension. The given weight function encodes the distance from the center of the input matrix. This step is used to calculate a velocity signal dependent on the distance between a peak position and the center of the input matrix."
             MAINTAINER       "Jan Tekuelve"
             DEPRECATED_NAME  VelocityStep
            )
