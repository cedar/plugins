# Welcome to the cedar plugin collection #

This is a collection of plugins for [cedar](http://cedar.ini.rub.de).

# Building the plugin #

TODO describe how to build the plugin

# Plugin configurations #

TODO find a better name for plugin configurations

Each pulgin configuration defines two things: 1. the name of the plugin being built; and, 2. what classes to build into
the plugin. To declare a plugin configuration, create a buildset (put it into the shared_buildsets folder if you want
others to have access to it) or edit the buildset.cmake in the root directory.

First, declare the name of the plugin configuration using

    DECLARE_PLUGIN(MyPlugin)

This tells the buildsystem that the shared object (or dll on Windows, dylib on Mac OS) should be named
libMyPlugin.so (Windows: MyPlugin.dll, Mac OS: libMyPlugin.dylib).

Next, you should add classes to this plugin. This is done with the `ADD_TO_PLUGIN` command, which offers various
options. To add a specific step class, call it like this:

    ADD_TO_PLUGIN(STEP some::Step)

You can also add multiple steps:

    ADD_TO_PLUGIN(STEPS some::Step some::other::Step)

If you want to add all steps from a category, you can write

    ADD_TO_PLUGIN(CATEGORY YourCategory)
    
As before, you can add multiple categories like this:

    ADD_TO_PLUGIN(CATEGORIES YourCategory YourOtherCategory)
    
Note that you can combine several of these commands in a single `ADD_TO_PLUGIN` call, for example:

    ADD_TO_PLUGIN(STEPS some::Step some::other::Step CATEGORY YourCategory)
    
Finally, you can add all classes of a kind using these commands:

    ADD_TO_PLUGIN(ALL_STEPS ALL_KERNELS)


# Steps included in the plugin #

| step                         | description                                                                    |
|------------------------------|--------------------------------------------------------------------------------|
| **Dynamics**                                                                                                  |
| *PatternMemory*              | A dynamics that relaxes to the input pattern as long as a learning input is active. |
| **Keypoints**                                                                                                 |
| *DoubleStoppedCellResponses* | Extracts the double-stopped cell responses from keypoint data.                 |
| *KeypointExtractor*          | A step that extracts keypoint information.                                     |
| *KeypointLinesAndEdges*      | Extracts line- and edge information from a KeypointData object.                |
| *KeypointPatchRescaler*      | Rescales a local image patch according to a chosen keypoint size.              |
| *KeypointVisualization*      | Draws visualizations for a list of opencv keypoints onto an image.             |
| **Motion detection**                                                                                          |
| *CounterChangeCombination*   | The inputs have to be 3D toward and away matrices from a single edge filtered video or camera input. The first dimension of the input matrices is considered as the edge orientation dimension. The input matrices are shifted orthogonal the edge orientation by the size of the configurable shift parameter. Output matrices are two 3D matrices that reflect the combination of toward and away signals according to the counter-change rule. The first dimension is assigned to the motion direction. The two matrices represent the polarity if a dark pattern moves on bright ground (BtW: Black to White motion) or the other way around (WtB: White to Black motion) |
| *MotionGradient*             | This class provides a cedar processing step implementing the OpenCV function calcMotionGradient. |
| **Nao**                                                                                                       |
| *NaoCamera*                  | Nao's camera.                                                                  |
| **Object Recognition**                                                                                        |
| *FeatureStacks*              | This step builds localized histograms around keypoints. See Lomp et al. (2014) for details. |
| *KeypointEdgeHistogramExtractor* | This step builds localized histograms around keypoints using shape features.   |
| *TopDownReconstruction*      | Reconstructs a shape based object representation.                              |
| **Sources**                                                                                                   |
| *ImageProvider*              | A step that outputs an image from a directory based on an index specified via a parameter. |
| *SpatialPattern*             | Outputs a matrix of synaptic weights that correspond to a spatial relational template (e.g., "to the left of"). |
| **Utilities**                                                                                                 |
| *AttentionSlice*             | A step that determines the location of the maximum in an activation matrix and cuts out a region around this location from an input image. |
| *BufferThief*                | This step can grab any buffer from a step in the same architecture and output the data in the buffer. Use with caution, as this may lead to issues due to unsafe/unlocked data. |
| *CrossCorrelation*           | Calculates the cross correlation in different configurations.                  |
| *Demultiplexer*              | Splits a vector (1xn or nx1 matrix) into individual scalars (1x1 matrices).    |
| *ElementwiseDivide*          | To input matrices Nom and Denom will be divided elementwisely. The output is Nom/(Denom + scalar). "scalar" is a scalar and configurable parameter. This value is added to all elements in the Denom matrix to prevent dividing by zero or very small values. |
| *HyperAcuteRescaling*        | Rescales an input matrix so that as little information as possible is lost. Note that this step's output is not properly normalized. |
| *Integrator*                 | A looped step that integrates nx1 inputs across time.                          |
| *LabelString*                | Outputs a label and ordered list of labels for activation from a label field.  |
| *MatrixThreadDecoupler*      | A looped step that makes a copy of its input matrix. This may help make threads more independent of each other. |
| *Multiplexer*                | Joins several scalars (1x1 matrices) into a vector (nx1 matrix).               |
| *ShiftedAddition*            | no description.                                                                |
| *WeightedSum*                | A looped step that multiplies each input value with a given weight and sums them up across each input dimension. The given weight function encodes the distance from the center of the input matrix. This step is used to calculate a velocity signal dependent on the distance between a peak position and the center of the input matrix. |
| *WeightedSumOfSlices*        | Calculates a weighted sum of the entries in a 3d or 4d matrix along the first dimension. |




# Kernels included in the plugin #

| kernel                       | description                                                                    |
|------------------------------|--------------------------------------------------------------------------------|
| *MotionKernel*               | This is a general version of the gauss kernel class. A global inhibition term can be added to each dimension separately. There is an amplitude parameter which scales the gaussian local excitation of each dimension but not the global inhibition term of this dimension. The benefit of this class in contrast to the usual Gauss kernel is that it is possible to introduce global excitation along only one dimension. This is, e.g., important for motion detection where you want to detect a single direction selective motion percept (global inhibition for motion direction) but for multiple objects (no global inhibition in the space dimension). |




# Data structures included in the plugin #

| data structure               | description                                                                    |
|------------------------------|--------------------------------------------------------------------------------|
| *KeypointData*               | A data structure that holds a vislab::keypoints::KPData object.                |
| *KeypointListData*           | A std::vector of (open)cv::Keypoints.                                          |
| *StringData*                 | A data structure that holds a string.                                          |




# Plots included in the plugin #

| plot                         | description                                                                    |
|------------------------------|--------------------------------------------------------------------------------|
| *StringPlot*                 | A plot that displays a string (cedar::aux::StringData).                        |




# External libraries used by the plugin #

## naoqi ##

TODO describe how to get, build and include the naoqi.
## vislab_toolbox ##

TODO describe how to get, build and include the vision lab toolbox.

