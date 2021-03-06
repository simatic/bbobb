/**
* @brief File holding the types' definition for the messages exchanged between processes
* @file bb_msg.h
* @author May CATTANT - Quentin MONNOT - Victor ROUQUETTE - Michel SIMATIC
* @date 23 february 2016
*/
 
#ifndef _BB_MSG_H
#define _BB_MSG_H

#include "address.h"
#include "applicationMessage.h"

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
  address      sender;  /**< Sender of wagon */
  message      messages[];  /**< Messages stored in this batch */
} __attribute__((packed)) BbBatch;

/**
* @brief Data structure for BB_SET messages
*/
typedef struct {
  unsigned char wave; /**< Wave in which this set was sent */
  unsigned char step; /**< Step in which this set was sent */
  BbBatch batches[];
} __attribute__((packed)) BbSet;

/**
* @brief Data structure for BB_RECOVER messages
*/
typedef struct {
  address       sender;   /**< Sender of message */
  circuitView   view;     /**< View at the moment the message was sent */
  bool          initDone; /**< Value of initDone at the moment the message was sent */
  unsigned char nbSets;   /**< Number of sets which are sent in this message */
  BbSet         sets[];   /**< Sets not yet delivered by the sender */
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

/*
 * @brief Description of the structure wich will be placed before the messages
 * @note Holds two paameters : a @a mutex to control the writting and a @a counter
 */
typedef prefix ControlSharing;

/*
 * @brief Data structure used to improve message's traitement in automaton
 * @note TO DO
 */
typedef struct {
    ControlSharing prefix;
    BbMsg msg;
}__attribute__((packed)) BbSharedMsg;

/*
 * @brief Data structure used to deliver batchs to applicationnal level
 */
typedef struct {
   BbSharedMsg* msg;
   BbBatch* batch;
}__attribute__((packed)) BatchInSharedMsg;

#endif /* _BB_MSG_H */
