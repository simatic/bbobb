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

sem_t *bbSem_init_done;

int bbErrno;

/*int main(int argc, char* argv[]){
    bbInit();
    
    return 0;
}*/

int bbInit(){
    
    bbErrno = 0;
    
    if(bbSingletonInit()) {
        bbErrorAtLineWithoutErrnum(EXIT_FAILURE,
                                    __FILE__,
                                    __LINE__,
                                    "bbAutomatonStateInit error with singletonInit");
    }
   
    bbErrno=bbAutomatonInit();
    if(bbSingleton) {
        bbErrorAtLineWithoutErrnum(EXIT_FAILURE,
                                    __FILE__,
                                    __LINE__,
                                    "bbAutomatonStateInit error with AutomatonInit");        
    }
    
    bbErrno=trInit(0, 0, 0, 0, CallbackCircuitChange, CallbackODeliver, UNIFORM_TOTAL_ORDER);
    if(bbErrno){
        bbErrorAtLineWithoutErrnum(EXIT_FAILURE,
                                    __FILE__,
                                    __LINE__,
                                    "bbAutomatonStateInit error with trInit");
    }
    
    return 0;   
}
