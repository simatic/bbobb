/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   bbCallBack.h
 * Author: terag
 *
 * Created on May 23, 2016, 2:34 PM
 */

#ifndef BBCALLBACK_H
#define BBCALLBACK_H
#include "trains.h"

/** 
 * @brief Callback function called by Trains middleware when there is a change in circuit members
 */
void bbCallbackCircuitChange(circuitView *pcv);

/** 
 * @brief Callback function called by Trains middleware when it wants to o-deliver a message to BBOBB
 */
void bbCallbackODeliver(address sender, t_typ messageType, message * mp);




#endif /* BBCALLBACK_H */

