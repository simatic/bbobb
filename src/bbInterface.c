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
    
    if(bbSingletonInit()) {
    bbErrorAtLineWithoutErrnum(EXIT_FAILURE,
			       __FILE__,
			       __LINE__,
			       "bbAutomatonStateInit error with singletonInit");
    }
   
    bbAutomatonInit();
    
    return 0;   
}
