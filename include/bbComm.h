/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   bbComm.h
 * Author: terag
 *
 * Created on April 21, 2016, 11:37 AM
 */

#ifndef BBCOMM_H
#define BBCOMM_H

#include "bbStateMachine.h"
#include "comm.h"

#ifdef __cplusplus
extern "C" {
#endif

void waitCommForAccept(bbQueueComm QCForAcceptThread);

void bbConnectionMgt(bbQueueComm QCForAcceptThread);




#ifdef __cplusplus
}
#endif

#endif /* BBCOMM_H */

