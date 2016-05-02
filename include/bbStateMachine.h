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
  BB_STATE_VIEW_CHANGE, /**< Managing a view change */
} BbState;

typedef enum {
    BB_TOTAL_ORDER,
    BB_UNIFORM_TOTAL_ORDER,
    BB_CAUSAL_ORDER,
    BB_UNIFORM_TOTAL_CAUSAL_ORDER
} BbOrder;

typedef struct {
    trBqueue * queue;
    trComm * comm;
} bbQueueComm;

#define BB_LAST_STATE BB_STATE_VIEW_CHANGE

/** 
 * @brief Type of function used to code transitions in state machine
 */
typedef  BbState (*BbStateMachineFunc)(BbState, BbMsg*);

extern trComm * bbCommForAccept;
extern trBqueue * bbMsgQueue;
extern bbQueueComm * QCForAcceptThread;

/**
 * @brief Order used in BBOBB
 */
extern BbOrder bbreqOrder;

/**
 * @brief mutex used in statemachine
 */
extern pthread_mutex_t bbStateMachineMutex;

/**
* @brief State of BBOBB automaton
*/
extern BbState bbAutomatonState;

/**
 * @brief address of the host process
 */
extern address bbMyAddress;

/**
 * @brief current wave for automaton
 */
extern unsigned char currentWave;

/**
 * @brief current step in current wave
 */
extern unsigned char currentStep;

/**
 * @brief number of paticipants, max 256
 * @brief equal to size of view
 */
extern unsigned char bbViewSize;

/**
 * @brief true if automaton is init
 */
extern bool bbInitDone;

/**
 * @brief current view of participants
 */
extern circuitView bbView;

/**
 * @brief mutex used when view changes
 */
extern pthread_mutex_t viewChangeMutex;

/** 
 * @brief Execute a transition of state machine based on @a pMsg
 * @param[in] pMsg Message to be processed by state machine
 */
void bbStateMachineTransition(BbMsg* pMsg);

/*
 * @brief initializes the automaton with default parameters and star usefull threads
 */
int bbAutomatonInit();

/*
 * @brief thread to treat msg in queue with fifo order
 */
void * bbMsgTreatement(void * data);

#endif /* _BB_STATE_MACHINE_H */
