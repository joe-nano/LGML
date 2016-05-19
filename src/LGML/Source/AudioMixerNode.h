/*
 ==============================================================================

 AudioMixerNode.h
 Created: 3 Mar 2016 10:14:46am
 Author:  bkupe

 ==============================================================================
 */

#ifndef AUDIOMIXERNODE_H_INCLUDED
#define AUDIOMIXERNODE_H_INCLUDED


#include "NodeBase.h"
class NodeBaseUI;

class AudioMixerNode : public NodeBase
{

public:
	class OutputBus : public ControllableContainer {
	public:
		OutputBus(int _outputIndex, int numInput);
		void setNumInput(int numInput);

		Array<FloatParameter*> volumes;
		Array<float> lastVolumes;
		int outputIndex;
	};


    AudioMixerNode() ;

	OwnedArray<OutputBus> outBuses;
	AudioBuffer<float> cachedBuffer;


	IntParameter * numberOfInput;
	IntParameter * numberOfOutput;

	void updateInput();
	void updateOutput();


	void processBlockInternal(AudioBuffer<float>& buffer, MidiBuffer&)override;
	void onContainerParameterChanged(Parameter * p) override;

    NodeBaseUI * createUI() override;


private:


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioMixerNode)
};

#endif  // AUDIOMIXERNODE_H_INCLUDED
