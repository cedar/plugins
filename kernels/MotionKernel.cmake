DECLARE_KERNEL(cedar::aux::kernel::MotionKernel
               MOC
               DESCRIPTION      "This is a general version of the gauss kernel class. A global inhibition term can be added to each dimension separately. There is an amplitude parameter which scales the gaussian local excitation of each dimension but not the global inhibition term of this dimension. The benefit of this class in contrast to the usual Gauss kernel is that it is possible to introduce global excitation along only one dimension. This is, e.g., important for motion detection where you want to detect a single direction selective motion percept (global inhibition for motion direction) but for multiple objects (no global inhibition in the space dimension)."
               DEPRECATED_NAME  motionDetection::MotionKernel
              )
