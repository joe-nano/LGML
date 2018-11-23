/*
 ==============================================================================

 PdNode.cpp
 Created: 20 Nov 2018 2:41:30am
 Author:  Martin Hermant

 ==============================================================================
 */

#include "PdNode.h"
#include "../../Engine.h"

#define DEFDACBLKSIZE 64
REGISTER_NODE_TYPE (PdNode)


static void lgml_print_hook(const char * m){
    LOGW(m);
}


StringArray getSearchPaths(){

    auto savedV = getAppProperties()->getUserSettings()->getValue("PureDataExternalPaths");
    if(savedV.isEmpty() ){
        static String defaultSearchPath =
#if JUCE_MAC
        "/Applications/Pd-extended.app/Contents/Resources/extra/"
#else
        ""
#endif
        ;
        if(!defaultSearchPath.isEmpty()){
            if(File(defaultSearchPath).exists()){
                getAppProperties()->getUserSettings()->setValue("PureDataExternalPaths",defaultSearchPath);
            }
        }
    }
    StringArray paths  ;
    paths.addTokens(getAppProperties()->getUserSettings()->getValue("PureDataExternalPaths"), ",\n","\"");
    return paths;
}

PdNode::PdNode (StringRef name) :
NodeBase (name),
patchHandle(NULL),
pdinstance(NULL),
midiChooser(this,false,true)
{

    pdPath = addNewParameter<StringParameter>("pd path", "path where to load pd Vanilla file , if a patch contain a subpatch named gui graphical object with send names will be added as parameters", "");
    isLoadedParam = addNewParameter<BoolParameter>("is loaded", "is file loaded", false);
    isLoadedParam->isEditable=false;
    reloadPatch = addNewParameter<Trigger>("reload Patch","reload current patch");
    midiActivityTrigger= addNewParameter<Trigger>("midi trigger", "trigger when incomming midi messages");
    if(libpd_init()==0){
        libpd_set_verbose(999);
        libpd_set_printhook(lgml_print_hook);
    }
    pdinstance = libpd_new_instance();
    jassert(pdinstance!=NULL);
    libpd_set_instance(pdinstance);
    setPreferedNumAudioInput(2);
    setPreferedNumAudioOutput(2);
    auto paths = getSearchPaths();
    if(paths.size()){
        for(auto & p : paths){
            libpd_add_to_search_path(p.toRawUTF8());
        }

    }
    if(getEngine()->getFile().exists()){
        libpd_add_to_search_path(getEngine()->getFile().getFullPathName().toRawUTF8());
    }
    //    numChannelsChanged(true);
    //    numChannelsChanged(false);
}


PdNode::~PdNode(){
    libpd_free_instance(pdinstance);
}
void PdNode::numChannelsChanged (bool isInput) {
    libpd_set_instance(pdinstance);
    //    if(getSampleRate()!=0);
    libpd_init_audio(getTotalNumInputChannels(),getTotalNumOutputChannels(),getSampleRate());
    jassert(getBlockSize()%DEFDACBLKSIZE==0);
    numTicks =0;// getBlockSize() / DEFDACBLKSIZE; //DEFDACBLKSIZE in libpd
    // [; pd dsp $1(



    libpd_start_message(1);
    libpd_add_float(true);
    libpd_finish_message("pd", "dsp");
}

void PdNode::prepareToPlay(double sr, int blk) {
    jassert(sr>0);
    messageCollector.reset(sr);
    libpd_init_audio(getTotalNumInputChannels(),getTotalNumOutputChannels(),sr);


    numTicks = blk / DEFDACBLKSIZE;
    jassert(blk%DEFDACBLKSIZE==0 && blk>DEFDACBLKSIZE);
    tempInBuf = HeapBlock<float>(blk*getTotalNumInputChannels());
    tempOutBuf = HeapBlock<float>(blk*getTotalNumOutputChannels(),true);
    libpd_init_audio(getTotalNumInputChannels(),getTotalNumOutputChannels(),getSampleRate());
}


