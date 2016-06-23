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
#include "bbSingleton.h"
#include "bbStateMachine.h"
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

static BbBatch* tabPBatches[NB_MESSAGES] = {NULL};
static trList *listBatchInSharedMsg = NULL;
    
enum {
    TEST_RECOVER_WHEN_INITDONE_FALSE,
    TEST_RECOVER_BOTH_SETS_EMPTY,
    TEST_RECOVER_FIRST_SET_EMPTY,
    TEST_RECOVER_SECOND_SET_EMPTY,
    TEST_RECOVER_NON_EMPTY_SETS
} doneTest;

message *allocateRecoverForTest(int payloadSize) {
    int len = offsetof(message,payload) + payloadSize;
    message *mp = malloc(len);
    assert(mp != NULL);
    mp->header.len = len;
    return mp;
}

int testBuiltRecover(t_typ messageTyp, message *mp) {
    int len = payloadSize(mp);
    BbSharedMsg *pRecover = newBbSharedMsg(len);
    BbBatchInSharedMsg *pBatchInSharedMsg = NULL;
    int i;

    memcpy(&(pRecover->msg), &(mp->payload), len);
    
    if (memcmp(&(pRecover->msg.body.recover.view), &(bbSingleton.view), sizeof(bbSingleton.view)) != 0) {
        fprintf(stderr, "ERROR : Incorrect view in RECOVER message\n");
        abort();
    }
    assert(pRecover->msg.body.recover.viewId == 43);

    switch(doneTest){
        case TEST_RECOVER_WHEN_INITDONE_FALSE:
            assert(pRecover->msg.body.recover.initDone == false);
            pBatchInSharedMsg = getBatchInSharedMsg(pRecover, NULL, 0);
            assert(pBatchInSharedMsg == NULL);
            break;
        case TEST_RECOVER_BOTH_SETS_EMPTY:
            assert(pRecover->msg.body.recover.initDone == true);
            pBatchInSharedMsg = getBatchInSharedMsg(pRecover, NULL, 0);
            assert(pBatchInSharedMsg == NULL);
            pBatchInSharedMsg = getBatchInSharedMsg(pRecover, NULL, 1);
            assert(pBatchInSharedMsg == NULL);
            break;
        case TEST_RECOVER_FIRST_SET_EMPTY:
            assert(pRecover->msg.body.recover.initDone == true);
            pBatchInSharedMsg = getBatchInSharedMsg(pRecover, NULL, 0);
            assert(pBatchInSharedMsg == NULL);
            pBatchInSharedMsg = getBatchInSharedMsg(pRecover, NULL, 1);
            assert(pBatchInSharedMsg != NULL);
            listAppend(listBatchInSharedMsg, pBatchInSharedMsg);
            for (i = 0, pBatchInSharedMsg = getBatchInSharedMsg(pRecover, NULL, 1);
                    pBatchInSharedMsg != NULL;
                    ++i, pBatchInSharedMsg = getBatchInSharedMsg(pRecover, pBatchInSharedMsg, 1)) {
                assert(i < NB_MESSAGES);
                listAppend(listBatchInSharedMsg, pBatchInSharedMsg);
                if (memcmp(tabPBatches[i], pBatchInSharedMsg->batch, tabPBatches[i]->len) != 0) {
                    fprintf(stderr, "ERROR : Difference found during TEST_RECOVER_FIRST_SET_EMPTY for %d-th batch in pRecover\n", i);
                    return EXIT_FAILURE;
                }
            }          
            break;
        case TEST_RECOVER_SECOND_SET_EMPTY:
            assert(pRecover->msg.body.recover.initDone == true);
            pBatchInSharedMsg = getBatchInSharedMsg(pRecover, NULL, 0);
            assert(pBatchInSharedMsg != NULL);
            listAppend(listBatchInSharedMsg, pBatchInSharedMsg);
            for (i = 0, pBatchInSharedMsg = getBatchInSharedMsg(pRecover, NULL, 0);
                    pBatchInSharedMsg != NULL;
                    ++i, pBatchInSharedMsg = getBatchInSharedMsg(pRecover, pBatchInSharedMsg, 0)) {
                assert(i < NB_MESSAGES);
                listAppend(listBatchInSharedMsg, pBatchInSharedMsg);
                if (memcmp(tabPBatches[i], pBatchInSharedMsg->batch, tabPBatches[i]->len) != 0) {
                    fprintf(stderr, "ERROR : Difference found during TEST_RECOVER_SECOND_SET_EMPTY for %d-th batch in pRecover\n", i);
                    return EXIT_FAILURE;
                }
            }          
            pBatchInSharedMsg = getBatchInSharedMsg(pRecover, NULL, 1);
            assert(pBatchInSharedMsg == NULL);
            break;
        case TEST_RECOVER_NON_EMPTY_SETS:
            assert(pRecover->msg.body.recover.initDone == true);
            pBatchInSharedMsg = getBatchInSharedMsg(pRecover, NULL, 0);
            assert(pBatchInSharedMsg != NULL);
            listAppend(listBatchInSharedMsg, pBatchInSharedMsg);
            for (i = 0, pBatchInSharedMsg = getBatchInSharedMsg(pRecover, NULL, 0);
                    pBatchInSharedMsg != NULL;
                    ++i, pBatchInSharedMsg = getBatchInSharedMsg(pRecover, pBatchInSharedMsg, 0)) {
                assert(i < NB_MESSAGES/2);
                listAppend(listBatchInSharedMsg, pBatchInSharedMsg);
                if (memcmp(tabPBatches[i], pBatchInSharedMsg->batch, tabPBatches[i]->len) != 0) {
                    fprintf(stderr, "ERROR : Difference found during TEST_RECOVER_NON_EMPTY_SETS for %d-th batch in pRecover\n", i);
                    return EXIT_FAILURE;
                }
            }          
            pBatchInSharedMsg = getBatchInSharedMsg(pRecover, NULL, 1);
            assert(pBatchInSharedMsg != NULL);
            listAppend(listBatchInSharedMsg, pBatchInSharedMsg);
            for (i = 0, pBatchInSharedMsg = getBatchInSharedMsg(pRecover, NULL, 1);
                    pBatchInSharedMsg != NULL;
                    ++i, pBatchInSharedMsg = getBatchInSharedMsg(pRecover, pBatchInSharedMsg, 1)) {
                assert(i < NB_MESSAGES/2);
                listAppend(listBatchInSharedMsg, pBatchInSharedMsg);
                if (memcmp(tabPBatches[i+NB_MESSAGES/2], pBatchInSharedMsg->batch, tabPBatches[i+NB_MESSAGES/2]->len) != 0) {
                    fprintf(stderr, "ERROR : Difference found during TEST_RECOVER_NON_EMPTY_SETS for %d-th batch in pRecover\n", i);
                    return EXIT_FAILURE;
                }
            }          
            break;
    }
    free(mp);
    deleteBbSharedMsg(pRecover);
    return 0;
}

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
    BbBatchInSharedMsg* tabPBatchesInSharedMsg[NB_MESSAGES] = {NULL};
    int i,j;
    
    ///////////////////////////////
    // Initializations
    ///////////////////////////////
    listBatchInSharedMsg = newList();
    bbSingleton.currentWave = 0;
    memset(&(bbSingleton.view), 42, sizeof(bbSingleton.view));
    bbSingleton.viewId = 43;

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
        tabPBatchesInSharedMsg[i] = pBatchInSharedMsg;
        if (memcmp(tabPBatches[i], pBatchInSharedMsg->batch, tabPBatches[i]->len) != 0) {
            fprintf(stderr, "ERROR : Difference found for %d-th batch in pSet\n", i);
            return EXIT_FAILURE;
        }
    }
    
    deleteBbSharedMsg(pSet);
    
    ///////////////////////////////
    // Test on a Recover when bbSingleton.initDone == false
    ///////////////////////////////
    for (i = 0 ; i < NB_WAVE ; ++i) {
        for (j = 0 ; j< MAX_MEMB ; ++j) {
            rcvdBatch[i][j] = NULL;
        }
    }
    bbSingleton.initDone = false;
    doneTest = TEST_RECOVER_WHEN_INITDONE_FALSE;
    buildAndProcess_RECOVER(allocateRecoverForTest, testBuiltRecover);

    ///////////////////////////////
    // Test on a Recover when bbSingleton.initDone == true and both sets are empty
    ///////////////////////////////
    for (i = 0 ; i < NB_WAVE ; ++i) {
        for (j = 0 ; j< MAX_MEMB ; ++j) {
            rcvdBatch[i][j] = NULL;
        }
    }
    bbSingleton.initDone = true;
    doneTest = TEST_RECOVER_BOTH_SETS_EMPTY;
    buildAndProcess_RECOVER(allocateRecoverForTest, testBuiltRecover);

    ///////////////////////////////
    // Test on a Recover when bbSingleton.initDone == true and first set is empty
    ///////////////////////////////
    for (i = 0 ; i < NB_WAVE ; ++i) {
        for (j = 0 ; j< MAX_MEMB ; ++j) {
            rcvdBatch[i][j] = NULL;
        }
    }
    for (j = 0 ; j< NB_MESSAGES ; ++j) {
       rcvdBatch[bbSingleton.currentWave][j] = tabPBatchesInSharedMsg[j];
    }
    bbSingleton.initDone = true;
    doneTest = TEST_RECOVER_FIRST_SET_EMPTY;
    buildAndProcess_RECOVER(allocateRecoverForTest, testBuiltRecover);

    ///////////////////////////////
    // Test on a Recover when bbSingleton.initDone == true and second set is empty
    ///////////////////////////////
    for (i = 0 ; i < NB_WAVE ; ++i) {
        for (j = 0 ; j< MAX_MEMB ; ++j) {
            rcvdBatch[i][j] = NULL;
        }
    }
    for (j = 0 ; j< NB_MESSAGES ; ++j) {
       rcvdBatch[NB_WAVE-1][j] = tabPBatchesInSharedMsg[j];
    }
    bbSingleton.initDone = true;
    doneTest = TEST_RECOVER_SECOND_SET_EMPTY;
    buildAndProcess_RECOVER(allocateRecoverForTest, testBuiltRecover);

    ///////////////////////////////
    // Test on a Recover when bbSingleton.initDone == true and both sets are non-empty
    ///////////////////////////////
    for (i = 0 ; i < NB_WAVE ; ++i) {
        for (j = 0 ; j< MAX_MEMB ; ++j) {
            rcvdBatch[i][j] = NULL;
        }
    }
    for (j = 0 ; j< NB_MESSAGES/2 ; ++j) {
       rcvdBatch[NB_WAVE-1][j] = tabPBatchesInSharedMsg[j];
    }
    for (j = 0 ; j< NB_MESSAGES/2 ; ++j) {
       rcvdBatch[bbSingleton.currentWave][j+NB_MESSAGES/2] = tabPBatchesInSharedMsg[j+NB_MESSAGES/2];
    }
    bbSingleton.initDone = true;
    doneTest = TEST_RECOVER_NON_EMPTY_SETS;
    buildAndProcess_RECOVER(allocateRecoverForTest, testBuiltRecover);
    
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

