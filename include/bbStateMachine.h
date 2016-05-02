/**
* @brief File holding the types' definition related to the state machine of BBOBB
* @file bb_msg.h
* @author May CATTANT - Quentin MONNOT - Victor ROUQUETTE - Michel SIMATIC
* @date 23 february 2016
*/
 
#ifndef _BB_STATE_MACHINE_H
#define _BB_STATE_MACHINE_H

#include <pthread.h>
#include "bbMsg.h"
#include "bqueue.h"
#include "comm.h"

/**
* @brief Data structure for BbState
*/
typedef enum {
  BB_STATE_ALONE,              /**< The process is alone */
  BB_STATE_SEVERAL,            /**< There is more than one participating process */
  BB_STATE_MANAGE_VIEW_CHANGE, /**< Managing a view change */
  BB_STATE_WAIT_VIEW_CHANGE    /**< Just started ==> waiting for a view change */
} BbState;

typedef struct {
    trBqueue * queue;
    trComm * comm;
} bbQueueComm;

#define BB_LAST_STATE BB_STATE_WAIT_VIEW_CHANGE

/**
* @brief State of BBOBB automaton
*/
extern BbState bbAutomatonState;

/** 
 * @brief Type of function used to code transitions in state machine
 */
typedef  BbState (*BbStateMachineFunc)(BbState, BbMsg*);

extern trComm * bbCommForAccept;
extern trBqueue * bbMsgQueue;
extern bbQueueComm * QCForAcceptThread;

/** 
 * @brief Execute a transition of state machine based on @a pMsg
 * @param[in] pMsg Message to be processed by state machine
 */
void bbStateMachineTransition(BbMsg* pMsg);

int bbAutomatonInit();

void bbMsgTreatement(void * data);

#endif /* _BB_STATE_MACHINE_H */
