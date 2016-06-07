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
#include "bbSharedMsg.h"

/**
* @brief Data structure for BbState
*/
typedef enum {
  BB_STATE_ALONE,              /**< The process is alone */
  BB_STATE_SEVERAL,            /**< There is more than one participating process */
  BB_STATE_START,              /**< Starting state of the automaton */
  BB_STATE_VIEW_CHANGE, /**< Managing a view change */
} BbState;

#define BB_LAST_STATE BB_STATE_VIEW_CHANGE

#define WAVE_MAX 256

/** 
 * @brief Type of function used to code transitions in state machine
 */
typedef  BbState (*BbStateMachineFunc)(BbState, BbSharedMsg*);

/** 
 * @brief Execute a transition of state machine based on @a pMsg
 * @param[in] pMsg Message to be processed by state machine
 */
void bbStateMachineTransition(BbSharedMsg* pSharedMsg);

/*
 * @brief initializes the automaton with default parameters and star usefull threads
 */
int bbAutomatonInit();

/*
 * @brief thread to treat msg in queue with fifo order
 */
void * bbMsgTreatement(void* unused);

/**
 * @brief create a BbMsg with all batches in rcvdBatch
 * @brief function uses in tOBroadcastRecover
 * @param[in] waveNum of wave in which to take batches
 * @return a BbMsg set which contains all rcvdBatches of waveNum
 */
BbMsg * createSet(unsigned char waveNum);

void buildNewSet(BbMsg * pset, struct iovec * piov, int * piovcnt);
#endif /* _BB_STATE_MACHINE_H */
