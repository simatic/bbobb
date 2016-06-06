//#include "bbError.h"
#include "bbStateMachine.h"
#include "bbMsg.h"
#include "bbComm.h"
#include "bbSingleton.h"
#include <error.h>

// VAR STATICS setrecovered[], nbSetRcvd], waitingset
// VAR EXTERNES view, sizeView, viewId, wavemax

// fonctions à faire: sizeview, newview
//A ajouter = remplir waiting set, queue, etc
unsigned char NB_WAVE;
unsigned char waveMax;

BbBatchInSharedMsg* rcvdBatch[MAX_MEMB][NB_WAVE];//[MAX_MEMB][maxwave]
static int nbRecoverRcvd; 
static bool rcvdSet[MAX_MEMB][NB_WAVE];
static trList *waitingSharedSets = newList();
BbState bbAutomatonState;
//int addrToIndex();


//TEMP
BbBatchInSharedMsg* getBatchInSharedMsg(BbSharedMsg *sharedMsg, BbBatchInSharedMsg * lastReturnedBatch, int rankSet);
static unsigned int newView();


BbState bbError(BbState, BbSharedMsg*);
BbState bbProcessRecover(BbState, BbSharedMsg*);
BbState bbProcessSet(BbState, BbSharedMsg*);
BbState bbProcessViewChange(BbState, BbSharedMsg*);
//BbState bbSaveSet(BbState, BbSharedMsg*); A VOIR

BbStateMachineFunc bbTransitions[BB_LAST_STATE+1][BB_LAST_MSG+1] = {
  /*      State  /  Received msg :    BB_MSG_RECOVER             BB_MSG_SET                 BB_MSG_VIEW_CHANGE            */
  /* BB_STATE_ALONE              */ { bbError,                   bbError,                   bbProcessViewChange },
  /* BB_STATE_SEVERAL            */ { bbError,                   bbProcessSet,              bbProcessViewChange },
  /* BB_STATE_VIEW_CHANGE        */ { bbProcessRecover,          bbSaveSet,                 bbProcessViewChange }
};

