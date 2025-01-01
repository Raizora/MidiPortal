add_test([=[AudioProcessor.Foo]=]  [==[/Users/jbenchia/JUCE Projects/MidiPortal/cmake-build-debug/test/AudioPluginTest]==] [==[--gtest_filter=AudioProcessor.Foo]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[AudioProcessor.Foo]=]  PROPERTIES WORKING_DIRECTORY [==[/Users/jbenchia/JUCE Projects/MidiPortal/cmake-build-debug/test]==] SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==])
set(  AudioPluginTest_TESTS AudioProcessor.Foo)
