cmake_minimum_required(VERSION 3.22)
project(OctaveChaos VERSION 0.1.0)
include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.6 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)
juce_add_plugin(OctaveChaos
 COMPANY_NAME "Independent Audio Tools"
 BUNDLE_ID "com.independentaudiotools.octavechaos"
 PLUGIN_MANUFACTURER_CODE IATo PLUGIN_CODE OcHs
 FORMATS VST3 PRODUCT_NAME "Octave Chaos"
 IS_SYNTH FALSE NEEDS_MIDI_INPUT FALSE NEEDS_MIDI_OUTPUT FALSE IS_MIDI_EFFECT FALSE
 EDITOR_WANTS_KEYBOARD_FOCUS FALSE)
juce_generate_juce_header(OctaveChaos)
target_sources(OctaveChaos PRIVATE Source/PluginProcessor.cpp Source/PluginProcessor.h Source/PluginEditor.cpp Source/PluginEditor.h)
target_compile_features(OctaveChaos PRIVATE cxx_std_17)
target_link_libraries(OctaveChaos PRIVATE juce::juce_audio_utils juce::juce_dsp PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)
