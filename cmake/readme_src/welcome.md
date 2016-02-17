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
