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
#include "bbMsg.h"
#include "comm.h"

#include <sys/uio.h>

#ifdef __cplusplus
extern "C" {
#endif

void waitCommForAccept(void * data);

void bbConnectionMgt(void * data);

BbSharedMsg * bbReceive(trComm * aComm);


#ifdef __cplusplus
}
#endif

#endif /* BBCOMM_H */

