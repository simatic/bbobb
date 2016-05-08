#include "bbError.h"
#include "bbStateMachine.h"
#include "bbMsg.h"
#include <error.h>
// VAR STATICS setrecovered[], nbSetRcvd], waitingset
// VAR EXTERNES view, sizeView, viewId

// fonctions à faire: sizeview, newview
//A ajouter = remplir waiting set, queue, etc
BbState bbAutomatonState = BB_STATE_WAIT_VIEW_CHANGE;

int bbAutomatonInit();
void bbMsgTreatement(trBqueue * msgToTreatQueue);
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

void bbAutomatonInit(){
    int error;
    trComm * bbCommForAccept = NULL;
    trBqueue * bbMsgQueue = newBqueue();
    bbCommForAccept = commNewForAccept(8000);
    
    pthread_t msgTreatementThread;
    error = pthread_create(&msgTreatementThread, NULL, bbMsgTreatement, bbMsgQueue);
    if(!error){
        perror("pthread_create");
        return EXIT_FAILURE;
    }
    
    bbQueueComm QCForAcceptThread;
    QCForAcceptThread.comm = trComm;
    QCForAcceptThread.queue = bbMsgQueue;
    pthread_t waitCommForAcceptThread;
    error = pthread_create(&waitCommForAcceptThread, NULL, waitCommForAccept, QCForAcceptThread);
    if(!error){
        perror("pthread_create");
        return EXIT_FAILURE;
    };
        
    /*TO DO : Others StateMachine Init*/
    
    trInit();
    
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
    //int i;
    if (pMsg->(body.BbRecover.view) == view) {
       //??nbRecoverRcvd++
       // if (pMsg->(body.BbRecover.initDone)){
	// ?? viewIdInLastSignificantRecover = senderViewId
 	// if (pMsg->(body.BbRecover.wave) >= wave){
 	//   for(i=0, i<nbView, i++){
	 //      if (rcvdAgg[set1.wave].get(agg.sender) == null then
		  // We did not already receive the batch

        if (nbRecoverRcvd == sizeView){
        //forceDeliver()
	//? sendAggForStep0()
        // traiter les waiting set
        }
    
  return BB_STATE_ALONE; // TODO : Put the correct return value
}

BbState bbProcessSet(BbState state, BbMsg* pMsg){
  //set de données à traiter
    //a placer dans une queue
    //creer tableau de double pointeur
  return BB_STATE_ALONE; // TODO : Put the correct return value
}

BbState bbProcessViewChange(BbState state, BbMsg* pMsg){
    waitingSets = [];
    view = newView();// TODO a definir le calcul de la nouvelle vue
    if (initDone){
        viewId += 1;
    }
    else{
        viewId = 0;
    }
    if (size(newView) == 1){ //SIZE
        if (initDone) {
            waveMax = wave
            forceDeliver()
        }
        else{
            initDone = true
        }
    else
        waveMax = wave
        nbRecoverRcvd = 0      
//APPEL TO-Broadcast(RECOVER,
  }
  return BB_STATE_VIEW_CHANGE; 
}

BbState bbSaveSet(BbState state, BbMsg* pMsg){
    //if state ?
    if (pMsg->(body.set.view) == view || !initDone){
        //waitingSets = waitingSets + set
    }
  return BB_STATE_ALONE; // TODO : Put the correct return value
}