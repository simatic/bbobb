#include "bbError.h"
#include "bbStateMachine.h"
#include "bbMsg.h"
#include "bbComm.h"
#include "bbSingleton.h"
#include <error.h>

// VAR STATICS setrecovered[], nbSetRcvd], waitingset
// VAR EXTERNES view, sizeView, viewId, wavemax

// fonctions à faire: sizeview, newview
//A ajouter = remplir waiting set, queue, etc
static int nbRecoverRcvd; 
static BbBatchInSharedMsg* rcvdBatch[][]=malloc((MAX_MEMB)*waveMax*sizeof(BbBatchInSharedMsg));;//[MAX_MEMB][maxwave]
static bool rcvdSet[][];//[MAX_MEMB][maxwave]
static trList *waitingSets = newList();
static unsigned char waveMax;
BbState bbAutomatonState;


//TEMP
BbBatchInSharedMsg* getBatchInSharedMsg(BbSharedMsg *sharedMsg, BbBatchInSharedMsg * lastReturnedBatch, int rankSet);
static unsigned int newView();


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
        msg = bqueueDequeue(bbSingleton.msgQueue);
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


BbState bbProcessRecover(BbState state, BbSharedMsg* pMsg){//A CHANGER?
    char* offset;
    offset = (char*)pMsg + offsetof(BbSharedMsg, body.recover.sets[0]);
    offset += pMsg->msg.body.recover.sets[0].len + offsetof(BbSetInRecover, set);
    BbSet* pSet = (BbSet*)offset;

    BbBatchInSharedMsg* pBatch=NULL;
    BbBatchInSharedMsg* pPreviousBatch=NULL;
    if (pMsg->msg.body.recover.view == bbSingleton.view) {
        nbRecoverRcvd++;
        if(pMsg->msg.body.recover.initDone){
            if(!(bbSingleton.initDone)){
            //viewIdInLastSignificantRecover = senderViewId
            //viewinlastblabla -> on affecte directement au singleton dans le cas ou pas initdone
                pMsg->msg.body.recover.viewId = bbSingleton.viewId;
            }
            //nop(recoverRcvd[nbRecoverRcvd])->(body.recover.viewId)=pMsg->(body.recover.viewId);
            if(pMsg->msg.body.recover.sets[0].set.wave >= bbSingleton.currentWave){//a voir comment acceder au set
                //foreach batch in set1.batches do
                do{
                    
                    pBatch=getBatchInSharedMsg(pMsg, pPreviousBatch, 0);
                    
                    if(rcvdBatch[(pMsg->msg.body.recover.sets[0].set.wave)][pBatch->batch.sender]==NULL){
                        rcvdBatch[(pMsg->msg.body.recover.sets[0].set.wave)][pBatch->batch.sender]==pBatch.batch;
                    }
                    else{
                        deleteBatchInSharedMsg(pPreviousBatch);
                    }
                    pPreviousBatch=pBatch;
                }while(pBatch!=NULL);
             
            }
            if(pSet.wave > bbSingleton.currentWave){//>
                
                waveMax = pSet.wave;

            }
            pBatch=NULL;
            do{
                pBatch=getBatchInSharedMsg(pMsg, pBatch, 1);
                if(rcvdBatch[(pSet.wave)][pBatch->batch.sender]==NULL){
                    rcvdBatch[(pSet.wave)][pBatch->batch.sender]==pBatch;
                }
                else{
                    deleteBatchInSharedMsg(pPreviousBatch);
                }
                    
            }while(pBatch!=NULL); 
            
        }    
        else{
            bbSingleton.currentWave=pSet.wave;
            waveMax=pSet.wave;
        }
            if (nbRecoverRcvd == bbSingleton.view->cv_nmemb){
            //forceDeliver()
            //sendBatchForStep0()
                do{
                    pSet=(BbSet*) listRemoveFirst(waitingSets); 
                    bbSaveSet(state,{pSet,NULL});
                }while(pSet!=NULL);
                    
                if (nbRecoverRcvd == 1){
                return BB_STATE_ALONE;
                }
            return BB_STATE_SEVERAL;
            }
        
    }
  return BB_STATE_VIEW_CHANGE; 
}
        

BbState bbProcessSet(BbState state, BbSharedMsg* pMsg){
  //set de données à traiter
    //a placer dans une queue
    //creer tableau de double pointeur
  return BB_STATE_ALONE; // TODO : Put the correct return value
}

BbState bbProcessViewChange(BbState state, BbSharedMsg* pMsg){
  
    nbRecoverRcvd=0; 
    waitingSets = [];
    bbSingleton.view = newView();// TODO a definir le calcul de la nouvelle vue
    if (bbSingleton.initDone){
        bbSingleton.viewId += 1;
    }
    else{
        bbSingleton.viewId = 0;
    }
    if (bbSingleton.view->cv_nmemb == 1){ 
        if (bbSingleton.initDone) {
            waveMax = bbSingleton.currentWave;
            //forceDeliver();
        }
        else{
            bbSingleton.initDone = true ;
        }
    } 
    else{
        waveMax = bbSingleton.currentWave;
        nbRecoverRcvd = 0;     
//APPEL TO-Broadcast(RECOVER,
    
    }
  return BB_STATE_VIEW_CHANGE; 
}

BbState bbSaveSet(BbState state, BbSharedMsg* pMsg){
    if (pMsg->msg.body.set.viewId == bbSingleton.viewId || !(bbSingleton.initDone)){
        waitingSets = listAppend(waitingSets, (void *) pMsg->msg.body.set);
    }
  return state ; // TODO : Put the correct return value
}


void forceDeliver() {
    int i,j;
  
    if (bbSingleton.initDone){
        if(bbSingleton.reqOrder == UNIFORM_TOTAL_ORDER){
            //foreach key in rcvdBatch[wave-1].keys() do
            //O-deliver(rcvdBatch[wave-1].get(key).msgs)
            
        }
        //foreach key in rcvdBatch[wave].keys() do
        //si causal order -> suprrimer
        //if req_order != CAUSAL_ORDER or (reqOrder == CAUSAL_ORDER and delivBatch.get(key) != true) then
        //O-deliver(rcvdBatch[wave].get(key).msgs) A FAIRE TOUT LE TEMPS
        if(bbSingleton.currentWave < waveMax){//pas sûre
            //foreach key in rcvdBatch[waveMax].keys() do
            //O-deliver(rcvdBatch[waveMax].get(key).msgs) -> bqueueEnqueue 
        }   
    }
    for(i=0; i<; i++){
        for(j=0; j<waveMax; j++){
            rcvdBatch[i][j]=NULL;
        }
    }

    bbSingleton.initDone=true;
/*
38:    Mettre à false tous les éléments de setRcvd
39:    delivBatch = []
42:    if waveMax > wave + 2 then
43:       Arreter l'execution en signalant une erreur
44:    end if
45:    wave = waveMax + 1
*/
}
//revoir tout avec     MAX_MEMB