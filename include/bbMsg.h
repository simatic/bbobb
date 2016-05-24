/**
* @brief File holding the types' definition for the messages exchanged between processes
* @file bbMsg.h
* @author May CATTANT - Quentin MONNOT - Victor ROUQUETTE - Michel SIMATIC
* @date 23 february 2016
*/
 
#ifndef _BB_MSG_H
#define _BB_MSG_H

#include "address.h"
#include "applicationMessage.h"
#include "advanced_struct.h"

/**
* @brief Data structure for BB_MType (not defined as an enum in order to use only
* one byte instead of four)
*/
typedef unsigned char BB_MType;

#define BB_MSG_RECOVER      0
#define BB_MSG_SET          1
#define BB_MSG_VIEW_CHANGE  2

#define BB_LAST_MSG BB_MSG_VIEW_CHANGE

/**
* @brief Data structure for batch
*/
typedef struct {
  unsigned int len;     /**< Total lngth of structure */
  address      sender;  /**< Sender of set */
  message      messages[];  /**< Messages stored in this batch */
} __attribute__((packed)) BbBatch;

/**
* @brief Data structure for BB_SET messages
*/
typedef struct {
  unsigned char wave; /**< Wave in which this set was sent */
  unsigned char step; /**< Step in which this set was sent */
  unsigned int viewId; /**<Id of current view */
  BbBatch batches[];
} __attribute__((packed)) BbSet;

typedef struct {
    unsigned int len;
    BbSet set;
}__attribute__((packed)) BbSetInRecover;

/**
* @brief Data structure for BB_RECOVER messages
*/
typedef struct {
  address       sender;   /**< Sender of message */
  unsigned int  viewId;   /** View Id */
  circuitView   view;     /**< View at the moment the message was sent */
  bool          initDone; /**< Value of initDone at the moment the message was sent */
  unsigned char nbSets;   /**< Number of sets which are sent in this message */
  BbSetInRecover sets[];   /**< Sets not yet delivered by the sender */
} __attribute__((packed)) BbRecover;

/**
* @brief Data structure for BB_VIEW_CHANGE messages
*/
typedef struct {
  circuitView   view;     /**< View */
} __attribute__((packed)) BbViewChange;

/**
* @brief Data structure for BbMsg
*/
typedef struct {
  unsigned int len;   /**< Total length of structure */
  BB_MType type;      /**< Type of message */

  union {
    BbRecover    recover;
    BbSet        set;
    BbViewChange viewChange;
  }body;/**<A union between all the types of possible messages */
}__attribute__((packed)) BbMsg;

/**
 * @brief Search the first message contained in a batch @a b
 * @param[in] b A pointer on the batch in which to search the first message
 * @return A pointer on a first message or NULL if @a b contains no messages
 */
message *firstMsgInBatch(BbBatch *b);

/**
 * @brief Give the message following message @a mp in batch @a b
 * @param[in] b A pointer on a batch in which to search next message
 * @param[in] mp A pointer on a message after which to search for an other message
 * @return A pointer on a next message or NULL if @a b contains no other messages
 */
message *nextMsgInBatch(BbBatch *w, message *mp);

/**
 * @brief Allocate a message in @a bbSingleton.batchToSend
 * @param[in] payloadSize the required size to allocate
 * @return A pointer on a message
 */
message * allocInBatchToSend(int payloadSize);

#endif /* _BB_MSG_H */
