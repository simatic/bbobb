/**
* @brief File holding the types' definition for the messages exchanged between processes
* @file bbMsg.c
* @author May CATTANT - Quentin MONNOT - Victor ROUQUETTE - Michel SIMATIC
* @date 23 february 2016
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/uio.h>
#include <stddef.h>
#include "bbMsg.h"
#include "address.h"
#include "bbSingleton.h"
#include "bbStateMachine.h"
#include "bbError.h"
#include "bbSharedMsg.h"

/*
BbBatch initBatch () {
    BbBatch batch;
    
    batch.len=sizeof(batch);   
    //batch.sender;  //A DEFINIR valeur par defaut
    //batch.messages[]=NULL; 
    return batch;
}


BbBatch newBbBatch (address sender, message messages[]) {
    BbBatch batch=initBatch();
    batch.len=sizeof(int)+sizeof(address)+sizeof(messages);   
    batch.sender= sender;
    batch.messages=messages[]; 
    
    return batch;
}
*/
address bbAddrPrec(address ad);

BbSharedMsg * createSet(int waveNum) {
    BbSharedMsg * set = NULL;
    
    int lenOfSet;
    int lenOfBatches;
    int processIndex;
    
    for(processIndex=0; processIndex<MAX_MEMB; processIndex++) {
        if(rcvdBatch[waveNum][processIndex] != NULL) {
            lenOfBatches += rcvdBatch[waveNum][processIndex]->batch->len;
        }
    }
    
    lenOfSet = offsetof(BbSharedMsg, msg.body.set.batches) + lenOfBatches;
    set = newBbSharedMsg(lenOfSet);
    
    //We fill the fields of set
    set->msg.len = lenOfSet;
    set->msg.type = BB_MSG_SET;
    set->msg.body.set.viewId = bbSingleton.viewId;
    set->msg.body.set.wave = waveNum;
    set->msg.body.set.step = 0; //unused in case of RECOVER messages
    
    //We fill the batches of the set
    lenOfBatches = 0;
    for(processIndex=0; processIndex<MAX_MEMB; processIndex++) {
        if(rcvdBatch[waveNum][processIndex] != NULL) {
            memcpy((char*)&(set->msg.body.set.batches)+lenOfBatches, rcvdBatch[waveNum][processIndex], rcvdBatch[waveNum][processIndex]->batch->len);
            lenOfBatches += rcvdBatch[waveNum][processIndex]->batch->len;
        }
    }
    
    return set;
}

BbSharedMsg buildNewSet() {
    BbSharedMsg * newSet;
    struct iovec iov[MAX_MEMB];
    int iovcnt = 0;
    int senderBatchToAdd = 0;
    int i;
    int nbBatchesToAdd = ((1 << bbSingleton.currentStep) < bbSingleton.view.cv_nmemb - (1 << bbSingleton.currentStep) ?
                        1 << bbSingleton.currentStep :
                        bbSingleton.view.cv_nmemb - (1 << bbSingleton.currentStep));
    
    newSet = newBbSharedMsg();
    newSet->msg.type = BB_MSG_SET;
    newSet->msg.body.set.viewId = bbSingleton.viewId;
    newSet->msg.body.set.wave = bbSingleton.currentWave;
    newSet->msg.body.set.step = bbSingleton.currentStep;
    
    iov[iovcnt].iov_base = &newSet;
    iov[iovcnt].iov_len = offsetof(BbSharedMsg, msg.body.set.batches);
    newSet->msg.len = iov[iovcnt].iov_len;
    iovcnt++;
    
    int rank;
    for(i=0, senderBatchToAdd; i < nbBatchesToAdd ; i++, senderBatchToAdd = bbAddrPrec(senderBatchToAdd)) {
        rank = addrToRank(senderBatchToAdd);
        if(rcvdBatch[rank][bbSingleton.currentWave] != NULL) {
            iov[iovcnt].iov_base = rcvdBatch[rank][bbSingleton.currentWave];
            iov[iovcnt].iov_len = rcvdBatch[rank][bbSingleton.currentWave]->batch->len;
            newSet->msg.len += iov[iovcnt].iov_len;
            iovcnt++;
        }
    }
    
    return newSet;
}

message *firstMsgInBatch(BbBatch *b){
  if (b->len == sizeof(BbBatch))
    return NULL;
  else
    return b->messages;
}

message *nextMsgInBatch(BbBatch *b, message *mp){
  message *mp2 = (message*)((char*)mp + mp->header.len);
  if ((char*)mp2 - (char*)b >= b->len)
    return NULL;
  else
    return mp2;
}

address bbAddrPrec(address ad) {
    int i = 0;
    for(i=0; addrIsEqual(ad, bbSingleton.view.cv_members[i]) && i < MAX_MEMB + 1; i++) {
        //find pos of ad in current view
    }
    if(i==MAX_MEMB) {
        bbErrorAtLineWithoutErrnum(EXIT_FAILURE,
                                   error,
                                   __FILE__,
                                   __LINE__,
                                   "error to find address of predecessor, address didn't found");
    }
    
    if(i==0) {
        return bbSingleton.view.cv_members[bbSingleton.view.cv_nmemb - 1];
    } else {
        return bbSingleton.view.cv_members[i - 1];
    }
}