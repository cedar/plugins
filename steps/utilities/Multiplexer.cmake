# all options that are specified with a keyword are optional (e.g., MOC
DECLARE_STEP(cedar::proc::steps::Multiplexer # the class name as used in the code; will be added to the plugin.cpp
#  ADDITIONAL_SOURCES my_source.cpp
             CATEGORY         "Programming" # the category in which the step appears in cedar
             MOC
#  ADDITIONAL_MOC_HEADERS my_special_field.h # we may also want to moc some headers for the step (usually just one)
  DESCRIPTION      "Joins several scalars (1x1 matrices) into a vector (nx1 matrix)."
#  MAINTAINER       "Dr. Whoeverisresponsible"
#  REQUIRES         # section for specifying different requirements
#    CEDAR          DEVELOPMENT # this step requires cedar development; maybe also/instead something like VERSION >= 5.0.0, and development always has a higher version than release?
#    LIBS           KEYPOINTS # an external library needed by this step
DEPRECATED       "Use Scalars to Vector step instead."
#  DEPRECATED_NAMES
#  OS               UNIX WINDOWS
#  DEPENDS_ON       cedar::proc::steps::SomeOtherStep
)
