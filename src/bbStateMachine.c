#include "bbError.h"
#include "bbStateMachine.h"
#include "bbMsg.h"
#include "bbComm.h"
#include "bbSingleton.h"
#include <error.h>
#include <stddef.h>
#include <pthread.h>
#include <unistd.h>

// VAR STATICS setrecovered[], nbSetRcvd], waitingset
// VAR EXTERNES view, sizeView, viewId, wavemax

// fonctions à faire: sizeview, newview
//A ajouter = remplir waiting set, queue, etc


//BbBatchInSharedMsg* rcvdBatch[MAX_MEMB][waveMax];//[MAX_MEMB][maxwave]
static int nbRecoverRcvd; 
static bool rcvdSet[MAX_MEMB][WAVE_MAX];
static trList *waitingSharedMsg;
//BbState bbAutomatonState;
//int addrToIndex();


//TEMP
//BbBatchInSharedMsg* getBatchInSharedMsg(BbSharedMsg *sharedMsg, BbBatchInSharedMsg * lastReturnedBatch, int rankSet);
//static circuitView newView();


BbState bbError(BbState, BbSharedMsg*);
BbState bbProcessRecover(BbState, BbSharedMsg*);
BbState bbProcessSet(BbState, BbSharedMsg*);
BbState bbProcessViewChange(BbState, BbSharedMsg*);
BbState bbSaveSet(BbState, BbSharedMsg*); //A VOIR

BbStateMachineFunc bbTransitions[BB_LAST_STATE+1][BB_LAST_MSG+1] = {
  /*      State  /  Received msg :    BB_MSG_RECOVER             BB_MSG_SET                 BB_MSG_VIEW_CHANGE            */
  /* BB_STATE_ALONE              */ { bbError,                   bbError,                   bbProcessViewChange },
  /* BB_STATE_SEVERAL            */ { bbError,                   bbProcessSet,              bbProcessViewChange },
  /* BB_STATE_VIEW_CHANGE        */ { bbProcessRecover,          bbSaveSet,                 bbProcessViewChange }
};

