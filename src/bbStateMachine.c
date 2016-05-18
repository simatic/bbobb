#include "bbError.h"
#include "bbStateMachine.h"
#include "bbMsg.h"
#include "bbComm.h"
#include "bbSingleton.h"
#include <error.h>

BbState bbAutomatonState;

BbState bbError(BbState, BbMsg*);
BbState bbProcessRecover(BbState, BbMsg*);
BbState bbProcessSet(BbState, BbMsg*);
BbState bbProcessViewChange(BbState, BbMsg*);
BbState bbSaveSet(BbState, BbMsg*);

BbStateMachineFunc bbTransitions[BB_LAST_STATE+1][BB_LAST_MSG+1] = {
  /*      State  /  Received msg :    BB_MSG_RECOVER             BB_MSG_SET                 BB_MSG_VIEW_CHANGE            */
  /* BB_STATE_ALONE              */ { bbError,                   bbError,                   bbProcessViewChange },
  /* BB_STATE_SEVERAL            */ { bbError,                   bbProcessSet,              bbProcessViewChange },
  /* BB_STATE_VIEW_CHANGE        */ { bbProcessRecover,          bbSaveSet,                 bbProcessViewChange }
};

int bbAutomatonInit(){
    int error = 0;
    
    if(bbSingletonInit()) {
    bbErrorAtLineWithoutErrnum(EXIT_FAILURE,
			       __FILE__,
			       __LINE__,
			       "bbAutomatonStateInit error with singletonInit");
    }
    
    pthread_t msgTreatementThread;
    error = pthread_create(&msgTreatementThread, NULL, bbMsgTreatement, NULL);
    if(error){
    bbErrorAtLine(  EXIT_FAILURE,
                    error,
                    __FILE__,
                     __LINE__,
                    "bbAutomatonStateInit error with msgTreatementThreadInit");
    }
    
    pthread_t waitCommForAcceptThread;
    error = pthread_create(&waitCommForAcceptThread, NULL, waitCommForAccept, NULL);
    if(error){
    bbErrorAtLine(  EXIT_FAILURE,
                    error,
                    __FILE__,
                    __LINE__,
                    "bbAutomatonStateInit error waitCommForAcceptThread");
    };
    
    
    printf("AutomatonInit : OK\n");
        
    return 0;
}

void * bbMsgTreatement(void){ //TO DO rajouter mutex

    BbMsg * msg = malloc(sizeof(BbMsg));
    do {
        msg = bqueueDequeue(bbSingleton->msgQueue);
        //bbStateMachineTransition(msg);
        if(msg!=NULL){
            printf("message reçus !");
            bbStateMachineTransition(msg);
        }
    }while(1);
}


void bbStateMachineTransition(BbMsg* pMsg){
  if ( (pMsg->type < 0) || (pMsg->type > BB_LAST_MSG) ) {
    bbErrorAtLineWithoutErrnum(EXIT_FAILURE,
			       __FILE__,
			       __LINE__,
			       "pMsg->type == %d which is outside interval [%d,%d]",
			       pMsg->type,
			       0,
			       BB_LAST_MSG);
  }
  if ( (bbAutomatonState < 0) || (bbAutomatonState > BB_LAST_STATE) ) {
    bbErrorAtLineWithoutErrnum(EXIT_FAILURE,
			       __FILE__,
			       __LINE__,
			       "bbAutomatonState == %d which is outside interval [%d,%d]",
			       bbAutomatonState,
			       0,
			       BB_LAST_STATE);
  }
  bbAutomatonState = (*bbTransitions[bbAutomatonState][pMsg->type])(bbAutomatonState, pMsg);
}

BbState bbError(BbState state, BbMsg* pMsg){
  bbErrorAtLineWithoutErrnum(EXIT_FAILURE,
			     __FILE__,
			     __LINE__,
			     "%s function has been called because state machine received unexpected message %d in state %d\n",
			     __func__,
			     state,
			     pMsg->type);
  return BB_STATE_ALONE; 
}

BbState bbProcessRecover(BbState state, BbMsg* pMsg){
  // TODO : To be completed
  return BB_STATE_ALONE; // TODO : Put the correct return value
}

BbState bbProcessSet(BbState state, BbMsg* pMsg){
  // TODO : To be completed
  return BB_STATE_ALONE; // TODO : Put the correct return value
}

BbState bbProcessViewChange(BbState state, BbMsg* pMsg){
  // TODO : To be completed
  return BB_STATE_ALONE; // TODO : Put the correct return value
}

BbState bbSaveSet(BbState state, BbMsg* pMsg){
  // TODO : To be completed
  return BB_STATE_ALONE; // TODO : Put the correct return value
}


