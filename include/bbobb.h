/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   bbobb.h
 * Author: terag
 *
 * Created on April 22, 2016, 1:04 AM
 */

#ifndef BBOBB_H
#define BBOBB_H

#include "trains.h"
#include "bbApplicationMessage.h"

typedef enum {
    BB_TOTAL_ORDER,
    BB_UNIFORM_TOTAL_ORDER,
    BB_CAUSAL_ORDER
} BbOrder;

/**
 * @brief Localisation of the file where addresses are written
 */
#define LOCALISATION "./addr_file"/**<File's here.*/

/**
 * @brief The maximum number of rounds used for the modulos
 */
#define NR 3

/**
 * @brief Time to wait
 */
#define CONNECT_TIMEOUT 2000

/**
 * @brief The limit of time to wait
 */
extern int bbWaitNbMax;

/**
 * @brief The default time to wait
 */
extern int bbWaitDefaultTime; /**<in microsecond*/

/**
 * @brief Max size of a set
 */
 extern int bbSetMaxLen;

/**
 * @brief The limit of time to wait
 */
extern int bbWaitNbMax;

/**
 * @brief The default time to wait
 */
extern int bbWaitDefaultTime; /**<in microsecond*/

/**
 * @brief Store the error number specific to errors in bbobb middleware
 */
extern int bbErrno;

/**
 * @brief Initialization of bbobb protocol middleware

 * @param[in] batchMaxLen, max len of batches
 * @param[in] waitNb The number of time to wait
 * @param[in] waitTime The time to wait (in microsecond)
 * @param[in] callbackCircuitChange Function to be called when there is a circuit changed (Arrival or departure of a process)
 * @param[in] callbackODeliver    Function to be called when a message can be o-delivered by bbobb protocol
 * @param[in] reqOrder Order guarantees which Bbobb algorithm must provide while it is running
 * @return 0 upon successful completion, or -1 if an error occurred (in which case, @a bbErrno is set appropriately)
 */
int bbInit(int batchMaxLen, int waitNb, int waitTime, CallbackCircuitChange aCallbackCircuitChange, CallbackODeliver aCallbackODeliver, BbOrder reqOrder);

/**
 * @brief function uses to terminate bbobb protocol
 */
int bbTerminate();

#endif /* BBOBB_H */

