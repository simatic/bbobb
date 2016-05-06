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

/*
 * @brief structure to contain all global var of bbobb
 */
typedef struct {
    bool bbInitDone; /*!<true if automaton is init>*/
    int error; /*!<use to detect error in BBOBB*/
    BbOrder bbreqOrder; /*!<Order used in BBOBB>*/
    pthread_mutex_t bbStateMachineMutex; /*!<mutex used in statemachine>*/
    BbState bbAutomatonState; /*!<State of BBOBB automaton>*/
    address bbMyAddress; /*!<address of the host process>*/
    unsigned char currentWave; /*!<current wave for automaton>*/
    unsigned char currentStep; /*!<current step in current wave>*/
    unsigned char bbViewSize; /*!<number of paticipants, max 256>*/
    circuitView bbView; /*!<current view of participants>*/
    pthread_mutex_t viewChangeMutex; /*!<mutex used when view changes>*/
    trComm * bbCommForAccept; /*!<trComm used to accept connections>*/
    trBqueue * bbMsgQueue; /*!<trBqueue used to store bbMsg before treatement>*/
} BbSingleton;


/*
 * @brief initialisation of Singleton
 * @return BbSingleton
 */
BbSingleton * bbSingletonInit();

#endif /* BBSINGLETON_H */

