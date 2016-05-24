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
static BbBatch* rcvdBatch[][16];//[sizeview][maxwave] TO DO : refactor to BbBatchInSharedMsg
static BbSet* waitingSets[];//size?
static unsigned char waveMax;
BbState bbAutomatonState;


//TEMP
BbBatchInSharedMsg* getBatchInSharedMsg(BbSharedMsg *sharedMsg, BbBatchInSharedMsg * lastReturnedBatch, int rankSet);
static unsigned int newView();
int nbBatch(BbSet);
int rcvdBatchWave(int);
//VIEWID DANS LES MESSAGES?

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
    
    rcvdBatch = NULL;
    
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
  //  offset += pMsg->msg.body.recover.sets[0].len;
   // set = (BbSet*)offset;

    BbBatch pBatch=NULL;
    if (pMsg->msg.body.recover.view == bbSingleton.view) {
        nbRecoverRcvd++;
        if(pMsg->msg.body.recover.initDone){
            if(!(bbSingleton.initDone)){
            //viewIdInLastSignificantRecover = senderViewId
            //viewinlastblabla -> on affecte directement au singleton dans le cas ou pas initdone
                pMsg->msg.body.recover.viewId = bbSingleton.viewId;
            }
            //nop(recoverRcvd[nbRecoverRcvd])->(body.recover.viewId)=pMsg->(body.recover.viewId);
            if(pMsg->msg.body.recover.sets[0].wave >= bbSingleton.currentWave){//a voir comment acceder au set
                //foreach batch in set1.batches do
                do{
                    pBatch=getBatchInSharedMsg(pMsg, pBatch, 0);//delete?
                    if(rcvdBatch[(pMsg->msg.body.recover.sets[0].wave)][pBatch.sender]==NULL){
                        rcvdBatch[(pMsg->msg.body.recover.sets[0].wave)][pBatch.sender]==pBatch;
                    }
                    
                }while(pBatch!=NULL);
             
               // for(i=0;i<nbBatch(pMsg->msg.body.recover.sets[0]);i++){
            // if rcvdBatch[set1.wave].get(batch.sender) == null then
                 //   if(1){                 // We did not already receive the batch
               //    rcvdBatch[set1.wave].put(batch.sender,batch)
                //    pMsg->(body.recover.sets[0].batches[i]
                        
                        
                        //SI POINTEUR NON STOCKE, LIBERER LE POINTEUR :deletebatchinsharedmsg
                 //   }
               // }

            }
            if(pMsg->msg.body.recover.sets[1].wave > bbSingleton.currentWave){//a voir comment acceder au set
                
                waveMax = pMsg->msg.body.recover.sets[1].wave;

            }
            pBatch=NULL;
            do{
                pBatch=getBatchInSharedMsg(pMsg, pBatch, 1);
                if(rcvdBatch[(pMsg->msg.body.recover.sets[1].wave)][pBatch.sender]==NULL){
                    rcvdBatch[(pMsg->msg.body.recover.sets[1].wave)][pBatch.sender]==pBatch;
                }
                    
            }while(pBatch!=NULL); 
            deleteBatchInSharedMsg({pBatch,pMsg});
        }    
        else{
            bbSingleton.currentWave=pMsg->msg.body.recover.sets[1].wave;
            waveMax=pMsg->msg.body.recover.sets[1].wave;
        }
            if (nbRecoverRcvd == bbSingleton.viewSize){
            //forceDeliver()
            //sendBatchForStep0()
            // traiter les waiting set
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
    rcvdBatch=malloc((bbSingleton.viewSize)*waveMax*sizeof(BbBatch)); 
    waitingSets = [];
    bbSingleton.view = newView();// TODO a definir le calcul de la nouvelle vue
    if (bbSingleton.initDone){
        bbSingleton.viewId += 1;
    }
    else{
        bbSingleton.viewId = 0;
    }
    if (bbSingleton.viewSize == 1){ 
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

BbState bbSaveSet(BbState state, BbSharedMsg* pMsg){//PB VUE
    //if state ?
    if (pMsg->msg.view == bbSingleton.view || !(bbSingleton.initDone)){
        //waitingSets = waitingSets + set
    }
  return BB_STATE_ALONE; // TODO : Put the correct return value
}


