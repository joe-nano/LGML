/*
  ==============================================================================

    JavascriptEnvironnement.h
    Created: 5 May 2016 9:03:35am
    Author:  Martin Hermant

  ==============================================================================
*/

#ifndef JAVASCRIPTENVIRONNEMENT_H_INCLUDED
#define JAVASCRIPTENVIRONNEMENT_H_INCLUDED


#include "JuceHeader.h"
#include "ControllableContainer.h"
class JavascriptEnvironment : public JavascriptEngine,public ControllableContainer::Listener{
public:
    JavascriptEnvironment();

    juce_DeclareSingleton(JavascriptEnvironment, true);

    typedef juce::var::NativeFunctionArgs NativeFunctionArgs;


    static  DynamicObject *  createDynamicObjectFromContainer(ControllableContainer * c,DynamicObject * parent);
    void    linkToControllableContainer(const String & jsNamespace,ControllableContainer * c);
    void    removeNamespace(const String & jsNamespace);
    void    loadFile(const String & path);

    class OwnedJsArgs {


    public:
        OwnedJsArgs(DynamicObject * _scope):scope(_scope){}
        void addArg(float f){ownedArgs.add(new var(f));}
        void addArg(String f){ownedArgs.add(new var(f));}
        void addArgs(const StringArray & a){for(auto & s:a){addArg(s.getFloatValue());}}

        NativeFunctionArgs getNativeArgs(){
            return NativeFunctionArgs(scope,ownedArgs.getFirst(),ownedArgs.size());
        }
    private:
        DynamicObject * scope;
        OwnedArray<var> ownedArgs;
    };



    static var post(const NativeFunctionArgs& a);
    static var set(const NativeFunctionArgs& a);

    ReferenceCountedObjectPtr<DynamicObject> localEnvironment;

    class JsNamespace{
    public:
        JsNamespace(String n,ControllableContainer * c,DynamicObject * o):name(n),container(c),jsObject(o){}
        String name;
        WeakReference<ControllableContainer> container;
        DynamicObject::Ptr jsObject;

    };
    OwnedArray<JsNamespace> linkedNamespaces;


    StringArray loadedFiles;
    void rebuildAllNamespaces();

private:

    void internalLoadFile(const File &);
    void structureChanged(ControllableContainer *)override;


};




#endif  // JAVASCRIPTENVIRONNEMENT_H_INCLUDED
