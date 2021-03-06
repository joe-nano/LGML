/* Copyright © Organic Orchestra, 2017
*
* This file is part of LGML.  LGML is a software to manipulate sound in realtime
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation (version 3 of the License).
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
*/
#if !ENGINE_HEADLESS && !NON_INCREMENTAL_COMPILATION
//
#include "AudioDeviceInNodeUI.h"
#include "AudioDeviceInNode.h"
#include "../../UI/VuMeter.h"
#include "../../Controllable/Parameter/UI/BoolToggleUI.h"
#include "../../UI/Style.h"
#include "../../Controllable/Parameter/UI/ParameterUIFactory.h"

#include "../UI/NodeUIFactory.h"


AudioDeviceManager& getAudioDeviceManager();


AudioDeviceInNodeContentUI::AudioDeviceInNodeContentUI()
{
    getAudioDeviceManager().addChangeListener (this);
}

AudioDeviceInNodeContentUI::~AudioDeviceInNodeContentUI()
{
    getAudioDeviceManager().removeChangeListener (this);

    audioInNode->removeConnectableNodeListener (this);


    while (vuMeters.size() > 0)
    {
        removeLastVuMeter();
    }
}

void AudioDeviceInNodeContentUI::init()
{
    audioInNode = (AudioDeviceInNode*)node.get();
    audioInNode->addConnectableNodeListener (this);

    updateVuMeters();
    setDefaultSize (100, 80);
    setDefaultPosition(10,100);

}

void AudioDeviceInNodeContentUI::resized()
{
    if (vuMeters.size() == 0) return;

    Rectangle<int> r = getLocalBounds().reduced (10);

    int gap = 0;
    auto vWidth = jmin<int> ((r.getWidth() / vuMeters.size()) - gap, 30);

    for (int i = 0; i < vuMeters.size(); i++)
    {
        Rectangle<int> vr = r.removeFromLeft (vWidth);
        muteToggles[i]->setBounds (vr.removeFromBottom (20).reduced (2));
        vr.removeFromBottom (2);
        vuMeters[i]->setBounds (vr.removeFromLeft (vr.getWidth() / 2).reduced (2));
        volumes[i]->setBounds (vr.reduced (2));
        r.removeFromLeft (gap);
    }
}

void AudioDeviceInNodeContentUI::updateVuMeters()
{

    int desiredNumInputs = audioInNode->desiredNumAudioInput->intValue();
    int actualNumberOfMutes = jmin (desiredNumInputs,
                                    audioInNode->AudioGraphIOProcessor::getTotalNumOutputChannels());


    while (vuMeters.size() < desiredNumInputs)
    {
        addVuMeter();
    }

    while (vuMeters.size() > desiredNumInputs)
    {
        removeLastVuMeter();
    }

    for (int i = 0; i < desiredNumInputs; i++)
    {
        bool isActive = i < actualNumberOfMutes;
        volumes[i]->setColour(Slider::trackColourId,isActive ? findColour (Slider::trackColourId) : Colours::darkgrey);
        vuMeters[i]->isActive = isActive;
        volumes[i]->repaint();
    }

    resized();
}

void AudioDeviceInNodeContentUI::addVuMeter()
{
    auto * v = new VuMeter (VuMeter::Type::IN);
    v->targetChannel = vuMeters.size();
    audioInNode->addDeviceChannelListener (v);
    addAndMakeVisible (v);
    vuMeters.add (v);

    int curVuMeterNum = muteToggles.size();
    jassert (curVuMeterNum < audioInNode->volumes.size() );
    auto b = muteToggles.add (ParameterUIFactory::createDefaultUI (audioInNode->inMutes[curVuMeterNum]));
    b->setCustomText(String(v->targetChannel+1));

    addAndMakeVisible (b);


    auto * vol  = new FloatSliderUI (audioInNode->volumes[curVuMeterNum]);
    vol->orientation = FloatSliderUI::Direction::VERTICAL;
    volumes.add (vol);
    addAndMakeVisible (vol);
}

void AudioDeviceInNodeContentUI::removeLastVuMeter()
{
    int curVuMeterNum = vuMeters.size() - 1;
    VuMeter* v = vuMeters[curVuMeterNum];
    audioInNode->removeDeviceChannelListener (v);
    removeChildComponent (v);
    vuMeters.removeLast();

    removeChildComponent (muteToggles[curVuMeterNum]);
    muteToggles.removeLast();

    removeChildComponent (volumes[curVuMeterNum]);
    volumes.removeLast();
}

void AudioDeviceInNodeContentUI::nodeParameterChangedAsync (ConnectableNode*, ParameterBase* p)
{
    int index = 0;

    for (auto& m : muteToggles)
    {
        if (p == m->parameter.get())
        {
            vuMeters[index]->isActive = p && !p->boolValue();
        }

        index++;
    }

}

void AudioDeviceInNodeContentUI::numAudioOutputChanged (ConnectableNode*, int)
{
    updateVuMeters();
}


void AudioDeviceInNodeContentUI::changeListenerCallback (ChangeBroadcaster*)
{
    updateVuMeters();
}
#endif
