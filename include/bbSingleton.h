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
 * @BbOrder Order used in BBOBB

 * @brief mutex used in statemachine
 */
/**
* @brief State of BBOBB automaton
*/
/**
 * @brief address of the host process
 */

/**
 * @brief current wave for automaton
 */

/**
 * @brief current step in current wave
 */
/**
 * @brief number of paticipants, max 256
 * @brief equal to size of view
 */
/**
 * @brief true if automaton is init
 */
/**
 * @brief current view of participants
 */
/**
 * @brief mutex used when view changes
 */
typedef struct {
    BbOrder bbreqOrder;

    pthread_mutex_t bbStateMachineMutex;

    BbState bbAutomatonState;

    address bbMyAddress;
    unsigned char currentWave;
    unsigned char currentStep;

    unsigned char bbViewSize;

    bool bbInitDone;

    circuitView bbView;

    pthread_mutex_t viewChangeMutex;

    trComm * bbCommForAccept;
    trBqueue * bbMsgQueue;
    BbSingleton * singleton;
}

#endif /* BBSINGLETON_H */

