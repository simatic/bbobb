/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   bbSingleton.h
 * Author: terag
 *
 * Created on May 5, 2016, 6:16 PM
 */

#ifndef BBSINGLETON_H
#define BBSINGLETON_H

#include <stdio.h>
#include <stdlib.h>
#include "comm.h"
#include "bqueue.h"
#include "bbMsg.h"
#include "bbStateMachine.h"

/*typedef enum{
    TRUE,
    FALSE,
} bool;*/

/*
 * @brief structure to contain all global var of bbobb
 */
typedef struct {
    bool initDone; /*!<true if automaton is init>*/
    int error; /*!<use to detect error in BBOBB*/
    BbOrder reqOrder; /*!<Order used in BBOBB>*/
    pthread_mutex_t stateMachineMutex; /*!<mutex used in statemachine>*/
    BbState automatonState; /*!<State of BBOBB automaton>*/
    address myAddress; /*!<address of the host process>*/
    unsigned char currentWave; /*!<current wave for automaton>*/
    unsigned char currentStep; /*!<current step in current wave>*/
    unsigned char viewSize; /*!<number of paticipants, max 256>*/
    unsigned int viewId; /*current view ID number*/
    circuitView * view; /*!<current view of participants>*/
    pthread_mutex_t viewChangeMutex; /*!<mutex used when view changes>*/
    trComm * commForAccept; /*!<trComm used to accept connections>*/
    trBqueue * msgQueue; /*!<trBqueue used to store bbMsg before treatement>*/
} BbSingleton;

extern BbSingleton * bbSingleton;

/*
 * @brief initialisation of @a Singleton
 * @return a pointer on @a BbSingleton
 */
int bbSingletonInit();

#endif /* BBSINGLETON_H */
