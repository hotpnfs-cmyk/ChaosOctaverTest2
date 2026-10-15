#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
namespace { constexpr auto p1="octave1"; constexpr auto p2="octave2"; constexpr auto pb="blend"; constexpr auto pp="panic"; constexpr auto pc="chaos"; constexpr auto ps="speed"; constexpr auto pr="rise"; }

void OctaveChaosAudioProcessor::PitchVoice::prepare(int channels,double sr) {
 ringSize=juce::jmax(4096,(int)(sr*0.18)); ring.assign((size_t)channels,std::vector<float>((size_t)ringSize,0.0f)); writePos=0; phase=0;
}
void OctaveChaosAudioProcessor::PitchVoice::reset() {
 for(auto& c:ring) std::fill(c.begin(),c.end(),0.0f); writePos=0; phase=0;
}
float OctaveChaosAudioProcessor::PitchVoice::read(int ch,float delay) const {
 if(ch<0||ch>=(int)ring.size()||ringSize<2)return 0;
 float pos=(float)writePos-delay; while(pos<0)pos+=ringSize; while(pos>=ringSize)pos-=ringSize;
 int a=(int)pos,b=(a+1)%ringSize; float f=pos-a;
 return ring[(size_t)ch][(size_t)a]*(1-f)+ring[(size_t)ch][(size_t)b]*f;
}
void OctaveChaosAudioProcessor::PitchVoice::process(const juce::AudioBuffer<float>& in,juce::AudioBuffer<float>& out,float ratio,float mod) {
 const int chs=juce::jmin(in.getNumChannels(),out.getNumChannels(),(int)ring.size()), n=in.getNumSamples();
 const float lo=48.0f, hi=(float)(ringSize*0.72), span=hi-lo;
 const float step=(juce::jlimit(1.5f,4.5f,ratio)-1.0f)/span;
 const float depth=1.0f+juce::jlimit(-0.08f,0.08f,mod)*0.2f;
 for(int i=0;i<n;++i) {
  for(int c=0;c<chs;++c) ring[(size_t)c][(size_t)writePos]=in.getReadPointer(c)[i];
  phase+=step; phase-=std::floor(phase); float q=(float)phase,q2=q<0.5f?q+0.5f:q-0.5f;
  float da=lo+q*span,db=lo+q2*span;
  float wa=0.5f-0.5f*std::cos(juce::MathConstants<float>::twoPi*q),wb=1-wa;
  for(int c=0;c<chs;++c) out.getWritePointer(c)[i]=read(c,da*depth)*wa+read(c,db*depth)*wb;
  writePos=(writePos+1)%ringSize;
 }
 for(int c=chs;c<out.getNumChannels();++c)out.clear(c,0,n);
}
OctaveChaosAudioProcessor::OctaveChaosAudioProcessor():AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),parameters(*this,nullptr,"PARAMETERS",createParameterLayout()){}
OctaveChaosAudioProcessor::APVTS::ParameterLayout OctaveChaosAudioProcessor::createParameterLayout(){
 APVTS::ParameterLayout l;
 l.add(std::make_unique<juce::AudioParameterBool>(p1,"Octave +1",false));
 l.add(std::make_unique<juce::AudioParameterBool>(p2,"Octave +2",false));
 l.add(std::make_unique<juce::AudioParameterFloat>(pb,"Blend",juce::NormalisableRange<float>(0,1,0.001f),0.65f));
 l.add(std::make_unique<juce::AudioParameterBool>(pp,"Panic",false));
 l.add(std::make_unique<juce::AudioParameterFloat>(pc,"Chaos",juce::NormalisableRange<float>(0,1,0.001f),0.25f));
 l.add(std::make_unique<juce::AudioParameterFloat>(ps,"Speed",juce::NormalisableRange<float>(0.1f,20,0.01f,0.5f),5));
 l.add(std::make_unique<juce::AudioParameterFloat>(pr,"Rise",juce::NormalisableRange<float>(0,2,0.001f),0.15f)); return l;
}
void OctaveChaosAudioProcessor::prepareToPlay(double sr,int block){
 currentSampleRate=sr; int ch=getTotalNumOutputChannels(); dryBuffer.setSize(ch,block);voice1Buffer.setSize(ch,block);voice2Buffer.setSize(ch,block);voice1.prepare(ch,sr);voice2.prepare(ch,sr);chaosCurrent=chaosTarget=0;samplesUntilNewTarget=0;
}
void OctaveChaosAudioProcessor::releaseResources(){}
bool OctaveChaosAudioProcessor::isBusesLayoutSupported(const BusesLayout& l) const {auto in=l.getMainInputChannelSet(),out=l.getMainOutputChannelSet();return in==out&&(out==juce::AudioChannelSet::mono()||out==juce::AudioChannelSet::stereo());}
void OctaveChaosAudioProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&){
 juce::ScopedNoDenormals guard;int ch=b.getNumChannels(),n=b.getNumSamples();
 if(dryBuffer.getNumSamples()<n||dryBuffer.getNumChannels()!=ch){dryBuffer.setSize(ch,n,false,false,true);voice1Buffer.setSize(ch,n,false,false,true);voice2Buffer.setSize(ch,n,false,false,true);}
 dryBuffer.makeCopyOf(b,true);
 bool o1=parameters.getRawParameterValue(p1)->load()>0.5f,o2=parameters.getRawParameterValue(p2)->load()>0.5f,panic=parameters.getRawParameterValue(pp)->load()>0.5f;
 float blend=parameters.getRawParameterValue(pb)->load(),chaos=parameters.getRawParameterValue(pc)->load(),speed=parameters.getRawParameterValue(ps)->load(),rise=parameters.getRawParameterValue(pr)->load();
 float smooth=1.0f-std::exp(-1.0f/(float)(currentSampleRate*juce::jmax(0.001f,rise)));
 int period=juce::jmax(1,(int)(currentSampleRate/juce::jmax(0.1f,speed)));
 for(int i=0;i<n;++i){if(samplesUntilNewTarget<=0){chaosTarget=panic?(random.nextFloat()*2-1)*chaos:0;samplesUntilNewTarget=period;}--samplesUntilNewTarget;chaosCurrent+=(chaosTarget-chaosCurrent)*(rise<=0.001f?1.0f:smooth);}
 voice1.process(dryBuffer,voice1Buffer,2.0f,chaosCurrent);voice2.process(dryBuffer,voice2Buffer,4.0f,chaosCurrent);
 for(int c=0;c<ch;++c){auto*out=b.getWritePointer(c);auto*d=dryBuffer.getReadPointer(c);auto*a=voice1Buffer.getReadPointer(c);auto*z=voice2Buffer.getReadPointer(c);for(int i=0;i<n;++i){float wet=0;int active=0;if(o1){wet+=a[i];++active;}if(o2){wet+=z[i];++active;}if(active>1)wet/=active;out[i]=active==0?d[i]:d[i]*(1-blend)+wet*blend;}}
}
juce::AudioProcessorEditor* OctaveChaosAudioProcessor::createEditor(){return new OctaveChaosAudioProcessorEditor(*this);}
void OctaveChaosAudioProcessor::getStateInformation(juce::MemoryBlock& dest){auto s=parameters.copyState();std::unique_ptr<juce::XmlElement>x(s.createXml());copyXmlToBinary(*x,dest);}
void OctaveChaosAudioProcessor::setStateInformation(const void*data,int size){std::unique_ptr<juce::XmlElement>x(getXmlFromBinary(data,size));if(x&&x->hasTagName(parameters.state.getType()))parameters.replaceState(juce::ValueTree::fromXml(*x));}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new OctaveChaosAudioProcessor();}
