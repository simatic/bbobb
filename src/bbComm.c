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
#include "iomsg.h"

void * waitCommForAccept(void * data){
    trComm * aComm;
    int error;
    BbSingleton * singleton = data;
    printf("ConnectionMgt : OK\n");
    do{
        aComm = commAccept(singleton->comm);
        pthread_t connectionMgtThread;
        error = pthread_create(&connectionMgtThread, NULL, bbConnectionMgt, data);
        pthread_detach(connectionMgtThread);
    }while(1);
}

void * bbConnectionMgt(void * data){
    
    womim * aMsg; /* TO DO : Changer en SharedMsg */
    BbSingleton * singleton = data;
    printf("ConnectionMgt : OK\n");
    do{
        aMsg = receive(singleton->comm);
        bqueueEnqueue(singleton->queue, &aMsg);
    }while(1); 
}

