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
    wowmim aMsg;
    do{
        aMsg = receive()
    
}

