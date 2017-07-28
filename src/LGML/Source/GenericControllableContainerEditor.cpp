/*
  ==============================================================================

    GenericControllableContainerEditor.cpp
    Created: 9 May 2016 6:41:59pm
    Author:  bkupe

  ==============================================================================
*/

#include "GenericControllableContainerEditor.h"
#include "InspectableComponent.h"
#include "ControllableUI.h"
#include "ParameterUIFactory.h"

GenericControllableContainerEditor::GenericControllableContainerEditor(ControllableContainer * _sourceContainer) :
	InspectorEditor(),
	parentBT("Up","Go back to parent container")
{

	parentBT.addListener(this);

  sourceContainer = _sourceContainer;
	addChildComponent(parentBT);

	
	setCurrentInspectedContainer(sourceContainer);

	sourceContainer->addControllableContainerListener(this);

	resized();
}

GenericControllableContainerEditor::~GenericControllableContainerEditor()
{
	if(sourceContainer.get())sourceContainer->removeControllableContainerListener(this);
	parentBT.removeListener(this);
	innerContainer->clear();
}

void GenericControllableContainerEditor::setCurrentInspectedContainer(ControllableContainer * cc,bool forceUpdate,	int recursiveInspectionLevel,bool canInspectChildContainersBeyondRecursion)
{
	if (cc == nullptr) return;
  bool wasShowingUser =false;
	if (innerContainer != nullptr)
	{
		if (!forceUpdate && cc == innerContainer->container) return;
    wasShowingUser = innerContainer->showUser.getToggleState();
		removeChildComponent(innerContainer);
		innerContainer = nullptr;
	}


	int ccLevel = 0;
	ControllableContainer * tc = cc;
	while (tc != sourceContainer)
	{
		ccLevel++;
		tc = tc->parentContainer;

		jassert(tc != nullptr); //If here, trying to inspect a container that is not a child of the source inspectable container
	}

  innerContainer = new CCInnerContainer(this,cc, 0, ccLevel == 0?recursiveInspectionLevel:0, canInspectChildContainersBeyondRecursion,wasShowingUser);

	addAndMakeVisible(innerContainer);

	parentBT.setVisible(ccLevel > 0);
	if(parentBT.isVisible() && cc->parentContainer != nullptr) parentBT.setButtonText("Up : " + cc->parentContainer->getNiceName());

	resized();

}

int GenericControllableContainerEditor::getContentHeight()
{
	if (innerContainer == nullptr) return InspectorEditor::getContentHeight();
	else return InspectorEditor::getContentHeight()+innerContainer->getContentHeight() + parentBT.getHeight() + 5;
}

void GenericControllableContainerEditor::resized()
{
	InspectorEditor::resized();

	if (innerContainer == nullptr) return;
	Rectangle<int> r = getLocalBounds();

	if (parentBT.isVisible())
	{
		parentBT.setBounds(r.removeFromTop(20));
		r.removeFromTop(2);
	}

	innerContainer->setBounds(r);
}

void GenericControllableContainerEditor::clear()
{
	if (innerContainer == nullptr) return;

	innerContainer->clear();
}

void GenericControllableContainerEditor::buttonClicked(Button * b)
{
	if (b == &parentBT)
	{
		setCurrentInspectedContainer(innerContainer->container->parentContainer);
	}
}

void GenericControllableContainerEditor::childStructureChanged(ControllableContainer *,ControllableContainer *)
{
  if(!MessageManager::getInstance()->isThisTheMessageThread()){
//    removeChildComponent(innerContainer);
    if(innerContainer){innerContainer->clear();removeChildComponent(innerContainer);innerContainer = nullptr;}
  postCommandMessage(CHILD_STRUCTURE_CHANGED);}
  else{
    handleCommandMessage(CHILD_STRUCTURE_CHANGED);
  }
	
}
void GenericControllableContainerEditor::handleCommandMessage(int cID){
  switch(cID){
    case CHILD_STRUCTURE_CHANGED:
      // force clear for now
      // TODO: check differences


      startTimer(100);

      break;
    default:
      jassertfalse;
      break;
  }
}
void GenericControllableContainerEditor::timerCallback(){
  if(sourceContainer.get())setCurrentInspectedContainer(sourceContainer,true);
  inspectorEditorListeners.call(&InspectorEditorListener::contentSizeChanged, this);
  stopTimer();
};


/////////////////////////////
//Inner Container
//////////////////////////////

