/*
 ==============================================================================

 NodeManagerUI.cpp
 Created: 3 Mar 2016 10:38:22pm
 Author:  bkupe

 ==============================================================================
 */

#include "NodeManagerUI.h"
#include "NodeConnectionUI.h"



SelectableComponentHandler NodeManagerUI::selectableHandler;




//==============================================================================
NodeManagerUI::NodeManagerUI(NodeManager * nodeManager) :
nodeManager(nodeManager),
editingConnection(nullptr),
isSelectingNodes(false)
{
    nodeManager->addNodeManagerListener(this);
    setInterceptsMouseClicks(true, true);
    addAndMakeVisible(selectingBounds);

}

NodeManagerUI::~NodeManagerUI()
{
    if (editingConnection != nullptr)
    {
        editingConnection->getBaseConnector()->removeMouseListener(this);
        delete editingConnection;
        editingConnection = nullptr;
    }
}

/*
 void NodeManagerUI::setNodeManager(NodeManager * nodeManager)
 {
 this->nodeManager = nodeManager;
 nodeManager->addListener(this);
 }
 */

void NodeManagerUI::clear()
{
    nodesUI.clear();
}

void NodeManagerUI::paint (Graphics& g)
{
    /* This demo code just fills the component's background and
     draws some placeholder text to get you started.

     You should replace everything in this method with your own
     drawing code..
     */

    g.fillAll (BG_COLOR);   // clear the background
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

}

void NodeManagerUI::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..


}

void NodeManagerUI::nodeAdded(NodeBase * node)
{
    addNodeUI(node);
}

void NodeManagerUI::nodeRemoved(NodeBase * node)
{
    removeNodeUI(node);
}


void NodeManagerUI::connectionAdded(NodeConnection * connection)
{
    addConnectionUI(connection);
}

void NodeManagerUI::connectionEdited(NodeConnection * connection)
{
    //do nothing ?
}

void NodeManagerUI::connectionRemoved(NodeConnection * connection)
{
    removeConnectionUI(connection);
}


void NodeManagerUI::addNodeUI(NodeBase * node)
{

    if (getUIForNode(node) == nullptr)
    {
        NodeBaseUI * nui = node->createUI();
        nodesUI.add(nui);
        addAndMakeVisible(nui);

    }
    else
    {
        //ui for this node already in list
    }
}



void NodeManagerUI::removeNodeUI(NodeBase * node)
{
    DBG("Remove NodeUI");
    NodeBaseUI * nui = getUIForNode(node);
    if (nui != nullptr)
    {
        nodesUI.removeObject(nui);
        removeChildComponent(nui);
    }
    else
    {
        //nodeBaseUI isn't in list
    }
}


NodeBaseUI * NodeManagerUI::getUIForNode(NodeBase * node)
{
    for (int i = nodesUI.size(); --i >= 0;)
    {
        NodeBaseUI * nui = nodesUI.getUnchecked(i);
        if (nui->node == node) return nui;
    }

    return nullptr;
}

void NodeManagerUI::addConnectionUI(NodeConnection * connection)
{
    DBG("NMUI :: addConnectionUI From NodeManagerListener");

    if (getUIForConnection(connection) != nullptr)
    {
        DBG("AddConnectionUI :: already exists");
        return;
    }

    NodeBaseUI * n1 = getUIForNode(connection->sourceNode);
    NodeBaseUI * n2 = getUIForNode(connection->destNode);

    ConnectorComponent * c1 = (n1 != nullptr) ? n1->getFirstConnector(connection->connectionType,ConnectorComponent::OUTPUT): nullptr;
    ConnectorComponent * c2 = (n2 != nullptr) ? n2->getFirstConnector(connection->connectionType,ConnectorComponent::INPUT) : nullptr;


    NodeConnectionUI * cui = new NodeConnectionUI(connection, c1,c2);
    connectionsUI.add(cui);

    DBG("Add And MakeVisible connection");
    addAndMakeVisible(cui,0);
}

void NodeManagerUI::removeConnectionUI(NodeConnection * connection)
{
    NodeConnectionUI * nui = getUIForConnection(connection);
    if (nui == nullptr)
    {
        DBG("RemoveConnectionUI :: not exists");
        return;
    }

    connectionsUI.removeObject(nui);
    removeChildComponent(nui);
}

NodeConnectionUI * NodeManagerUI::getUIForConnection(NodeConnection* connection)
{
    for (int i = connectionsUI.size(); --i >= 0;)
    {
        NodeConnectionUI * cui = connectionsUI.getUnchecked(i);
        if (cui->connection == connection) return cui;
    }

    return nullptr;
}


