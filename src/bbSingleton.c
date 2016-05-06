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

BbSingleton * bbSingletonInit() {
    BbSingleton * singleton = NULL;
    singleton = malloc(sizeof(BbSingleton));
    
    singleton->bbInitDone = false;
    if(singleton->error = pthread_mutex_unlock(&(singleton->bbStateMachineMutex)) != 0){
        perror("error with stateMachineMutex's init");
    }
}