CCInnerContainer::CCInnerContainer(GenericControllableContainerEditor * _editor, ControllableContainer * _container, int _level, int _maxLevel, bool _canAccessLowerContainers,bool shouldShowUser) :
	editor(_editor),
	container(_container),
	level(_level),
	maxLevel(_maxLevel),
	canAccessLowerContainers(_canAccessLowerContainers),
	containerLabel("containerLabel",_container->getNiceName())
{
	container->addControllableContainerListener(this);

	addAndMakeVisible(containerLabel);
	containerLabel.setFont(containerLabel.getFont().withHeight(10));
	containerLabel.setColour(containerLabel.backgroundColourId,BG_COLOR.brighter(.2f));
	containerLabel.setSize(containerLabel.getFont().getStringWidth(containerLabel.getText()) + 10,14);
	containerLabel.setColour(containerLabel.textColourId, TEXTNAME_COLOR);

  showUser.setClickingTogglesState(true);
  showUser.setToggleState(shouldShowUser, dontSendNotification);
  showUser.setButtonText("show User Params");
  addAndMakeVisible(showUser);
  showUser.addListener(this);
  
  rebuild();




}

CCInnerContainer::~CCInnerContainer()
{
	container->removeControllableContainerListener(this);
	clear();
}

void CCInnerContainer::rebuild(){
  clear();
  if (container->canHavePresets)
  {
    presetChooser = new PresetChooser(container);
    addAndMakeVisible(presetChooser);
  }
  for (auto &c : container->controllables)
  {
    if(!c->hideInEditor) addControllableUI(c);
  }

  if (level < maxLevel)
  {
    for (auto &cc : container->controllableContainers)
    {
      addCCInnerUI(cc);
    }
  }else if (level == maxLevel && canAccessLowerContainers)
  {
    for (auto &cc : container->controllableContainers)
    {
      addCCLink(cc);

    }
    
    
  }
  resized();
  editor->inspectorEditorListeners.call(&InspectorEditor::InspectorEditorListener::contentSizeChanged, editor);


}
void CCInnerContainer::addCCInnerUI(ControllableContainer * cc)
{

	CCInnerContainer * ccui = new CCInnerContainer(editor, cc, level + 1, maxLevel, canAccessLowerContainers);
	innerContainers.add(ccui);
	addAndMakeVisible(ccui);
  
}

void CCInnerContainer::removeCCInnerUI(ControllableContainer * cc)
{
	CCInnerContainer * ccui = getInnerContainerForCC(cc);
	if (ccui == nullptr) return;

	removeChildComponent(ccui);
	innerContainers.removeObject(ccui);
}

void CCInnerContainer::addCCLink(ControllableContainer * cc)
{
  if( auto subEditor = cc->getCustomEditor()){
    addAndMakeVisible(subEditor);
    lowerContainerLinks.add(subEditor);
  }
  else{
	CCLinkBT * bt = new CCLinkBT(cc);
	bt->addListener(this);
	addAndMakeVisible(bt);
	lowerContainerLinks.add(bt);
  }
}

void CCInnerContainer::removeCCLink(ControllableContainer * cc)
{

	CCLinkBT * bt = getCCLinkForCC(cc);
	if (bt == nullptr) return;

	bt->removeListener(this);
	removeChildComponent(bt);
	lowerContainerLinks.removeObject(bt);
}

void CCInnerContainer::addControllableUI(Controllable * c)
{
	if (c->isControllableFeedbackOnly || !c->isControllableExposed) return;
  if(c->isUserDefined != showUser.getToggleState()) return;

  NamedControllableUI * cui = new NamedControllableUI(ParameterUIFactory::createDefaultUI(c->getParameter()), 100);
	controllablesUI.add(cui);
	addAndMakeVisible(cui);
}

void CCInnerContainer::removeControllableUI(Controllable * c)
{
	NamedControllableUI * cui = getUIForControllable(c);
	if (cui == nullptr) return;

	removeChildComponent(cui);
	controllablesUI.removeObject(cui);

}

NamedControllableUI * CCInnerContainer::getUIForControllable(Controllable * c)
{
	for (auto &cui : controllablesUI)
	{

      if(cui->controllable == c) return cui;

	}

	return nullptr;
}

CCInnerContainer * CCInnerContainer::getInnerContainerForCC(ControllableContainer * cc)
{
	for (auto &ccui : innerContainers)
	{if (auto ncui = dynamic_cast<CCInnerContainer*>(ccui)){
		if (ncui->container == cc) return ncui;
  }
	}

	return nullptr;
}

CCInnerContainer::CCLinkBT * CCInnerContainer::getCCLinkForCC(ControllableContainer * cc)
{
	for (auto &_cclink : lowerContainerLinks)
	{
    if(auto cclink = dynamic_cast<CCInnerContainer::CCLinkBT *>(_cclink)){
		if (cclink->targetContainer == cc) return cclink;
    }
	}

	return nullptr;
}

