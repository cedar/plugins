# Welcome to the cedar plugin collection #

This is a collection of plugins for [cedar](http://cedar.ini.rub.de).

# Building the plugin #

TODO describe how to build the plugin

# Plugin configurations #

TODO find a better name for plugin configurations
TODO describe plugin configurations, show examples


# Steps included in the plugin #

## Motion detection ##

**CounterChangeCombination**: The inputs have to be 3D toward and away matrices from a single edge filtered video or camera input. The first dimension of the input matrices is considered as the edge orientation dimension. The input matrices are shifted orthogonal the edge orientation by the size of the configurable shift parameter. Output matrices are two 3D matrices that reflect the combination of toward and away signals according to the counter-change rule. The first dimension is assigned to the motion direction. The two matrices represent the polarity if a dark pattern moves on bright ground (BtW: Black to White motion) or the other way around (WtB: White to Black motion)

**MotionGradient**: This class provides a cedar processing step implementing the OpenCV function calcMotionGradient.



## Nao ##

**NaoCamera**: Nao's camera.



## Sources ##

**ImageProvider**: A step that outputs an image from a directory based on an index specified via a parameter.

**SpatialPattern**: Outputs a matrix of synaptic weights that correspond to a spatial relational template (e.g., "to the left of").



## Utilities ##

**AttentionSlice**: A step that determines the location of the maximum in an activation matrix and cuts out a region around this location from an input image.

**BufferThief**: This step can grab any buffer from a step in the same architecture and output the data in the buffer. Use with caution, as this may lead to issues due to unsafe/unlocked data.

**Demultiplexer**: Splits a vector (1xn or nx1 matrix) into individual scalars (1x1 matrices).

**ElementwiseDivide**: To input matrices Nom and Denom will be divided elementwisely. The output is Nom/(Denom + scalar). "scalar" is a scalar and configurable parameter. This value is added to all elements in the Denom matrix to prevent dividing by zero or very small values.

**MatrixThreadDecoupler**: A looped step that makes a copy of its input matrix. This may help make threads more independent of each other.

**Multiplexer**: Joins several scalars (1x1 matrices) into a vector (nx1 matrix).

**ShiftedAddition**: no description.





# External libraries used by the plugin #

## naoqi ##

TODO describe how to get, build and include the naoqi.

