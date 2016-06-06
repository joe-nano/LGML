/*
  ==============================================================================

    AudioDeviceOutNodeUI.cpp
    Created: 25 May 2016 9:31:21am
    Author:  bkupe

  ==============================================================================
*/

#include "AudioDeviceOutNodeUI.h"



#include "AudioDeviceOutNodeUI.h"
#include "AudioDeviceOutNode.h"
#include "VuMeter.h"
#include "BoolToggleUI.h"
#include "FloatSliderUI.h"

AudioDeviceOutNodeContentUI::AudioDeviceOutNodeContentUI() :
	NodeBaseContentUI()
{

}

AudioDeviceOutNodeContentUI::~AudioDeviceOutNodeContentUI()
{
	audioOutNode->removeNodeBaseListener(this);
	audioOutNode->removeConnectableNodeListener(this);

	while (vuMeters.size() > 0)
	{
		removeLastVuMeter();
	}
}

void AudioDeviceOutNodeContentUI::init()
{
	audioOutNode = (AudioDeviceOutNode *)node.get();
	audioOutNode->addNodeBaseListener(this);
	audioOutNode->addConnectableNodeListener(this);
	updateVuMeters();

	setSize(240, 80);
}

void AudioDeviceOutNodeContentUI::resized()
{
	if (vuMeters.size() == 0) return;

	Rectangle<int> r = getLocalBounds().reduced(10);

	int gap = 0;
	int vWidth = (r.getWidth() / vuMeters.size()) - gap;

	for (int i = 0; i < vuMeters.size(); i++)
	{
        Rectangle<int> vr = r.removeFromLeft(vWidth);
        muteToggles[i]->setBounds(vr.removeFromBottom(20).reduced(2));
        vr.removeFromBottom(2);
        vuMeters[i]->setBounds(vr.removeFromLeft(vr.getWidth()/2).reduced(2));
        volumes[i]->setBounds(vr.reduced(2));
        r.removeFromLeft(gap);
	}
}

void AudioDeviceOutNodeContentUI::updateVuMeters()
{
    int actualNumberOfTracks = jmin(audioOutNode->desiredNumAudioOutput->intValue(),
                                   audioOutNode->AudioGraphIOProcessor::getTotalNumInputChannels());
	while (vuMeters.size() < actualNumberOfTracks)
	{
		addVuMeter();
	}

	while (vuMeters.size() > actualNumberOfTracks)
	{
		removeLastVuMeter();
	}
	resized();
}

void AudioDeviceOutNodeContentUI::addVuMeter()
{
    VuMeter * v = new VuMeter(VuMeter::Type::OUT);
    v->targetChannel = vuMeters.size();
    audioOutNode->addRMSChannelListener(v);
    addAndMakeVisible(v);
    vuMeters.add(v);

    int curVuMeterNum = muteToggles.size();
    BoolToggleUI * b = audioOutNode->outMutes[curVuMeterNum]->createToggle();
    b->invertVisuals = true;
    muteToggles.add(b);
    addAndMakeVisible(b);


    FloatSliderUI * vol  = audioOutNode->volumes[curVuMeterNum]->createSlider();
    vol->orientation = FloatSliderUI::Direction::VERTICAL;
    volumes.add(vol);
    addAndMakeVisible(vol);
}

void AudioDeviceOutNodeContentUI::removeLastVuMeter()
{
    int curVuMeterNum = vuMeters.size() - 1;
    VuMeter * v = vuMeters[curVuMeterNum];
    audioOutNode->removeRMSChannelListener(v);
    removeChildComponent(v);
    vuMeters.removeLast();

    removeChildComponent(muteToggles[curVuMeterNum]);
    muteToggles.removeLast();

    removeChildComponent(volumes[curVuMeterNum]);
    volumes.removeLast();
}
void AudioDeviceOutNodeContentUI::nodeParameterChanged(ConnectableNode *, Parameter * p)
{
    if(p==audioOutNode->desiredNumAudioOutput){
        updateVuMeters();
    }
	int index = 0;
	for (auto &m : muteToggles)
	{
		if (p == m->parameter)
		{
			if (p && p->boolValue()) vuMeters[index]->setVoldB(0);
		}
		index++;
	}
}

void AudioDeviceOutNodeContentUI::numAudioOutputChanged(NodeBase *, int)
{
	updateVuMeters();
}