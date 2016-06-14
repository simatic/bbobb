/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

#include "interface.h"
#include "bbCallBack.h"
#include "bbInterface.h"
#include "management_addr.h"
#include "iomsg.h"
#include "bbStateMachine.h"
#include "bbError.h"
#include "bbSingleton.h"
#include "trains.h"
#include "bbobb.h"

sem_t *bbSem_init_done;

int bbErrno;

/*int main(int argc, char* argv[]){
    bbInit();
    
    return 0;
}*/

int bbInit(int batchMaxLen, int waitNb, int waitTime, CallbackCircuitChange aCallbackCircuitChange, CallbackODeliver aCallbackODeliver, t_reqOrder reqOrder){
    
    bbErrno = 0;
    
    bbErrno = bbSingletonInit(aCallbackCircuitChange, aCallbackODeliver, reqOrder);
    if(bbErrno) {
        bbErrorAtLineWithoutErrnum(EXIT_FAILURE,
                                    __FILE__,
                                    __LINE__,
                                    "bbAutomatonStateInit error with singletonInit");
    }
    
    pthread_t OdeliveriesThread;
    bbErrno = pthread_create(&OdeliveriesThread, NULL, bbODeliveries, NULL);
    if(bbErrno) {
        bbErrorAtLineWithoutErrnum(EXIT_FAILURE,
                                   __FILE__,
                                   __LINE__,
                                   "bbODeliveries, error with oDeliveries threadInit");
    }
   
    bbErrno=bbAutomatonInit();
    if(bbErrno) {
        bbErrorAtLineWithoutErrnum(EXIT_FAILURE,
                                    __FILE__,
                                    __LINE__,
                                    "bbAutomatonStateInit error with AutomatonInit");        
    }
    
    bbErrno=trInit(0, 0, 0, 0, bbCallbackCircuitChange, bbCallbackODeliver, UNIFORM_TOTAL_ORDER);
    if(bbErrno){
        bbErrorAtLineWithoutErrnum(EXIT_FAILURE,
                                    __FILE__,
                                    __LINE__,
                                    "bbAutomatonStateInit error with trInit");
    }
    
    return 0;   
}
