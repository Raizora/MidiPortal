include("/Users/jbenchia/JUCE_Projects/MidiPortal/libs/cpm/CPM_0.40.2.cmake")
CPMAddPackage("NAME;GOOGLETEST;GITHUB_REPOSITORY;google/googletest;GIT_TAG;v1.15.2;VERSION;1.15.2;SOURCE_DIR;/Users/jbenchia/JUCE_Projects/MidiPortal/libs/googletest;OPTIONS;INSTALL_GTEST OFF;gtest_force_shared_crt ON")
set(GOOGLETEST_FOUND TRUE)