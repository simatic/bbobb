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

#include "bbStateMachine.h"

/**
 * @brief Max size of a set
 */
 extern int setMaxLen;

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

int bbInit();

int bbTerminate();

#endif /* BBOBB_H */

