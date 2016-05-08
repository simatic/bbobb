#include <assert.h>
#include <stddef.h>
#include "common.h"
#include "bbSharedMsg.h"

BbSharedMsg* newBbSharedMsg(size_t size){
  BbSharedMsg *sharedMsg = malloc(offsetof(BbSharedMsg,msg)+size);
  assert(sharedMsg != NULL);
  pthread_mutex_init(&(sharedMsg->controlSharing.mutex), NULL);
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
