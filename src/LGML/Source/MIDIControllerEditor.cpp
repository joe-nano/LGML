/*
  ==============================================================================

    MIDIControllerEditor.cpp
    Created: 11 May 2016 7:04:03pm
    Author:  bkupe

  ==============================================================================
*/

#include "MIDIControllerEditor.h"
#include "MIDIController.h"

#include "MIDIListener.h"
#include "IntStepperUI.h"
#include "ParameterUIFactory.h"

MIDIControllerEditor::MIDIControllerEditor(Controller * _controller) :
	ControllerEditor(_controller,false),
	deviceChooser(true)
{

  midiController = dynamic_cast<MIDIController*>( controller);
  jassert(midiController);
  jsUI = new JsEnvironmentUI(midiController->jsParameters);
	addAndMakeVisible(deviceChooser);
	deviceChooser.addListener(this);
	deviceChooser.setSelectedDevice(midiController->midiPortName,true);

    addAndMakeVisible(jsUI);

  incomingToogle = ParameterUIFactory::createDefaultUI(midiController->logIncoming);
    addAndMakeVisible(incomingToogle);

	channelStepper = new NamedControllableUI(new IntStepperUI(midiController->channelFilter),50);
	addAndMakeVisible(channelStepper);

	midiController->addMIDIListenerListener(this);
}

MIDIControllerEditor::~MIDIControllerEditor()
{
	midiController->removeMIDIListenerListener(this);
}

void MIDIControllerEditor::resized()
{
	ControllerEditor::resized();
  Rectangle<int> r = getLocalBounds();
	//r.removeFromTop(ControllerEditor::getContentHeight() + 10);
	deviceChooser.setBounds(r.removeFromTop(30));
	channelStepper->setBounds(r.removeFromTop(20));
	r.removeFromTop(5);
	incomingToogle->setBounds(r.removeFromTop(20));
  jsUI->setBounds(r.removeFromTop(40));
}

int MIDIControllerEditor::getContentHeight()
{
  return ControllerEditor::getContentHeight() + 140;
}

void MIDIControllerEditor::currentDeviceChanged(MIDIListener *)
{
	DBG("Current device changed : " << midiController->midiPortName);
	deviceChooser.setSelectedDevice(midiController->midiPortName, true);
}

void MIDIControllerEditor::comboBoxChanged(ComboBox *cb)
{
	if (cb == &deviceChooser)
	{
		if (deviceChooser.getSelectedItemIndex() > 0)
		{
			midiController->deviceInName->setValue(deviceChooser.getItemText(deviceChooser.getSelectedItemIndex()));
		}
	}
}
