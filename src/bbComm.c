/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   bbComm.c
 * Author: terag
 *
 * Created on April 21, 2016, 11:37 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

#include "bbComm.h"
#include "comm.h"
#include "advanced_struct.h"
#include "counter.h"
#include "iomsg.h"
#include "bbSingleton.h"
#include "bbSharedMsg.h"

void * waitCommForAccept(void){

    printf("ConnectionWait : OK\n");
    
    do{
        receive(bbSingleton->commForAccept);
        pthread_t connectionMgtThread;
        pthread_create(&connectionMgtThread, NULL, bbConnectionMgt, NULL);
        pthread_detach(connectionMgtThread);
    }while(1);
}

void * bbConnectionMgt(void){
    
    BbSharedMsg * aMsg /*= newBbSharedMsg(sizeof(BbMsg))*/;

    //trComm * rcvdComm = commAlloc(fd);
    printf("ConnectionMgt : OK\n");
    
    do{
        //aMsg = receive(singleton->commForAccept);
        bqueueEnqueue(bbSingleton->msgQueue, &aMsg);
    }while(1); 
}