void NodeManagerUI::createDataConnectionFromConnector(Connector * baseConnector, const String &dataName, const String &elementName, DataProcessor::DataType dataType)
{

    DBG("Create Data connection from connector : " + dataName + ", " + elementName);

    if (editingConnection != nullptr)
    {
        DBG("Already editing a connection !");
        return;
    }

    bool isOutputConnector = baseConnector->ioType == Connector::ConnectorIOType::OUTPUT;


    if (isOutputConnector)
    {
        editingConnection = new NodeConnectionUI(nullptr, baseConnector, nullptr);
    }else
    {
        editingConnection = new NodeConnectionUI(nullptr, nullptr, baseConnector);
    }

    addAndMakeVisible(editingConnection);

    editingDataName = dataName;
    editingElementName = elementName;
    editingDataType = dataType;

    if (dataName == "" && elementName == "")
    {
        baseConnector->addMouseListener(this,false);
    }
    else
    {

    }
}

void NodeManagerUI::createAudioConnectionFromConnector(Connector * baseConnector, uint32 channel)
{
    DBG("Create Audio Connection from connector " + String(channel));

    if (editingConnection != nullptr)
    {
        DBG("Already editing a connection !");
        return;
    }

    bool isOutputConnector = baseConnector->ioType == Connector::ConnectorIOType::OUTPUT;

    if (isOutputConnector)
    {
        editingConnection = new NodeConnectionUI(nullptr, baseConnector, nullptr);
    }
    else
    {
        editingConnection = new NodeConnectionUI(nullptr, nullptr, baseConnector);
    }

    editingChannel = -1; //temp, will be able to select which channel later

    baseConnector->addMouseListener(this, false);

    addAndMakeVisible(editingConnection);
}

void NodeManagerUI::updateEditingConnection()
{
    if (editingConnection == nullptr) return;

    Point<int> cPos = getLocalPoint(editingConnection->getBaseConnector(), editingConnection->getBaseConnector()->getLocalBounds().getCentre());
    Point<int> mPos = getMouseXYRelative();
    int minX = jmin<int>(cPos.x, mPos.x);
    int minY = jmin<int>(cPos.y, mPos.y);
    int tw = abs(cPos.x - mPos.x);
    int th = abs(cPos.y - mPos.y);
    int margin = 50;

    checkDropCandidates();

    editingConnection->setBounds(minX-margin, minY-margin, tw+margin*2, th+margin*2);
}

bool NodeManagerUI::checkDropCandidates()
{
    Connector * candidate = nullptr;
    for (int i = 0; i < nodesUI.size(); i++)
    {
        Array<Connector *> compConnectors = nodesUI.getUnchecked(i)->getComplementaryConnectors(editingConnection->getBaseConnector());

        for (int j = 0; j < compConnectors.size(); j++)
        {
            Connector * c = compConnectors.getUnchecked(j);
            float dist = c->getMouseXYRelative().getDistanceFromOrigin();
            if (dist < 20)
            {
                candidate = c;
                setCandidateDropConnector(c);
                return true;
            }
        }
    }

    cancelCandidateDropConnector();
    return false;
}

bool NodeManagerUI::setCandidateDropConnector(Connector * connector)
{
    if (!isEditingConnection()) return false;

    bool result = editingConnection->setCandidateDropConnector(connector);
    editingConnection->candidateDropConnector->addMouseListener(this,false);
    return result;
}

void NodeManagerUI::cancelCandidateDropConnector()
{
    if (!isEditingConnection()) return;
    if(editingConnection->candidateDropConnector != nullptr) editingConnection->candidateDropConnector->removeMouseListener(this);
    editingConnection->cancelCandidateDropConnector();


}

void NodeManagerUI::finishEditingConnection()
{
    DBG("Finish Editing connection");
    if (!isEditingConnection()) return;

    bool isEditingDataOutput = editingConnection->getBaseConnector()->ioType == Connector::ConnectorIOType::OUTPUT;
    editingConnection->getBaseConnector()->removeMouseListener(this);
    if(editingConnection->candidateDropConnector != nullptr) editingConnection->candidateDropConnector->removeMouseListener(this);

    bool isDataConnection = editingConnection->getBaseConnector()->dataType == NodeConnection::ConnectionType::DATA;

    if (isDataConnection) //DATA
    {
        //Delete the editing connection
        String targetDataName = "";
        String targetElementName = "";
        DataProcessor::DataType targetDataType;

        if (editingDataName != "")
        {
            if(editingConnection->candidateDropConnector != nullptr) editingConnection->candidateDropConnector->selectDataAndElementPopup(targetDataName, targetElementName, targetDataType, editingDataType);
        }

        if (editingDataName == "" || targetDataName != "")
        {
            bool success = editingConnection->finishEditing();

            if (success)
            {
                NodeConnection * nc = nodeManager->addConnection(editingConnection->sourceConnector->node, editingConnection->destConnector->node, editingConnection->getBaseConnector()->dataType);
                if (isEditingDataOutput)
                {
                    nc->addDataGraphConnection(editingDataName, editingElementName, targetDataName, targetElementName);
                }
                else
                {
                    nc->addDataGraphConnection(targetDataName, targetElementName, editingDataName, editingElementName);
                }
            }
        }
    }
    else //AUDIO
    {
        bool success = editingConnection->finishEditing();

        //      int targetChannel = -1;//temp, will be able to edit channel later

        if (success)
        {
            //          NodeConnection * nc =
            nodeManager->addConnection(editingConnection->sourceConnector->node, editingConnection->destConnector->node, editingConnection->getBaseConnector()->dataType);

            //handled in Node Connection Constructor ?
            //            if(nc!=nullptr)
            //                nc->addAudioGraphConnection(editingChannel, targetChannel);
        }
    }

    removeChildComponent(editingConnection);
    delete editingConnection;
    editingConnection = nullptr;

}

