DECLARE_STEP(cedar::proc::steps::ElementwiseDivide
             MOC
             CATEGORY         "Algebra"
             DESCRIPTION      "To input matrices Nom and Denom will be divided elementwisely. The output is Nom/(Denom + scalar). \"scalar\" is a scalar and configurable parameter. This value is added to all elements in the Denom matrix to prevent dividing by zero or very small values."
             MAINTAINER       "Oliver Lomp"
             DEPRECATED_NAME  motionDetection::ElementwiseDivide
            )
