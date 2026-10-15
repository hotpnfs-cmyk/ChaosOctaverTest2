#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
class OctaveChaosAudioProcessorEditor : public juce::AudioProcessorEditor {
public:
 explicit OctaveChaosAudioProcessorEditor(OctaveChaosAudioProcessor&); ~OctaveChaosAudioProcessorEditor() override=default;
 void paint(juce::Graphics&) override; void resized() override;
private:
 OctaveChaosAudioProcessor& audioProcessor;
 juce::ToggleButton octave1{"OCT +1"},octave2{"OCT +2"},panic{"PANIC"};
 juce::Slider blend,chaos,speed,rise;
 juce::Label title,blendLabel,chaosLabel,speedLabel,riseLabel;
 using ButtonAttachment=juce::AudioProcessorValueTreeState::ButtonAttachment;
 using SliderAttachment=juce::AudioProcessorValueTreeState::SliderAttachment;
 std::unique_ptr<ButtonAttachment> octave1Attachment,octave2Attachment,panicAttachment;
 std::unique_ptr<SliderAttachment> blendAttachment,chaosAttachment,speedAttachment,riseAttachment;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OctaveChaosAudioProcessorEditor)
};
