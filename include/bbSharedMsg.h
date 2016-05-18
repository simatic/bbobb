/**
* @brief File for managing in memory the messages exchanged between processes
         and the batches they contain.
* @file bb_shared_msg.h
* @author May CATTANT - Quentin MONNOT - Victor ROUQUETTE - Michel SIMATIC
* @date 3 may 2016
*/
 
#ifndef _BB_SHARED_MSG_H
#define _BB_SHARED_MSG_H

#include <pthread.h>
#include "bbMsg.h"
#include "address.h"

/*
 * @brief Structure used to control how many pointers point on a BbMsg
          structure, so that we are able to free a structure when there are
          no more pointers.
   @note Holds two parameters : a @a counter of the number of pointers
         pointing on the structure and a @a mutex to control the @a counter
         modification.
 */
typedef struct{
  int counter;/**< Count the number of pointers refering to the associated
                   BbMsg */
  pthread_mutex_t mutex;/**< To make modifications of counter safe */
} BbControlSharing;

/*
 * @brief Data structure used to save malloc/free of BbMsg
 */
typedef struct {
    BbControlSharing controlSharing;
    BbMsg msg;
}__attribute__((packed)) BbSharedMsg;

/*
 * @brief Data structure used to point to batch stored in a BbMsg (when
          the BbMsg is a set)
 */
typedef struct {
   BbBatch* batch;
   BbSharedMsg* sharedMsg;
}__attribute__((packed)) BbBatchInSharedMsg;

/**
 * @brief Allocate a BbSharedMsg able to hold a @a BbMsg strcture 
          of @a size bytes
 * @param[in] size of the @a BbMsg structure
 * @return A pointer on the @a BbSharedMsg
 */
BbSharedMsg* newBbSharedMsg(size_t size);

/**
 * @brief Decrements the number of pointers pointing to @a sharedMsg and
          frees it if there are no more ponters pointing to it
 * @param[in] sharedMsg sharedMsg to work on
 */
void deleteBbSharedMsg(BbSharedMsg *sharedMsg);

/**
 * @brief Creates a BbBatchInSharedMsg which points to a @a batch located
          inside @a sharedMsg
 * @param[in] batch the @a BbBatch to point to
 * @param[in] sharedMsg the @a BbSharedMsg containing this @a BbBatch
 * @return A pointer on the @a BbBatchInSharedMsg
 */
BbBatchInSharedMsg* newBatchInSharedMsg(BbBatch *batch, BbSharedMsg *sharedMsg);

/**
 * @brief Creates a copy of @a batchInSharedMsg
 * @param[in] batchInSharedMsg the @a BbBatchInSharedMsg to copy
 * @return A pointer on a copy of @a batchInSharedMsg
 */
BbBatchInSharedMsg* copyBatchInSharedMsg(BbBatchInSharedMsg *batchInSharedMsg);

/**
 * @brief Frees @a batchInSharedMsg (and frees pointed BbSharedMsg if
          there are no more pointers on it)
 * @param[in] batchInSharedMsg BbBatchInSharedMsg to free
 */
void deleteBatchInSharedMsg(BbBatchInSharedMsg *batchInSharedMsg);

/**
 * @brief Get if exist a BbBatchInSharedMsg which points to a @a batch of @a sender located
          inside @a sharedMsg
 * @param[in] sharedMsg the @a BbSharedMsg containing this @a BbBatch
 * @param[in] last @a BbBatchInSharedMsg return by the function or NULL if it doesn't exist
 * @param[in] rank of @a BbSet where the searched @a BbBatch is
 * @return A pointer on the @a BbBatchInSharedMsg or NULL
 */
BbBatchInSharedMsg* getBatchInSharedMsg(BbSharedMsg *sharedMsg, BbBatchInSharedMsg * lastReturnedBatch, int rankSet);

#endif /* _BB_SHARED_MSG_H */
