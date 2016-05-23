/*
  ==============================================================================

    ConnectableNode.h
    Created: 18 May 2016 11:33:58pm
    Author:  bkupe

  ==============================================================================
*/

#ifndef CONNECTABLENODE_H_INCLUDED
#define CONNECTABLENODE_H_INCLUDED

#include "JuceHeader.h"
#include "ControllableContainer.h"
#include "NodeFactory.h"
#include "Data.h"

class ConnectableNodeUI;

class ConnectableNode :
	public ControllableContainer
{
public:
	ConnectableNode(const String &name, NodeType type, bool _hasMainAudioControl = true);
	virtual ~ConnectableNode();

	NodeType type;

	//Interaction
	bool canBeRemovedByUser;
	bool userCanAccessInputs;
	bool userCanAccessOutputs;

	virtual bool hasAudioInputs();
	virtual bool hasAudioOutputs();
	virtual bool hasDataInputs();
	virtual bool hasDataOutputs();

	//Controllables (from ControllableContainer)
	StringParameter * nameParam;
	BoolParameter * enabledParam;
	FloatParameter * xPosition;
	FloatParameter * yPosition;

	void remove(bool askBeforeRemove = false);

	virtual void clear();

	var getJSONData() override;
	void loadJSONDataInternal(var data) override;


	void onContainerParameterChanged(Parameter * p) override;

	//ui
	virtual ConnectableNodeUI *  createUI();

	//Listener
	class ConnectableNodeListener
	{
	public:
		virtual ~ConnectableNodeListener() {}
		virtual void askForRemoveNode(ConnectableNode *) {}
		virtual void nodeEnableChanged(ConnectableNode *) {}
		virtual void nodeBypassChanged(ConnectableNode *) {}
	};

	ListenerList<ConnectableNodeListener> nodeListeners;
	void addNodeListener(ConnectableNodeListener* newListener) { nodeListeners.add(newListener); }
	void removeNodeListener(ConnectableNodeListener* listener) { nodeListeners.remove(listener); }




	//AUDIO
	bool hasMainAudioControl;

	FloatParameter * outputVolume;
	BoolParameter * bypass;
	StringArray inputChannelNames;
	StringArray outputChannelNames;

	virtual AudioProcessorGraph::Node * getAudioNode(bool forInput = true);
	virtual void addToAudioGraph();
	virtual void removeFromAudioGraph();

	void setInputChannelNames(int startChannel, StringArray names);
	void setOutputChannelNames(int startChannel, StringArray names);
	void setInputChannelName(int channelIndex, const String &name);
	void setOutputChannelName(int channelIndex, const String &name);
	String getInputChannelName(int channelIndex);
	String getOutputChannelName(int channelIndex);

	class  RMSListener
	{
	public:
		/** Destructor. */
		virtual ~RMSListener() {}
		virtual void RMSChanged(ConnectableNode * node, float rmsInValue, float rmsOutValue) = 0;
	};

	ListenerList<RMSListener> rmsListeners;
	void addRMSListener(RMSListener* newListener) { rmsListeners.add(newListener); }
	void removeRMSListener(RMSListener* listener) { rmsListeners.remove(listener); }




	//DATA
	virtual Data* getInputData(int dataIndex);
	virtual Data* getOutputData(int dataIndex);

	virtual int getTotalNumInputData();
	virtual int getTotalNumOutputData();

	virtual StringArray getInputDataInfos();
	virtual StringArray getOutputDataInfos();

	virtual Data::DataType getInputDataType(const String &dataName, const String &elementName);
	virtual Data::DataType getOutputDataType(const String &dataName, const String &elementName);

	virtual Data * getOutputDataByName(const String &dataName);
	virtual Data * getInputDataByName(const String &dataName);

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConnectableNode)
};


#endif  // CONNECTABLENODE_H_INCLUDED
