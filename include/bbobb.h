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
 * @brief function uses to init bbobb protocol
 */
int bbInit(CallbackCircuitChange callbackCircuitChange, CallbackODeliver callbackODeliver, BbOrder reqOrder);

/**
 * @brief function uses to terminate bbobb protocol
 */
int bbTerminate();

#endif /* BBOBB_H */

