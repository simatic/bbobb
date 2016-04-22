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
#include "advanced_struct.h"
#include "counter.h"

void waitCommForAccept(bbQueueComm QCForAcceptThread){
    trComm * aComm;
    do{
        aComm = commAccept(&QCForAcceptThread.comm);
        pthread_t connectionMgtThread;
        error = pthread_create(&connectionMgtThread, NULL, bbConnectionMgt(), NULL);
        pthread_detach(connectionMgtThread);
    }while(1);
}

void bbConnectionMgt(bbQueueComm QCForAcceptThread){
    womim aMsg; /* TO DO : Changer en SharedMsg */
    do{
        aMsg = receive();
        bqueueEnqueue(QCForAcceptThread.queue, &aMsg);
    }while(1); 
}

