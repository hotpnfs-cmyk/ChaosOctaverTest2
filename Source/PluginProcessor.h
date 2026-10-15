#pragma once
#include <JuceHeader.h>
#include <vector>
class OctaveChaosAudioProcessor : public juce::AudioProcessor {
public:
 OctaveChaosAudioProcessor(); ~OctaveChaosAudioProcessor() override = default;
 void prepareToPlay(double,int) override; void releaseResources() override;
 bool isBusesLayoutSupported(const BusesLayout&) const override;
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
 juce::AudioProcessorEditor* createEditor() override; bool hasEditor() const override{return true;}
 const juce::String getName() const override{return JucePlugin_Name;}
 bool acceptsMidi() const override{return false;} bool producesMidi() const override{return false;} bool isMidiEffect() const override{return false;}
 double getTailLengthSeconds() const override{return 0.25;}
 int getNumPrograms() override{return 1;} int getCurrentProgram() override{return 0;}
 void setCurrentProgram(int) override{} const juce::String getProgramName(int) override{return {};}
 void changeProgramName(int,const juce::String&) override{}
 void getStateInformation(juce::MemoryBlock&) override; void setStateInformation(const void*,int) override;
 using APVTS=juce::AudioProcessorValueTreeState; APVTS parameters;
private:
 struct PitchVoice {
  std::vector<std::vector<float>> ring; int writePos=0, ringSize=0; double phase=0.0;
  void prepare(int,double); void reset();
  void process(const juce::AudioBuffer<float>&,juce::AudioBuffer<float>&,float,float);
  float read(int,float) const;
 };
 static APVTS::ParameterLayout createParameterLayout();
 juce::AudioBuffer<float> dryBuffer,voice1Buffer,voice2Buffer;
 PitchVoice voice1,voice2; double currentSampleRate=44100.0; juce::Random random;
 float chaosCurrent=0,chaosTarget=0; int samplesUntilNewTarget=0;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OctaveChaosAudioProcessor)
};
