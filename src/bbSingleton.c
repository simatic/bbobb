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

BbSingleton * bbSingleton;

int bbSingletonInit() {
    
    bbSingleton = malloc(sizeof(BbSingleton));
    if(bbSingleton == NULL) {
        return 1;
    }
    
    bbSingleton->initDone = false;
    
    char port[] = "8000";
    bbSingleton->commForAccept = commNewForAccept(port);
    bbSingleton->msgQueue = newBqueue();
    
    return 0;
}