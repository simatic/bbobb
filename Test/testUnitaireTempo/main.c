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

size_t tabSizeMessages[] = {
    89,
    127,
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
    int lenSet = offsetof(BbMsg,body.set.batches);
    for (i = 0 ; i < NB_MESSAGES ; ++i) {
        tabPBatches[i] = newBatchWithOneMessage(tabSizeMessages[i]);
        lenSet += tabPBatches[i]->len;
    }
    
    BbSharedMsg *pSet = newBbSharedMsg(lenSet);
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
    
    BbBatchInSharedMsg * pBatchInSharedMsg;
    for (i = 0 , pBatchInSharedMsg = getBatchInSharedMsg(pSet, NULL, 0) ;
         pBatchInSharedMsg != NULL ;
         ++i , pBatchInSharedMsg = getBatchInSharedMsg(pSet, pBatchInSharedMsg, 0) ) {
        assert(i < NB_MESSAGES);
        if (memcmp(tabPBatches[i], pBatchInSharedMsg->batch, tabPBatches[i]->len) != 0) {
            fprintf(stderr, "ERROR : Difference found for %d-th batch in pSet\n", i);
            return EXIT_FAILURE;
        }
        //deleteBatchInSharedMsg(pBatchInSharedMsg);
    }
    
    deleteBbSharedMsg(pSet);
    
    return (EXIT_SUCCESS);
}

