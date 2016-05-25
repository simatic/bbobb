#include <assert.h>
#include <stddef.h>
#include "common.h"
#include "bbMsg.h"
#include "bbSharedMsg.h"
#include "bbSingleton.h"
#include "bbError.h"

BbSharedMsg* newBbSharedMsg(size_t size){
  BbSharedMsg *sharedMsg = malloc(offsetof(BbSharedMsg,msg)+size);
  assert(sharedMsg != NULL);
  pthread_mutex_init(&(sharedMsg->controlSharing.mutex), NULL);
  sharedMsg->controlSharing.counter = 1;
  return sharedMsg;  
}

void deleteBbSharedMsg(BbSharedMsg *sharedMsg){
  if (sharedMsg != NULL){
    MUTEX_LOCK(sharedMsg->controlSharing.mutex);
    sharedMsg->controlSharing.counter -= 1;
    if (sharedMsg->controlSharing.counter == 0) {
      MUTEX_UNLOCK(sharedMsg->controlSharing.mutex);
      MUTEX_DESTROY(sharedMsg->controlSharing.mutex);
      free(sharedMsg);
    } else {
      MUTEX_UNLOCK(sharedMsg->controlSharing.mutex);
    }
  }
}

static void incrementCounterInBbSharedMsg(BbSharedMsg *sharedMsg){
  if (sharedMsg != NULL){
    MUTEX_LOCK(sharedMsg->controlSharing.mutex);
    sharedMsg->controlSharing.counter += 1;
    MUTEX_UNLOCK(sharedMsg->controlSharing.mutex);
  }
}

BbBatchInSharedMsg* newBatchInSharedMsg(BbBatch *batch, BbSharedMsg *sharedMsg){
  BbBatchInSharedMsg *batchInSharedMsg = malloc(sizeof(BbBatchInSharedMsg));
  assert(batchInSharedMsg != NULL);
  assert(batch != NULL);
  assert(sharedMsg != NULL);
  incrementCounterInBbSharedMsg(sharedMsg);
  batchInSharedMsg->batch = batch;
  batchInSharedMsg->sharedMsg = sharedMsg;
  return batchInSharedMsg;
}

BbBatchInSharedMsg* newEmptyBatchInNewSharedMsg(size_t batchMaxSize){
    BbSharedMsg *sharedSet = newBbSharedMsg(batchMaxSize);
    BbBatchInSharedMsg *batchInSharedMsg = newBatchInSharedMsg(sharedSet->msg.body.set.batches, sharedSet);
    deleteBbSharedMsg(sharedSet);
    sharedSet = NULL;
    batchInSharedMsg->batch->sender = bbSingleton.myAddress;
    batchInSharedMsg->batch->len = sizeof(BbBatch);
    return batchInSharedMsg;
}

BbBatchInSharedMsg* copyBatchInSharedMsg(BbBatchInSharedMsg *batchInSharedMsg){
  BbBatchInSharedMsg *aCopyBatchInSharedMsg = malloc(sizeof(BbBatchInSharedMsg));
  assert(aCopyBatchInSharedMsg != NULL);
  assert(batchInSharedMsg != NULL);
  incrementCounterInBbSharedMsg(batchInSharedMsg->sharedMsg);
  *aCopyBatchInSharedMsg = *batchInSharedMsg;
  return aCopyBatchInSharedMsg;
}

void deleteBatchInSharedMsg(BbBatchInSharedMsg *batchInSharedMsg){
  assert(batchInSharedMsg != NULL);
  deleteBbSharedMsg(batchInSharedMsg->sharedMsg);
  batchInSharedMsg->batch = NULL;
  batchInSharedMsg->sharedMsg = NULL;
  free(batchInSharedMsg);
}

BbBatchInSharedMsg* getBatchInSharedMsg(BbSharedMsg *sharedMsg, BbBatchInSharedMsg * lastReturnedBatch, int rankSet) {
    int type = sharedMsg->msg.type;
    BbBatch * batch = NULL; 
    unsigned int len = sharedMsg->msg.len;
    char * offset = NULL;
    int i;
    
    switch (type) {
    case BB_MSG_SET :
        if(lastReturnedBatch == NULL) {
            offset = (char*)&(sharedMsg->msg.body.set.batches[0]);
        } else {
            offset = (char*)lastReturnedBatch + lastReturnedBatch->batch->len;
        }
        if(offset - (char*)&(sharedMsg->msg) >= len) {
            return NULL;
        }
        batch = (BbBatch*)offset;
        break;
    case BB_MSG_RECOVER :
        if(rankSet > sharedMsg->msg.body.recover.nbSets) {
            return NULL;
        }
        
        offset = (char*)sharedMsg + offsetof(BbSharedMsg ,msg.body.recover.sets);
        BbSetInRecover * set = (BbSetInRecover*)offset;
        for(i=0; i < rankSet; i++) {
            offset += set->len;
            set = (BbSetInRecover*)offset;
        }
        
        /*now offset is on set in wave*/
        offset += 3*sizeof(char) + sizeof(int);
        /*now offset is on first batch in set*/
        if(lastReturnedBatch == NULL) {
            return newBatchInSharedMsg((BbBatch*)offset, sharedMsg);
        }
        if( (char*)lastReturnedBatch - (char*)set <= 0 || (char*)lastReturnedBatch - (char*)set >= set->len ) {
            bbErrorAtLineWithoutErrnum( EXIT_FAILURE,
                                        __FILE__,
                                        __LINE__,
                                        "lastReturnedBatch is not in rankSet");
        }
        
        offset = (char*)lastReturnedBatch + lastReturnedBatch->batch->len;
        if((char*)offset - (char*)set >= set->len) {
            return NULL;
        }
        batch = (BbBatch*)offset;
                
        break;
    case BB_MSG_VIEW_CHANGE :
        bbErrorAtLineWithoutErrnum( EXIT_FAILURE,
                                    __FILE__,
                                    __LINE__,
                                    "bbSharedMsg unexpected Msg_Type : VIEW_CHANGE");
        break;
    default :
        bbErrorAtLineWithoutErrnum( EXIT_FAILURE,
                           __FILE__,
                           __LINE__,
                           "bbSharedMsg unexpected Msg_Type : unknow type");
         break;   
    }
    
    return newBatchInSharedMsg(batch, sharedMsg);
}