/*
  ==============================================================================

    SerialControllerEditor.h
    Created: 22 May 2016 4:51:02pm
    Author:  bkupe

  ==============================================================================
*/

#ifndef SERIALCONTROLLEREDITOR_H_INCLUDED
#define SERIALCONTROLLEREDITOR_H_INCLUDED


#include "ControllerEditor.h"
#include "SerialUIHelper.h"
#include "JsEnvironmentUI.h"
#include "SerialController.h"//keep

class SerialControllerEditor :
	public ControllerEditor, ComboBoxListener,
	public SerialController::SerialControllerListener
{
public:
	SerialControllerEditor(Controller * controllerUI);
	virtual ~SerialControllerEditor();

	SerialDeviceChooser deviceChooser;

	SerialController * serialController;
	TextButton connectPortBT;

	ScopedPointer<JsEnvironmentUI> jsUI;
	ScopedPointer<ParameterUI> incomingToogle;

	OwnedArray<ParameterUI> variablesUI;

	int getContentHeight() override;


	void paint(Graphics &g) override;
	void resized() override;

	void updateConnectBTAndIndic();

	void comboBoxChanged(ComboBox *) override;
	void buttonClicked(Button *) override;

	void portOpened() override;
	void portClosed() override;
	void currentPortChanged() override;

};


#endif  // SERIALCONTROLLEREDITOR_H_INCLUDED