void PdNode::processBlockInternal (AudioBuffer<float>& buffer, MidiBuffer& incomingMidi){
    if(!patchHandle){ buffer.clear();return;}
    libpd_set_instance(pdinstance);
    //    jassert(getTotalNumOutputChannels()<=buffer.getNumChannels() && getTotalNumInputChannels()<=buffer.getNumChannels());

    jassert(sys_getsr()==getSampleRate());
    jassert(sys_get_outchannels()==getTotalNumOutputChannels());
    jassert(sys_get_inchannels()==getTotalNumInputChannels());
    jassert(buffer.getNumSamples()/libpd_blocksize()==numTicks);



    messageCollector.removeNextBlockOfMessages (incomingMidi, buffer.getNumSamples());
    MidiBuffer::Iterator it(incomingMidi);
    int pos;
    MidiMessage message;
    while(it.getNextEvent(message,pos)){
        if(message.isNoteOnOrOff()){
            libpd_noteon(message.getChannel(), message.getNoteNumber(), message.isNoteOn()?message.getVelocity():0);
        }
        else if(message.isController()){
            libpd_controlchange(message.getChannel(), message.getControllerNumber(), message.getControllerValue());
        }
    }
    AudioDataConverters::interleaveSamples(buffer.getArrayOfReadPointers(), tempInBuf, buffer.getNumSamples(),getTotalNumInputChannels());
    jassert( libpd_process_float(numTicks, tempInBuf, tempOutBuf)==0);
    AudioDataConverters::deinterleaveSamples(tempOutBuf, buffer.getArrayOfWritePointers(), buffer.getNumSamples(), getTotalNumOutputChannels());
}

void PdNode::onContainerTriggerTriggered(Trigger *t){
    if(t==reloadPatch){
        loadValidPdFile();
    }
    NodeBase::onContainerTriggerTriggered(t);
}

void PdNode::onContainerParameterChanged ( ParameterBase* p) {
    if(p==pdPath){
        pdPath->setValueFrom(this,getEngine()->getNormalizedFilePath(File(pdPath->stringValue())));

        loadValidPdFile();
    }

    else{
        auto idx = pdParameters.indexOf(p);
        if(idx!=-1){
            if(auto fp = p->getAs<FloatParameter>()){
                libpd_float(p->niceName.toRawUTF8(), fp->floatValue());
            }
            else if(auto sp = p->getAs<StringParameter>()){
                libpd_symbol(p->niceName.toRawUTF8(), sp->stringValue().toRawUTF8());
            }
        }
    }
    NodeBase::onContainerParameterChanged (p);
}

void PdNode::unloadFile(){
    if(patchHandle!=NULL){
        libpd_closefile(patchHandle);
        patchHandle = NULL;
        dollarZero = -1;
        isLoadedParam->setValueFrom(this,false);
    }

    //    tempOutBuf.clear(numTicks*DEFDACBLKSIZE*getTotalNumOutputChannels());

}

bool PdNode::isLoaded(){
    return patchHandle!=NULL;
}

void PdNode::loadValidPdFile(){
    auto p = pdPath->stringValue();

    bool shouldLoad = true;
    if(!p.endsWith(".pd")){
        LOGE(juce::translate("file should have pd extension"));
        shouldLoad = false;

    }
    if(p.startsWith("/")){
        LOGW("warning pd file path is absolute, the reference may unresolved if lgml project is transfered on other computer");
    }
    libpd_set_instance(pdinstance);

    unloadFile();
    if(!p.isEmpty() && shouldLoad){
        auto fileName = File(pdPath->stringValue()).getFileName();
        auto dirName = File(pdPath->stringValue()).getParentDirectory().getFullPathName();
        patchHandle = libpd_openfile(fileName.toRawUTF8(), dirName.toRawUTF8());
        if(patchHandle==NULL){
            LOGE("can't open patch at "<<pdPath->stringValue());
        }

        parseParameters();


        dollarZero = libpd_getdollarzero(patchHandle);
    }
    isLoadedParam->setValueFrom(this,patchHandle!=NULL);


}


