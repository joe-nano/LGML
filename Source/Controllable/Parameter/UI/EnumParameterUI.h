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


#ifndef ENUMPARAMETERUI_H_INCLUDED
#define ENUMPARAMETERUI_H_INCLUDED


#include "ParameterUI.h"
#include "../EnumParameter.h"



class EnumParameterUI : public ParameterUI, public EnumParameter::EnumListener, public ComboBox::Listener, public FileDragAndDropTarget
{
public:
    EnumParameterUI ( ParameterBase* parameter = nullptr);
    virtual ~EnumParameterUI();

    EnumParameter* ep;


    ComboBox cb;
    bool isSorted = true;

    void updateComboBox();
    String getCBSelectedKey();

    void resized() override;



    enum Actions
    {
        addElementId = -1,
        removeElementId = -2,
        NoneId = -3
    };


    void enumOptionAdded (EnumParameter*, const Identifier& key) override;
    void enumOptionRemoved (EnumParameter*, const Identifier& key) override;
    void enumOptionSelectionChanged (EnumParameter*, bool isSelected, bool isValid, const Identifier& name) override;
    void mouseDown(const MouseEvent & e)override;
    // Inherited via Listener
    virtual void comboBoxChanged (ComboBox*) override;

    bool isInterestedInFileDrag (const StringArray& files) override;
    void fileDragEnter (const StringArray& files, int x, int y) override;
    void fileDragMove (const StringArray& files, int x, int y) override;
    void fileDragExit (const StringArray& files) override;
    void filesDropped (const StringArray& files, int x, int y) override;
    
private:
    int lastId  = NoneId;
    HashMap<int, String> idKeyMap;
    HashMap<String, int> keyIdMap;

    void selectString (const String& );

protected:


    void paintOverChildren(Graphics & g) override;
    bool hoveredByFile = false;
    void valueChanged (const var&) override ;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnumParameterUI)


};

#endif  // ENUMPARAMETERUI_H_INCLUDED
