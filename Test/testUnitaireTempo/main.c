/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: terag
 *
 * Created on May 16, 2016, 6:09 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "bbSharedMsg.h"
#include "bbMsg.h"
#include "list.h"

size_t tabSizeMessages[] = {
    73,
    89,
    127,
    191,
    229,
    251
};

#define NB_MESSAGES (sizeof(tabSizeMessages)/sizeof(tabSizeMessages[0]))

BbBatch *newBatchWithOneMessage(size_t sizePayload){
    assert(sizePayload < 256);
    size_t len = offsetof(BbBatch,messages[0].payload) + sizePayload;
    BbBatch *pBatch = malloc(len);
    assert(pBatch != NULL);
    pBatch->len = len;
    pBatch->sender = sizePayload;
    pBatch->messages[0].header.len = offsetof(message,payload) + sizePayload;
    pBatch->messages[0].header.typ = sizePayload;
    memset(pBatch->messages[0].payload, sizePayload, sizePayload);
    return pBatch;
}

/*
 * 
 */
int main(int argc, char** argv) {
    BbBatch* tabPBatches[NB_MESSAGES] = {NULL};
    int i;
    trList *listBatchInSharedMsg = newList();

    ///////////////////////////////
    // Test on a set which contains no batch
    ///////////////////////////////
    int lenSet = offsetof(BbMsg,body.set.batches);
    BbSharedMsg *pSet = newBbSharedMsg(lenSet);
    pSet->msg.len = lenSet;
    pSet->msg.type = BB_MSG_SET;
    pSet->msg.body.set.step = 5;
    pSet->msg.body.set.viewId = 7;
    pSet->msg.body.set.wave = 11;
    BbBatchInSharedMsg *pBatchInSharedMsg = getBatchInSharedMsg(pSet, NULL, 0);
    assert(pBatchInSharedMsg == NULL);
    deleteBbSharedMsg(pSet);
    
    ///////////////////////////////
    // Test on a set which contains batches
    ///////////////////////////////    
    lenSet = offsetof(BbMsg,body.set.batches);
    for (i = 0 ; i < NB_MESSAGES ; ++i) {
        tabPBatches[i] = newBatchWithOneMessage(tabSizeMessages[i]);
        lenSet += tabPBatches[i]->len;
    }
    
    pSet = newBbSharedMsg(lenSet);
    pSet->msg.len = lenSet;
    pSet->msg.type = BB_MSG_SET;
    pSet->msg.body.set.step = 5;
    pSet->msg.body.set.viewId = 7;
    pSet->msg.body.set.wave = 11;

    BbBatch *pBatch = pSet->msg.body.set.batches;
    for (i = 0 ; i < NB_MESSAGES ; ++i) {
        memcpy(pBatch, tabPBatches[i], tabPBatches[i]->len);
        pBatch = (BbBatch*)(((char*)pBatch) + tabPBatches[i]->len);
    }
    
    for (i = 0 , pBatchInSharedMsg = getBatchInSharedMsg(pSet, NULL, 0) ;
         pBatchInSharedMsg != NULL ;
         ++i , pBatchInSharedMsg = getBatchInSharedMsg(pSet, pBatchInSharedMsg, 0) ) {
        assert(i < NB_MESSAGES);
        listAppend(listBatchInSharedMsg, pBatchInSharedMsg);
        if (memcmp(tabPBatches[i], pBatchInSharedMsg->batch, tabPBatches[i]->len) != 0) {
            fprintf(stderr, "ERROR : Difference found for %d-th batch in pSet\n", i);
            return EXIT_FAILURE;
        }
    }
    
    deleteBbSharedMsg(pSet);
    
    ///////////////////////////////
    // Test on a Recover which contains no batch
    ///////////////////////////////
    int lenRecover = offsetof(BbMsg,body.recover.sets);
    BbSharedMsg *pRecover = newBbSharedMsg(lenRecover);
    pRecover->msg.len = lenRecover;
    pRecover->msg.type = BB_MSG_RECOVER;
    pRecover->msg.body.recover.initDone = false;
    pRecover->msg.body.recover.nbSets = 0;
    memset(&(pRecover->msg.body.recover.view), 0, sizeof(pRecover->msg.body.recover.view));
    pRecover->msg.body.recover.viewId = 5;
    pBatchInSharedMsg = getBatchInSharedMsg(pRecover, NULL, 0);
    assert(pBatchInSharedMsg == NULL);
    deleteBbSharedMsg(pRecover);

    ///////////////////////////////
    // Free allocated memory
    ///////////////////////////////
    for(pBatchInSharedMsg = listRemoveFirst(listBatchInSharedMsg) ;
            pBatchInSharedMsg != NULL ;
            pBatchInSharedMsg = listRemoveFirst(listBatchInSharedMsg) ) {
        deleteBatchInSharedMsg(pBatchInSharedMsg);
    }
    
    freeList(listBatchInSharedMsg);

    for (i = 0 ; i < NB_MESSAGES ; ++i) {
        free(tabPBatches[i]);
    }
    
    return (EXIT_SUCCESS);
}

