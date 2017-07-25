/*
 ==============================================================================

 BufferBlockList.cpp
 Created: 15 Jul 2017 3:05:20pm
 Author:  Martin Hermant

 ==============================================================================
 */

#include "BufferBlockList.h"



BufferBlockList::BufferBlockList(int _numChannels,int  _minNumSample ,int _blockSize):bufferBlockSize(_blockSize),minNumSample(_minNumSample){
  jassert(bufferBlockSize>0);
  allocateSamples(_numChannels, minNumSample);

}

void BufferBlockList::allocateSamples(int numChannels,int numSamples){
  jassert(numChannels>0);
//  jassert(numSamples>0);
  if(size()>0 && getReference(0).getNumChannels()!=numChannels){
    for(int i = 0 ; i < size() ; i++){
      getReference(i).setSize(numChannels, bufferBlockSize);
    }
  }
  int i = size() * bufferBlockSize;

  numSamples = jmax(numSamples,minNumSample);
  while (i <= numSamples){
    add(AudioSampleBuffer(numChannels,bufferBlockSize));
    i+=bufferBlockSize;
  }

  while(i>numSamples+bufferBlockSize){
    removeLast();
    i-=bufferBlockSize;

  }



}

void BufferBlockList::setNumChannels(int numChannels){
  for(int i = 0 ; i < size() ; i++){
    getReference(i).setSize(numChannels, bufferBlockSize);
  }
}
void BufferBlockList::setNumSample(int numSamples){
  allocateSamples(getAllocatedNumChannels(), numSamples);
  targetNumSamples = numSamples;
}
int BufferBlockList::getNumSamples() const{
  return targetNumSamples;
}
int BufferBlockList::getAllocatedNumSample()const {
  return size()*bufferBlockSize;
}
int BufferBlockList::getAllocatedNumChannels()const{
  return size()>0?getReference(0).getNumChannels():0;
}
void BufferBlockList::copyTo(AudioSampleBuffer & outBuf,int listStartSample,int bufStartSample,int numSampleToCopy){
  if(numSampleToCopy==-1) numSampleToCopy = outBuf.getNumSamples();
  jassert(numSampleToCopy <= size()*bufferBlockSize);
  int numChannels = jmin(getAllocatedNumChannels(),outBuf.getNumChannels());
  int sampleProcessed = 0;
  int readPosInList = listStartSample;
  int readBlockIdx;
  int writePos = bufStartSample;

  while(sampleProcessed < numSampleToCopy){
    readBlockIdx=floor(readPosInList*1.0/bufferBlockSize);
    int readPosInBlock = (readPosInList%targetNumSamples)%bufferBlockSize ;

    int blockSize =  bufferBlockSize-readPosInBlock;
    if(sampleProcessed+ blockSize>numSampleToCopy)
      blockSize =  numSampleToCopy-sampleProcessed;

    jassert(blockSize>0);
    jassert(readPosInBlock + blockSize<=bufferBlockSize);
    jassert(readBlockIdx < size());
    for (int i = 0 ; i < numChannels ; i++){
      outBuf.copyFrom( i, writePos, getReference(readBlockIdx),i, readPosInBlock, blockSize);
    }
    readPosInList+=blockSize;
    writePos+=blockSize;
    sampleProcessed+=blockSize;

    if(readPosInList>=targetNumSamples){
      jassert(readPosInList<targetNumSamples+bufferBlockSize);
      readPosInList%=targetNumSamples;
      readBlockIdx = 0;
    }



  }

}

void BufferBlockList::copyFrom(const AudioSampleBuffer & inBuf,int listStartSample,int bufStartSample,int numSampleToCopy){
  if (numSampleToCopy==-1)numSampleToCopy =  inBuf.getNumSamples();
  jassert(listStartSample+numSampleToCopy< getAllocatedNumSample());
  jassert(inBuf.getNumChannels()==getAllocatedNumChannels());
  int numChannels = inBuf.getNumChannels();
  int sampleProcessed = 0;
  int readPos = bufStartSample;

  int writeBlockIdx ;
  int writePosInList = listStartSample;

  while(sampleProcessed < numSampleToCopy){
    writeBlockIdx=floor(writePosInList*1.0/bufferBlockSize);
    int startWrite = writePosInList%bufferBlockSize ;
    int blockSize =  bufferBlockSize-startWrite;
    if(sampleProcessed+ blockSize>numSampleToCopy)
      blockSize =  numSampleToCopy-sampleProcessed;
    jassert(blockSize>0);
    jassert(startWrite + blockSize<=bufferBlockSize);
    jassert(writeBlockIdx < size());
    for (int i = 0 ; i < numChannels ; i++){
      getReference(writeBlockIdx).copyFrom(i, startWrite, inBuf, i, readPos, blockSize);

    }
    readPos+=blockSize;
    writePosInList+=blockSize;
    sampleProcessed+=blockSize;

    jassert(writePosInList<getAllocatedNumSample());

  }
}

float BufferBlockList::getSample(int c, int n){
  int readI = n%bufferBlockSize ;
  int readBI=floor(n*1.0/bufferBlockSize);
  jassert(readBI<size());
  return getReference(readBI).getSample(c,readI);
}


AudioSampleBuffer & BufferBlockList::getContiguousBuffer(int sampleStart,int numSamples){
  if(numSamples==-1)numSamples = getNumSamples()-sampleStart;
  contiguous_Cache.setSize(getAllocatedNumChannels(), numSamples);
  copyTo(contiguous_Cache,sampleStart,0,numSamples);
  return contiguous_Cache;
}