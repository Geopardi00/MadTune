cmake_minimum_required(VERSION 3.22)

project(MadTune VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(MADTUNE_BUILD_PLUGIN "Build the VST3 plugin (downloads JUCE)" ON)
option(MADTUNE_BUILD_TESTS "Build the DSP unit tests" ON)

# Pure DSP code, no JUCE dependency, so it can be unit tested on its own.
add_library(madtune_dsp INTERFACE)
target_include_directories(madtune_dsp INTERFACE Source)

if(MADTUNE_BUILD_PLUGIN)
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.8
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)

    juce_add_plugin(MadTune
        COMPANY_NAME "Geopardi"
        PLUGIN_MANUFACTURER_CODE Gpdi
        PLUGIN_CODE MdTn
        FORMATS VST3
        PRODUCT_NAME "MadTune"
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        COPY_PLUGIN_AFTER_BUILD FALSE
        VST3_CATEGORIES Fx Pitch Shift)

    target_sources(MadTune PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp)

    target_compile_definitions(MadTune PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0)

    target_link_libraries(MadTune
        PRIVATE
            madtune_dsp
            juce::juce_audio_utils
            juce::juce_dsp
        PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags)
endif()

if(MADTUNE_BUILD_TESTS)
    enable_testing()
    add_executable(madtune_tests tests/ScaleTests.cpp)
    target_link_libraries(madtune_tests PRIVATE madtune_dsp)
    add_test(NAME madtune_tests COMMAND madtune_tests)

    add_executable(madtune_pitch_tests tests/PitchDetectorTests.cpp)
    target_link_libraries(madtune_pitch_tests PRIVATE madtune_dsp)
    add_test(NAME madtune_pitch_tests COMMAND madtune_pitch_tests)
endif()