int bbAutomatonInit(){
    int i,j, error = 0;
    waitingSharedMsg = newList();
    for (i = 0; i<MAX_MEMB; i++) {
        for (j = 0; j < waveMax; j++) {
            rcvdBatch[i][j] = NULL;
        }
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
    pthread_detach(msgTreatementThread);
    
    pthread_t waitCommForAcceptThread;
    error = pthread_create(&waitCommForAcceptThread, NULL, waitCommForAccept, NULL);
    if(error){
    bbErrorAtLine(  EXIT_FAILURE,
                    error,
                    __FILE__,
                    __LINE__,
                    "bbAutomatonStateInit error waitCommForAcceptThread");
    }
    
    
    printf("AutomatonInit : OK\n");
        
    return 0;
}

void * bbMsgTreatement(void* unused){

    BbSharedMsg * sharedMsg = NULL;
    do {
        sharedMsg = (BbSharedMsg*)bqueueDequeue(bbSingleton.msgQueue);
        
        bbStateMachineTransition(sharedMsg);
    }while(1);
}


void bbStateMachineTransition(BbSharedMsg* pSharedMsg){
  if ( (pSharedMsg->msg.type < 0) || (pSharedMsg->msg.type > BB_LAST_MSG) ) {
    bbErrorAtLineWithoutErrnum(EXIT_FAILURE,
			       __FILE__,
			       __LINE__,
			       "pMsg->type == %d which is outside interval [%d,%d]",
			       pSharedMsg->msg.type,
			       0,
			       BB_LAST_MSG);
  }
  if ( (bbSingleton.automatonState < 0) || (bbSingleton.automatonState > BB_LAST_STATE) ) {
    bbErrorAtLineWithoutErrnum(EXIT_FAILURE,
			       __FILE__,
			       __LINE__,
			       "bbAutomatonState == %d which is outside interval [%d,%d]",
			       bbSingleton.automatonState,
			       0,
			       BB_LAST_STATE);
  }
  bbSingleton.automatonState = (*bbTransitions[bbSingleton.automatonState][pSharedMsg->msg.type])(bbSingleton.automatonState, pSharedMsg);
}

BbState bbError(BbState state, BbSharedMsg* pSharedMsg){
  bbErrorAtLineWithoutErrnum(EXIT_FAILURE,
			     __FILE__,
			     __LINE__,
			     "%s function has been called because state machine received unexpected message %d in state %d\n",
			     __func__,
			     state,
			     pSharedMsg->msg.type);
  return BB_STATE_ALONE; 
}
BbState bbProcessRecover(BbState state, BbSharedMsg* pSharedMsg) {//A CHANGER?
    char* offset;
    offset = (char*) pSharedMsg + offsetof(BbSharedMsg, msg.body.recover.sets[0]);
    offset += pSharedMsg->msg.body.recover.sets[0].len + offsetof(BbSetInRecover, set);
    BbSet* pSet = (BbSet*) offset;

    BbBatchInSharedMsg* pBatch = NULL;
    BbBatchInSharedMsg* pPreviousBatch = NULL;
    if (pSharedMsg->msg.body.recover.viewId == bbSingleton.viewId) {
        nbRecoverRcvd++;
        if (pSharedMsg->msg.body.recover.initDone) {
            if (!(bbSingleton.initDone)) {
                //viewIdInLastSignificantRecover = senderViewId
                //viewinlastblabla -> on affecte directement au singleton dans le cas ou pas initdone
                bbSingleton.viewId = pSharedMsg->msg.body.recover.viewId;
            }
            //nop(recoverRcvd[nbRecoverRcvd])->(body.recover.viewId)=pMsg->(body.recover.viewId);
            if (pSharedMsg->msg.body.recover.sets[0].set.wave >= bbSingleton.currentWave) {//a voir comment acceder au set
                //foreach batch in set1.batches do
                do {

                    pBatch = getBatchInSharedMsg(pSharedMsg, pPreviousBatch, 0);

                    if (rcvdBatch[(pSharedMsg->msg.body.recover.sets[0].set.wave)][pBatch->batch->sender] == NULL) {
                        rcvdBatch[(pSharedMsg->msg.body.recover.sets[0].set.wave)][pBatch->batch->sender] == pBatch;
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
                pBatch = getBatchInSharedMsg(pSharedMsg, pBatch, 1);
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
            //forceDeliver()
            //sendBatchForStep0()
            do {
                pSet = (BbSharedMsg*) listRemoveFirst(waitingSharedMsg); //TO DO : modifier :'(
                bbSaveSet(state, pSharedMsg);
            } while (pSet != NULL);

            if (nbRecoverRcvd == 1) {
                return BB_STATE_ALONE;
            }
            return BB_STATE_SEVERAL;
        }

    }
    return BB_STATE_VIEW_CHANGE;
}

BbState bbProcessSet(BbState state, BbSharedMsg* pSharedMsg) {
    //set de données à traiter
    //a placer dans une queue
    //creer tableau de double pointeur
    return BB_STATE_ALONE; // TODO : Put the correct return value
}

BbState bbProcessViewChange(BbState state, BbSharedMsg* pSharedMsg) {

    nbRecoverRcvd = 0;
    cleanList(waitingSharedMsg);
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
    if (pSharedMsg->msg.body.set.viewId == bbSingleton.viewId || !(bbSingleton.initDone)) {
        listAppend(waitingSharedMsg, (void *) pSharedMsg);
    }
    return state; // TODO : Put the correct return value
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
        for (j = 0; j < waveMax; j++) {
            rcvdBatch[i][j] = NULL;
                    rcvdSet[i][j] = false;
        }
    }

    bbSingleton.initDone = true;
    if (waveMax > bbSingleton.currentWave + 2) {
        bbErrorAtLineWithoutErrnum(EXIT_FAILURE,
                __FILE__,
                __LINE__,
                "forceDeliver problem\n",
                __func__);

    }

    bbSingleton.currentWave = waveMax + 1;
}

void sendBatchForStep0(){
    bbSingleton.currentStep=0;

    MUTEX_LOCK(bbSingleton.batchToSendMutex);
    BbSharedMsg *sharedSet = bbSingleton.batchToSend->sharedMsg;
    if (sharedSet->msg.body.set.batches[0].len > sizeof(BbBatch)) {
        rcvdBatch[bbSingleton.currentWave][addrToRank(myAddress)] = newBatchInSharedMsg(sharedSet->msg.body.set.batches, sharedSet);
        sharedSet->msg.len = offsetof(BbMsg,body.set.batches) + sharedSet->msg.body.set.batches[0].len;
    } else {
        sharedSet->msg.len = offsetof(BbMsg,body.set.batches);
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
