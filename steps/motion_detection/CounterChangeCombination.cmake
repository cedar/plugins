DECLARE_STEP(cedar::proc::steps::CounterChangeCombination
             MOC
             CATEGORY         "Motion detection"
             DESCRIPTION      "The inputs have to be 3D toward and away matrices from a single edge filtered video or camera input. The first dimension of the input matrices is considered as the edge orientation dimension. The input matrices are shifted orthogonal the edge orientation by the size of the configurable shift parameter. Output matrices are two 3D matrices that reflect the combination of toward and away signals according to the counter-change rule. The first dimension is assigned to the motion direction. The two matrices represent the polarity if a dark pattern moves on bright ground (BtW: Black to White motion) or the other way around (WtB: White to Black motion)"
             DEPRECATED_NAME  motionDetection::CounterChangeCombination
            )
