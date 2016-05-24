/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   bbSingleton.c
 * Author: terag
 *
 * Created on May 5, 2016, 6:17 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "bbSingleton.h"
#include "bbComm.h"

BbSingleton bbSingleton;

int bbSingletonInit() {
    int i;
    
    bbSingleton.initDone = false;
    
    char * port = bbGetLocalPort();
    bbSingleton.commForAccept = commNewForAccept(port);
    for (i = 0 ; i < MAX_MEMB ; ++i) {
        bbSingleton.commToViewMembers[i] = NULL;
    }
    bbSingleton.msgQueue = newBqueue();
    
    bbSingleton.currentWave = 0;
    bbSingleton.currentStep = 0;
    
    bbSingleton.batchToSend->sender = bbSingleton.myAddress;
    
    return 0;
}