int bbAutomatonInit(){
    int i,j, error = 0;
    
        for (i = 0; i<MAX_MEMB; i++) {
        for (j = 0; j < NB_WAVE; j++) {
            rcvdBatch[i][j] = NULL;
        }
    }
    
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

BbState bbError(BbState state, BbSharedMsg* pMsg){
  bbErrorAtLineWithoutErrnum(EXIT_FAILURE,
			     __FILE__,
			     __LINE__,
			     "%s function has been called because state machine received unexpected message %d in state %d\n",
			     __func__,
			     state,
			     pMsg->msg.type);
  return BB_STATE_ALONE; 
}
BbState bbProcessRecover(BbState state, BbSharedMsg* pMsg) {//A CHANGER?
    char* offset;
    offset = (char*) pMsg + offsetof(BbSharedMsg, body.recover.sets[0]);
    offset += pMsg->msg.body.recover.sets[0].len + offsetof(BbSetInRecover, set);
    BbSet* pSet = (BbSet*) offset;

    BbBatchInSharedMsg* pBatch = NULL;
    BbBatchInSharedMsg* pPreviousBatch = NULL;
    if (pMsg->msg.body.recover.viewId == bbSingleton.viewId) {
        nbRecoverRcvd++;
        if (pMsg->msg.body.recover.initDone) {
            if (!(bbSingleton.initDone)) {
                //viewIdInLastSignificantRecover = senderViewId
                //viewinlastblabla -> on affecte directement au singleton dans le cas ou pas initdone
                pMsg->msg.body.recover.viewId = bbSingleton.viewId;
            }
            //nop(recoverRcvd[nbRecoverRcvd])->(body.recover.viewId)=pMsg->(body.recover.viewId);
            if (pMsg->msg.body.recover.sets[0].set.wave >= bbSingleton.currentWave) {//a voir comment acceder au set
                //foreach batch in set1.batches do
                do {

                    pBatch = getBatchInSharedMsg(pMsg, pPreviousBatch, 0);

                    if (rcvdBatch[(pMsg->msg.body.recover.sets[0].set.wave)][pBatch->batch->sender] == NULL) {
                        rcvdBatch[(pMsg->msg.body.recover.sets[0].set.wave)][pBatch->batch->sender] == pBatch;
                    } else {
                        deleteBatchInSharedMsg(pPreviousBatch);
                    }
                    pPreviousBatch = pBatch;
                } while (pBatch != NULL);

            }
            if (pSet->wave > bbSingleton.currentWave) {//>

                waveMax = pSet->wave;

            }
            pBatch = NULL;
            do {
                pBatch = getBatchInSharedMsg(pMsg, pBatch, 1);
                if (rcvdBatch[(pSet->wave)][pBatch->batch->sender] == NULL) {
                    rcvdBatch[(pSet->wave)][pBatch->batch->sender] == pBatch;
                } else {
                    deleteBatchInSharedMsg(pPreviousBatch);
                }

            } while (pBatch != NULL);

        }
        else {
            bbSingleton.currentWave = pSet->wave;
            waveMax = pSet->wave;
        }
        if (nbRecoverRcvd == bbSingleton.view.cv_nmemb) {
            forceDeliver();
            sendBatchForStep0();
            do {

                pMsg = (BbSharedMsg*) listRemoveFirst(waitingSharedSets); 
                freceive(pMsg);
            } while (pMsg != NULL);


            if (nbRecoverRcvd == 1) {
                return BB_STATE_ALONE;
            }
            return BB_STATE_SEVERAL;
        }

    }
    return BB_STATE_VIEW_CHANGE;
}


BbState bbProcessSet(BbState state, BbSharedMsg* pSharedMsg) {
     
     int i;
     while (rcvdSet[bbSingleton.currentWave,bbSingleton.currentStep-1] && 1<<bbSingleton.currentStep <= bbSingleton.view.cv_nmemb){
        BbSharedMsg* newMsg = createSet(bbSingleton.currentWave);
    
        //buildNewSet();

//Fsend(newSet) to p_{i+2^step}
        deleteBbSharedMsg(newMsg);
        bbSingleton.currentStep++;
     }
    if(1<<bbSingleton.currentStep <= bbSingleton.view.cv_nmemb){
        if (bbSingleton.reqOrder == TOTAL_ORDER){
            for (i = 0; i < MAX_MEMB; i++) {
                if (rcvdBatch[i][bbSingleton.currentWave] != NULL){ 
                    //O-deliver(rcvdBatch[wave].get(key).msgs)
                    printf("odeliver");
                }
            }
        }        
        if (bbSingleton.reqOrder == UNIFORM_TOTAL_ORDER){
            for (i = 0; i < MAX_MEMB; i++) {
                if (rcvdBatch[i][bbSingleton.currentWave-1] != NULL){ 
                    //O-deliver(rcvdBatch[wave].get(key).msgs)
                    printf("odeliver");
                }
            }
        }
        for (i = 0; i < MAX_MEMB; i++) {
            rcvdBatch[i][bbSingleton.currentWave-1] = NULL;
            
        }
/**
59:       for each key in rcvdBatch[wave-1].keys() do
60:           free(rcvdBatch[wave-1].get(key)) 
61: 	 rcvdBatch[wave-1].remove(key)*/

    //delivBatch = [];
    bbSingleton.currentWave ++;
    sendBatchForStep0();
    //processPendingSets()??
     }
    return BB_STATE_SEVERAL; // TODO : Put the correct return value

}

BbState bbProcessViewChange(BbState state, BbSharedMsg* pMsg) {

    nbRecoverRcvd = 0;
    cleanList(waitingSharedSets);
    bbSingleton.view = newView(); // TODO a definir le calcul de la nouvelle vue
    if (bbSingleton.initDone) {
        bbSingleton.viewId += 1;
    } else {
        bbSingleton.viewId = 0;
    }
    if (bbSingleton.view.cv_nmemb == 1) {
        if (bbSingleton.initDone) {
            waveMax = bbSingleton.currentWave;
            //forceDeliver();
        } else {
            bbSingleton.initDone = true;
        }
    }
    else {
        waveMax = bbSingleton.currentWave;
        nbRecoverRcvd = 0;
        //APPEL TO-Broadcast(RECOVER,

    }
    return BB_STATE_VIEW_CHANGE;
}


BbState bbSaveSet(BbState state, BbSharedMsg* pSharedMsg) {

/*    rcvdSet[set.wave,set.step] = true //?
 7:    foreach batch in set.batches do
 8:       rcvdBatch[set.wave].put(batch.sender,batch)

13:    end for
14:    processPendingSets()*/
        BbBatchInSharedMsg* pBatch = NULL;
        pBatch = getBatchInSharedMsg(pSharedMsg, pBatch, 1);
        do {
            pBatch = getBatchInSharedMsg(pSharedMsg, pBatch, 1);
            rcvdBatch[(pSharedMsg.msg.body.set->wave)][pBatch->batch->sender] == pBatch;
        } while (pBatch != NULL);
            //bbProcessSet(state, pSharedMsg)
    return state; // TODO : Put the correct return value
}

void freceive(BbSharedMsg* pSharedMsg) {
    if (pSharedMsg->msg.body.set.viewId == bbSingleton.viewId || !(bbSingleton.initDone)) {
        listAppend(waitingSharedSets, (void *) pSharedMsg);

    }
}

void forceDeliver() {
    int i, j;

    if (bbSingleton.initDone) {


        if (bbSingleton.reqOrder == UNIFORM_TOTAL_ORDER) {

            for (i = 0; i < MAX_MEMB; i++) {//foreach key in rcvdBatch[wave-1].keys() do
                if (rcvdBatch[i][bbSingleton.currentWave - 1] != NULL){ // TODO : METTRE A NULL
                        //O-deliver(rcvdBatch[wave-1].get(key).msgs)
                        printf("odeliver");
                    }
            }
        }
        for (i = 0; i < MAX_MEMB; i++) {
            if (rcvdBatch[i][bbSingleton.currentWave] != NULL){ // TODO : METTRE A NULL
                    //O-deliver(rcvdBatch[wave].get(key).msgs)
                    printf("odeliver");
                }
        }
        //foreach key in rcvdBatch[wave].keys() do
        //si causal order -> suprrimer
        //if req_order != CAUSAL_ORDER or (reqOrder == CAUSAL_ORDER and delivBatch.get(key) != true) then
        //O-deliver(rcvdBatch[wave].get(key).msgs) A FAIRE TOUT LE TEMPS
        if (bbSingleton.currentWave < waveMax) {

            //foreach key in rcvdBatch[waveMax].keys() do
            for (i = 0; i < MAX_MEMB; i++) {
                if (rcvdBatch[i][bbSingleton.currentWave] != NULL){ // TODO : METTRE A NULL
                        //O-deliver(rcvdBatch[waveMax].get(key).msgs) -> bqueueEnqueue
                        printf("odeliver");
                    }

            }
        }
    }

    for (i = 0; i<MAX_MEMB; i++) {
        for (j = 0; j < NB_WAVE; j++) {
            rcvdBatch[i][j] = NULL;
                    rcvdSet[i][j] = false;
        }
    }

    bbSingleton.initDone = true;
    if (waveMax > bbSingleton.currentWave + 2) {
        bbErrorAtLineWithoutErrnum(EXIT_FAILURE,
                __FILE__,
                __LINE__,
                "forceDeliver problem\n");

    }

    bbSingleton.currentWave = waveMax + 1;
}


void sendBatchForStep0(){
    bbSingleton.currentStep=0;

    MUTEX_LOCK(bbSingleton.batchToSendMutex);
    BbSharedMsg *sharedSet = bbSingleton.batchToSend->sharedMsg;
    if (sharedSet->msg.body.set.batches[0].len > sizeof(BbBatch)) {
        rcvdBatch[bbSingleton.currentWave][addrToRank(myAddress)] = newBatchInSharedMsg(sharedSet->msg.body.set.batches, sharedSet);
        sharedSet->msg.len = offsetof(BbMsg,msg.body.set.batches) + sharedSet->msg.body.set.batches[0].len;
    } else {
        sharedSet->msg.len = offsetof(BbMsg,msg.body.set.batches);
    }
    bbSingleton.batchToSend = newEmptyBatchInNewSharedMsg(bbSingleton.batchMaxLen);//modif
    MUTEX_LOCK(bbSingleton.batchToSendMutex);
    pthread_cond_signal(&(bbSingleton.batchToSendCond));

    sharedSet->msg.body.set.viewId = bbSingleton.viewId;
    sharedSet->msg.body.set.wave = bbSingleton.currentWave;
    sharedSet->msg.body.set.step = bbSingleton.currentStep;

    // commWrite(leCommCorrespondantAuDestinataire, &(sharedSet->msg.body.set), sharedSet->msg.len);
    // Evidemment, il faut tester le code retour de commWritev

    deleteBbSharedMsg(sharedSet);

    bbSingleton.currentStep++;

}



 void processPendingSets(){
    
     int i;
     while (rcvdSet[bbSingleton.currentWave,bbSingleton.currentStep-1] && 1<<bbSingleton.currentStep <= bbSingleton.view.cv_nmemb){
        BbSharedMsg* newMsg = createSet(bbSingleton.currentWave);
    
        //buildNewSet();

//Fsend(newSet) to p_{i+2^step}
        deleteBbSharedMsg(newMsg);
        bbSingleton.currentStep++;
     }
    if(1<<bbSingleton.currentStep <= bbSingleton.view.cv_nmemb){
        if (bbSingleton.reqOrder == TOTAL_ORDER){
            for (i = 0; i < MAX_MEMB; i++) {
                if (rcvdBatch[i][bbSingleton.currentWave] != NULL){ 
                    //O-deliver(rcvdBatch[wave].get(key).msgs)
                    printf("odeliver");
                }
            }
        }        
        if (bbSingleton.reqOrder == UNIFORM_TOTAL_ORDER){
            for (i = 0; i < MAX_MEMB; i++) {
                if (rcvdBatch[i][bbSingleton.currentWave-1] != NULL){ 
                    //O-deliver(rcvdBatch[wave].get(key).msgs)
                    printf("odeliver");
                }
            }
        }
        for (i = 0; i < MAX_MEMB; i++) {
            rcvdBatch[i][bbSingleton.currentWave-1] = NULL;
            
        }
/**
59:       for each key in rcvdBatch[wave-1].keys() do
60:           free(rcvdBatch[wave-1].get(key)) 
61: 	 rcvdBatch[wave-1].remove(key)*/

    //delivBatch = [];
    bbSingleton.currentWave ++;
    sendBatchForStep0();
    //processPendingSets()??
     }
     }