void PdNode::parseParameters(){
    int canvasDepth = -1;
    for(auto p :pdParameters){removeControllable(p);}
    pdParameters.clear();
    if(!isLoaded()){return;}
    StringArray lines;

    FileInputStream file(File(pdPath->stringValue()));
    char c = file.readByte();
    String l;
    StringArray sp;
    bool jumpToNextLine = false;
    while(c){
        if(canvasDepth==-1 && !jumpToNextLine && sp.size()==1 && sp[0]!="#N" && sp[0]!="#X"){
            jumpToNextLine=true;
        }
        if(!jumpToNextLine ){
            if(c==' ' || c==';' || c=='\n'){
                l = l.trim();
                if(!l.isEmpty()){
                    sp.add(l);
                    l="";
                }
            }
            else{
                l+=c;
            }
        }


        if(c==';'){
            jumpToNextLine = false;
            if(sp.size()>6){
                if(sp[0]=="#N" && sp[1]=="canvas"){
                    auto cname = sp[6];
                    if(canvasDepth==-1 ){
                        if(cname=="gui")canvasDepth=1;
                    }
                    else{canvasDepth++;}
                }
                if(canvasDepth>=1){
                    if( sp[0]=="#X"){
                        if(sp[1]=="restore"){canvasDepth--;}

                        else if(sp[1]=="obj" && sp.size()>4){
                            auto type = sp[4];
                            if(sp.size()>6){
                                // auto width = sp[5].getIntValue();
                                // auto height = sp[6].getIntValue();
                                if (type=="s"){
                                    auto sname = sp[5];
                                    if(sname!="empty" && !getControllableByName(sname)){
                                        pdParameters.add(addNewParameter<StringParameter>(sname,"pd param "+sname));
                                    }
                                }
                            }
                            if(sp.size()>17){
                                if(type=="tgl"){
                                    auto sname = sp[7];auto label = sp[9];if(label=="empty")label=sname;
                                    if(sname!="empty" && !getControllableByName(sname)){
                                        pdParameters.add(addNewParameter<BoolParameter>(sname,"pd param "+sname,sp[17]=="1"));

                                    }
                                }
                            }
                            if(sp.size()>11){
                                if(type=="bng"){
                                    auto sname = sp[9];auto label = sp[11];if(label=="empty")label=sname;
                                    if(sname!="empty" && !getControllableByName(sname)){pdParameters.add(addNewParameter<Trigger>(sname,"pd param "+label));}
                                }
                            }

                            if(sp.size()>21){
                                if(type=="vsl" || type=="hsl" || type=="nbx"){

                                    auto min = sp[7].getFloatValue();
                                    auto max = sp[8].getFloatValue();
                                    auto sname = sp[11];
                                    // auto rname = sp[12];
                                    auto label = sp[13];
                                    if(label=="empty")label=sname;

                                    float value = sp[21].getFloatValue();
                                    if(type!="nbx"){value/=12700.0;}
                                    min = jmax(-1e5f,min);
                                    max = jmin(1e5f,min);
                                    if(sname!="empty" &&!getControllableByName(sname)){
                                        auto fp = addNewParameter<FloatParameter>(sname, "pd float param "+label, 0,min,max);
                                        pdParameters.add(fp);
                                        fp->setNormalizedValue(value);
                                    }
                                }

                            }
                            if(sp.size()>19){
                                if(type=="hradio" || type=="vradio"){
                                    auto min = sp[7].getIntValue();
                                    auto max = sp[8].getIntValue();
                                    auto sname = sp[9];
                                    // auto rname = sp[12];
                                    auto label = sp[11];
                                    if(label=="empty")label=sname;
                                    int value = sp[19].getIntValue();
                                    if(sname!="empty" && !getControllableByName(sname)){
                                        pdParameters.add(addNewParameter<IntParameter>(sname, "pd int param "+label, value,min,max));


                                    }
                                }

                            }
                        }
                    }
                }
                else if(canvasDepth==0){
                    break;
                }

            }
            sp.clear();

        }
        c = file.readByte();
    }
    
}


void PdNode::handleIncomingMidiMessage (MidiInput*,
                                        const MidiMessage& message)
{
    if (isLoaded())
    {
        messageCollector.addMessageToQueue (message);
        
    }
    
    midiActivityTrigger->trigger();
};
