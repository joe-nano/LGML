/*
==============================================================================

TriggerBlinkUI.cpp
Created: 8 Mar 2016 3:45:53pm
Author:  bkupe

==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "TriggerBlinkUI.h"

//==============================================================================
TriggerBlinkUI::TriggerBlinkUI(Trigger *t) :
	TriggerUI(t),
	blinkTime(200),
	refreshPeriod(10),
	intensity(0)
{
	setSize(30, 20);
}

TriggerBlinkUI::~TriggerBlinkUI()
{
	trigger->removeListener(this);
}

void TriggerBlinkUI::setTriggerReference(Trigger * t) {
	if (trigger != nullptr) {
		trigger->removeListener(this);
	}

	trigger = t;

	trigger->addListener(this);
}

void TriggerBlinkUI::triggerTriggered(Trigger * p) {
	startBlink();
}

void TriggerBlinkUI::paint(Graphics& g)
{
	g.setColour(Colours::white.withAlpha(intensity));
	g.fillRect(getLocalBounds());

	g.setColour(Colours::white);
	g.drawRect(getLocalBounds());

}


void TriggerBlinkUI::startBlink(){
	intensity = 1;
	startTimer(refreshPeriod);
}

void TriggerBlinkUI::timerCallback(){
	intensity-= refreshPeriod*1.0/blinkTime;
	if(intensity<0){
	intensity = 0;
	stopTimer();
	}
	repaint();
}


void TriggerBlinkUI::resized()
{
	// This method is where you should set the bounds of any child
	// components that your component contains..

}

void TriggerBlinkUI::mouseDown(const MouseEvent& event) {
	trigger->trigger();

}
