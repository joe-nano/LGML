/*
  ==============================================================================

    ShapeShifterFactory.h
    Created: 18 May 2016 11:33:09am
    Author:  bkupe

  ==============================================================================
*/

#ifndef SHAPESHIFTERFACTORY_H_INCLUDED
#define SHAPESHIFTERFACTORY_H_INCLUDED

#include "JuceHeader.h"//keep

class ShapeShifterContent;

const static StringArray globalPanelNames = { "Node Manager", "Time Manager", "Inpector","Logger","Controllers","Fast Mapper","Outliner" };
enum PanelName {NodeManagerPanel,TimeManagerPanel,InspectorPanel,LoggerPanel,ControllerPanel,FastMapperPanel,OutlinerPanel};

class ShapeShifterFactory
{
public:
	static ShapeShifterContent * createContentForIndex(PanelName panelName);
	static ShapeShifterContent * createContentForName(String name);
};



#endif  // SHAPESHIFTERFACTORY_H_INCLUDED
