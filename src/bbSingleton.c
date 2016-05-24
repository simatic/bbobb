/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   bbSingleton.c
 * Author: terag
 *
 * Created on May 5, 2016, 6:17 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stddef.h>
#include "bbSingleton.h"
#include "bbComm.h"

BbSingleton bbSingleton;

int bbSingletonInit() {
    int i;
    int rc;
    BbSharedMsg *set = newBbSharedMsg(offsetof(BbSharedMsg,msg.body.set.batches)+bbSingleton.batchMaxLen);
    
    bbSingleton.initDone = false;
    
    char * port = bbGetLocalPort();
    bbSingleton.commForAccept = commNewForAccept(port);
    for (i = 0 ; i < MAX_MEMB ; ++i) {
        bbSingleton.commToViewMembers[i] = NULL;
    }
    bbSingleton.msgQueue = newBqueue();
    
    bbSingleton.currentWave = 0;
    bbSingleton.currentStep = 0;
    
    bbSingleton.batchToSend = newBatchInSharedMsg(set->msg.body.set.batches, set);
    deleteBbSharedMsg(set);
    set = NULL;
    bbSingleton.batchToSend->batch->sender = bbSingleton.myAddress;
    bbSingleton.batchToSend->batch->len = sizeof(BbBatch);

    pthread_mutex_init(&(bbSingleton.batchToSendMutex), NULL );

    rc= pthread_cond_init(&(bbSingleton.batchToSendCond), NULL);
    assert(rc == 0);

    bbSingleton.batchesToDeliver = newBqueue();
    
    return 0;
}