int CCInnerContainer::getContentHeight()
{
	int gap = 2;
	int ccGap = 5;
	int controllableHeight = 15;
	int ccLinkHeight = 20;
	int margin = 5;
	int presetChooserHeight = 15;

	int h = ccGap;
	h += controllablesUI.size()* (controllableHeight + gap) + ccGap;
  for(auto & c:lowerContainerLinks){
    bool isCustom = dynamic_cast<CCInnerContainer::CCLinkBT*>(c)==nullptr;
    h +=  (ccLinkHeight*(isCustom?3:1) + gap) + ccGap;
  }

  for (auto &ccui : innerContainers){
    if(auto icUI = dynamic_cast<CCInnerContainer*>(ccui)){h += icUI->getContentHeight();}
    else{h+=controllableHeight*2;}

    h+=ccGap;
  }

	if(container->canHavePresets) h += presetChooserHeight + gap;
  if(showUser.getToggleState()) h += presetChooserHeight + gap;
	h += containerLabel.getHeight();
	h += margin * 2;

	return h;
}

void CCInnerContainer::paint(Graphics & g)
{
	//if (level == 0) return;
	g.setColour(BG_COLOR.brighter(.3f));
	g.drawRoundedRectangle(getLocalBounds().toFloat(),4,2);
}

void CCInnerContainer::resized()
{
	int gap = 2;
	int ccGap = 5;
	int controllableHeight = 15;
	int ccLinkHeight = 20;
	int margin = 5;
	int presetChooserHeight = 15;

	Rectangle<int> r = getLocalBounds();
	containerLabel.setBounds(r.removeFromTop(containerLabel.getHeight()).withSizeKeepingCentre(containerLabel.getWidth(), containerLabel.getHeight()));

	r.reduce(margin, margin);
  if(customEditor){
    customEditor->setBounds(r);
  }
  else{
	if (container->canHavePresets  )
	{
		presetChooser->setBounds(r.removeFromTop(presetChooserHeight));
		r.removeFromTop(gap);
	}
    if (container->userParameterMap.size()>0)
    {
      showUser.setBounds(r.removeFromTop(presetChooserHeight));
      r.removeFromTop(gap);
    }
    
    for (auto &cui : controllablesUI)
	{
    cui->setBounds(r.removeFromTop(controllableHeight));
		r.removeFromTop(gap);
	}
	r.removeFromTop(ccGap);

	if (canAccessLowerContainers)
	{
		for (auto &cclink : lowerContainerLinks)
		{
      bool isCustom = dynamic_cast<CCInnerContainer::CCLinkBT*> (cclink) ==nullptr;

      cclink->setBounds(r.removeFromTop(ccLinkHeight*(isCustom?3:1)));
			r.removeFromTop(gap);
		}

		r.removeFromTop(ccGap);
	}

	for (auto &ccui : innerContainers)
	{
    int h=controllableHeight*2;
    if(auto icUI = dynamic_cast<CCInnerContainer*>(ccui)){h = icUI->getContentHeight();}

		ccui->setBounds(r.removeFromTop(h));
		r.removeFromTop(ccGap);
	}
  }

}

void CCInnerContainer::clear()
{
	controllablesUI.clear();
	innerContainers.clear();
	lowerContainerLinks.clear();
}

void CCInnerContainer::controllableAdded(ControllableContainer *,Controllable * c)
{
	if (c->parentContainer != container) return;
	if (c->hideInEditor) return;
	addControllableUI(c);
}

void CCInnerContainer::controllableRemoved(ControllableContainer *,Controllable * c)
{
	removeControllableUI(c);
}

void CCInnerContainer::controllableContainerAdded(ControllableContainer*,ControllableContainer * cc)
{
	if (cc->parentContainer != container) return;

	if (level < maxLevel) addCCInnerUI(cc);
	else if (canAccessLowerContainers) addCCLink(cc);
}

void CCInnerContainer::controllableContainerRemoved(ControllableContainer *,ControllableContainer * cc)
{
	removeCCInnerUI(cc);
	removeCCLink(cc);
}


void CCInnerContainer::childStructureChanged(ControllableContainer *,ControllableContainer *)
{
	//resized();

}

void CCInnerContainer::buttonClicked(Button * b)
{
  if(b==&showUser){
      rebuild();

  }
  else{
    CCLinkBT * bt = dynamic_cast<CCLinkBT *>(b);
    if (bt == nullptr) return;

    editor->setCurrentInspectedContainer(bt->targetContainer);
  }
}

CCInnerContainer::CCLinkBT::CCLinkBT(ControllableContainer * _targetContainer) :
	targetContainer(_targetContainer),
	TextButton("[ Inspect "+_targetContainer->getNiceName()+" >> ]")
{
}
