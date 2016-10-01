/*
  ==============================================================================

    JavascriptControllerEditor.h
    Created: 10 May 2016 8:01:18pm
    Author:  bkupe

  ==============================================================================
*/

#ifndef JAVASCRIPTCONTROLLEREDITOR_H_INCLUDED
#define JAVASCRIPTCONTROLLEREDITOR_H_INCLUDED

#include "OSCControllerEditor.h"
class JavascriptControllerUI;
class JavascriptController;
class JsEnvironmentUI;

class JavascriptControllerEditor : public OSCControllerEditor
{
public:
	JavascriptControllerEditor(JavascriptControllerUI * jsControllerUI);
	virtual ~JavascriptControllerEditor();

	JavascriptController * jsController;

	ScopedPointer<JsEnvironmentUI> scriptUI;
    int getContentHeight()override;

	void resized() override;



	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JavascriptControllerEditor)
};


#endif  // JAVASCRIPTCONTROLLEREDITOR_H_INCLUDED