void NodeManagerUI::createNodeFromIndexAtPos(int modalResult, Viewport * c,int maxRes)
{
    if (modalResult >= 1 && modalResult <= maxRes)
    {
        NodeBase * n = NodeManager::getInstance()->addNode((NodeFactory::NodeType)(modalResult - 1));

        Point<int> mousePos = c->getMouseXYRelative();
        n->xPosition->setValue( mousePos.x);
        n->yPosition->setValue( mousePos.y);
    }
}



//Interaction Events
void NodeManagerUI::mouseDown(const MouseEvent & event)
{
    if (event.eventComponent == this )
    {
        if (event.mods.isRightButtonDown())
        {

            PopupMenu   menu;//(new PopupMenu());
            ScopedPointer<PopupMenu> addNodeMenu(NodeFactory::getNodeTypesMenu(0));
            menu.addSubMenu("Add Node", *addNodeMenu);

            menu.show(0,0,0,0,ModalCallbackFunction::forComponent(&NodeManagerUI::createNodeFromIndexAtPos,(Viewport*)this,addNodeMenu->getNumItems()));
        }
        else
        {
            if (event.mods.isCtrlDown())
            {
                NodeBase * n = nodeManager->addNode(NodeFactory::NodeType::Dummy);
                n->xPosition->setValue( getMouseXYRelative().x);
                n->yPosition->setValue( getMouseXYRelative().y);
            }
            else{
                Point<int> mouse = getMouseXYRelative();
                selectingBounds.setTopLeftPosition(mouse.x,mouse.y);

            }
        }
    }

}

void NodeManagerUI::mouseMove(const MouseEvent & event)
{
    if (editingConnection != nullptr)
    {
        //DBG("NMUI mouse mouve while editing connection");
        updateEditingConnection();
    }
}

void NodeManagerUI::mouseDrag(const MouseEvent & event)
{
    if (editingConnection != nullptr)
    {
        if (event.eventComponent == editingConnection->getBaseConnector())
        {
            updateEditingConnection();
        }
    }
    else{

        if(isSelectingNodes){
            selectingBounds.setSize(std::abs(event.getDistanceFromDragStartX()),std::abs(event.getDistanceFromDragStartY()));
            selectingBounds.setTopLeftPosition(jmin(event.getMouseDownX(),getMouseXYRelative().x),
                                               jmin(event.getMouseDownY(),getMouseXYRelative().y));
            selectingBounds.toFront(false);
            checkSelected();
            repaint();
        }
        else{

            isSelectingNodes = true;
            selectingBounds.setVisible(true);
        }
    }
}

void NodeManagerUI::mouseUp(const MouseEvent & event)
{
    if (isEditingConnection())
    {
        finishEditingConnection();
    }

    isSelectingNodes = false;
    selectingBounds.setVisible(false);
    selectingBounds.setSize(0, 0);
}


void NodeManagerUI::checkSelected(){
    // multiple ones
    if(isSelectingNodes){
        Array<SelectableComponent*> currentOnes;
        for(auto &n:nodesUI){
            if(selectingBounds.getBounds().intersects(n->getBounds())){
                currentOnes.add(n);
            }
        }
        for(auto &n:currentOnes){
            if(!selectableHandler.selected.contains(n)){
                n->askForSelection(true,false);
            }
        }
        for(auto &n:selectableHandler.selected){
            if(!currentOnes.contains(n)){
                n->askForSelection(false,false);
            }
        }

    }
    // only one

}

void NodeManagerUI::setAllNodesToStartAtZero(){
    if(nodesUI.size()==0)return;

    Point<int> minPoint = nodesUI.getUnchecked(0)->getBounds().getTopLeft();
    for(auto &n:nodesUI){
        minPoint.x = jmin(n->getX(),minPoint.x);
        minPoint.y = jmin(n->getY(),minPoint.y);
    }

    for(auto &n:nodesUI){
        n->setTopLeftPosition(n->getX()-(minPoint.x<0?minPoint.x:0),
                              n->getY()-(minPoint.y<0?minPoint.y:0));
    }

}
void NodeManagerUI::childBoundsChanged(Component * ){
    resizeToFitNodes();
}


void NodeManagerUI::resizeToFitNodes(){
    Rectangle<int> bounds = minBounds;
    for(auto &n:nodesUI){
        Rectangle<int> r = n->getBoundsInParent();
        bounds = bounds.getUnion(r);
    }
    setBounds(bounds);

}
