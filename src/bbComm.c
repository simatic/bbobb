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
#include "management_addr.h"
#include "advanced_struct.h"
#include "counter.h"
#include "bbiomsg.h"
#include "bbSingleton.h"
#include "bbSharedMsg.h"

void * waitCommForAccept(void *unused){

    printf("ConnectionWait : OK\n");

    do{
        trComm *aComm = commAccept(bbSingleton.commForAccept);
        pthread_t thread;
        pthread_create(&thread, NULL, bbConnectionMgt, (void*)aComm);
        pthread_detach(thread);            
    }while(1);
}

void * bbConnectionMgt(void *arg){    
    BbSharedMsg * aMsg ;
    trComm * myComm = (trComm*)arg;
    printf("ConnectionMgt : OK\n");
    
    do{
        aMsg = (BbSharedMsg*)bbReceive(myComm);
        if(aMsg != NULL) {
            bqueueEnqueue(bbSingleton.msgQueue, &aMsg);
        }
    }while(aMsg != NULL); 
}

char *bbGetLocalPort(){
    char *trainsPortAsString;
    int trainsPortAsInt;
    int rc;
    static char bbobbPortAsString[MAX_LEN_CHAN];
    trainsPortAsString = getenv("TRAINS_PORT");
    if (trainsPortAsString == NULL){
        ERROR_AT_LINE_WITHOUT_ERRNUM(EXIT_FAILURE,__FILE__,__LINE__,"TRAINS_PORT environment variable is not defined");
    }
    rc = sscanf(trainsPortAsString, "%d", &trainsPortAsInt);
    if (rc != 1){
        ERROR_AT_LINE_WITHOUT_ERRNUM(EXIT_FAILURE,__FILE__,__LINE__,"TRAINS_PORT environment variable contains \"%s\" which is not an integer",trainsPortAsString);
    }
    sprintf(bbobbPortAsString, "%d", trainsPortAsInt+INTERVAL_BETWEEN_TRAINS_AND_BBOBB_PORTS);
    return bbobbPortAsString;
}

void connectToViewMembers(circuitView *pcv){
    int i;
    // We close all open connections
    for (i = 0 ; i < MAX_MEMB ; ++i) {
        if (bbSingleton.commToViewMembers[i] != NULL) {
            freeComm(bbSingleton.commToViewMembers[i]);
            bbSingleton.commToViewMembers[i] = NULL;
        }
    }
    // We open the connections to the members of the current view
    for (i = 0 ; i < pcv->cv_nmemb ; ++i) {
        if (!addrIsMine(pcv->cv_members[i])) {
            // This member is not the current process. We must open
            // a connection to it.
            int rank = addrToRank(pcv->cv_members[i]);
            if (rank == -1) {
                ERROR_AT_LINE_WITHOUT_ERRNUM(EXIT_FAILURE, __FILE__, __LINE__,
                    "Wrong address %d in members", pcv->cv_members[i]);
            }
            if (globalAddrArray[rank].chan[0] == '\0'){
                ERROR_AT_LINE_WITHOUT_ERRNUM(EXIT_FAILURE, __FILE__, __LINE__,
                    "Address %d is not specified in addr_file", pcv->cv_members[i]);                
            }
            bbSingleton.commToViewMembers[rank] = commNewAndConnect(
                    globalAddrArray[rank].ip,
                    globalAddrArray[rank].chan,
                    CONNECT_TIMEOUT);
            if (bbSingleton.commToViewMembers[rank] == NULL ){
                ERROR_AT_LINE_WITHOUT_ERRNUM(EXIT_FAILURE, __FILE__, __LINE__,
                    "Could not connect to process with address %d", pcv->cv_members[i]);                                
            }
        }
    }
}
