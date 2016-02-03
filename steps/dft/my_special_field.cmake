# all options that are specified with a keyword are optional (e.g., MOC
DECLARE_STEP
(
                   MY_SPECIAL_FIELD # this is a sort of target name, used by DEPENDS_ON
                   "cedar::dyn::MySpecialField" # the class name as used in the code; will be added to the plugin.cpp
                   "my_special_field.cpp" # followed by a list of source files belonging to a step (usually, a single cpp)
  MOC              my_special_field.h # we may also want to moc some headers for the step (usually just one)
  ICON             "bla.svg" # the icon to be used, if any
  CATEGORY         "My Field Category" # the category in which the step appears in cedar
  DESCRIPTION      "This does something, I swear!"
  REQUIRES         # section for specifying different requirements
    CEDAR          DEVELOPMENT # this step requires cedar development; maybe also something like VERSION >= 5.0.0?
    LIBS           KEYPOINTS # an external library needed by this step
  DEPRECATED       "Don't use this step any more."
  DEPRECATED_NAMES
  OS               UNIX WINDOWS
  DEPENDS_ON       SOME_OTHER_STEP
)
