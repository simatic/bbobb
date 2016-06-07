#include "bbError.h"
#include "bbStateMachine.h"
#include "bbMsg.h"
#include "bbComm.h"
#include "bbSingleton.h"
#include <error.h>
#include <stddef.h>
#include <pthread.h>
#include <unistd.h>
#include "bbiomsg.h"
// VAR STATICS setrecovered[], nbSetRcvd], waitingset
// VAR EXTERNES view, sizeView, viewId, wavemax

// fonctions à faire: sizeview, newview
//A ajouter = remplir waiting set, queue, etc
static unsigned char waveMax;
static BbBatchInSharedMsg* rcvdBatch[MAX_MEMB][WAVE_MAX];
static int nbRecoverRcvd; 
static bool rcvdSet[MAX_MEMB][WAVE_MAX];
static trList *waitingSharedSets = NULL;
BbState bbAutomatonState;
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

void forceDeliver();
void sendBatchForStep0();

int bbAutomatonInit(){
    int i,j, error = 0;
    waitingSharedSets = newList();
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
    
    waitingSharedSets = newList();
    
    printf("AutomatonInit : OK\n");
        
    return 0;
}

void * bbMsgTreatement(void* unused){

    BbSharedMsg * sharedMsg = NULL;
    printf("MsgTreatementThread : OK\n");
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
                        rcvdBatch[(pSharedMsg->msg.body.recover.sets[0].set.wave)][pBatch->batch->sender] = pBatch;
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
                    rcvdBatch[(pSet->wave)][pBatch->batch->sender] = pBatch;
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

            if (nbRecoverRcvd == 1) {
                if ((pSharedMsg = (BbSharedMsg*) listRemoveFirst(waitingSharedSets)) != NULL) {
                    bbErrorAtLineWithoutErrnum(EXIT_FAILURE,
			       __FILE__,
			       __LINE__,
			       "Going to BB_STATE_ALONE state and there are waiting sets !");
                }
                return BB_STATE_ALONE;
            }
            bbSingleton.automatonState = BB_STATE_SEVERAL;
            while ((pSharedMsg = (BbSharedMsg*) listRemoveFirst(waitingSharedSets)) != NULL) {
                bbStateMachineTransition(pSharedMsg);
            }
            return BB_STATE_SEVERAL;
        }

    }
    return BB_STATE_VIEW_CHANGE;
}

int rankIthAfterMe(int ithAfterMe) {
    int myRank;
    for (myRank = 0 ; addrIsMine(bbSingleton.view.cv_members[myRank]) ; myRank++) {
    }
    return addrToRank(bbSingleton.view.cv_members[(myRank+ithAfterMe)%bbSingleton.view.cv_nmemb]);
}

void bbProcessPendingSets() {
     
    int i;

    while (rcvdSet[bbSingleton.currentWave][bbSingleton.currentStep-1] && 1<<bbSingleton.currentStep <= bbSingleton.view.cv_nmemb){
        BbMsg newSet;
        struct iovec iov[MAX_MEMB];
        int iovcnt = 0;
        int rc;

        buildNewSet(&newSet, iov, &iovcnt);
        rc = commWritev(
                bbSingleton.commToViewMembers[rankIthAfterMe(1<<bbSingleton.currentStep)],
                iov,
                iovcnt);
        if (rc != newSet.len) {
            bbErrorAtLine(EXIT_FAILURE,
                          errno,
                          __FILE__,
                          __LINE__,
                          "error on write to a successor",
                          1<<bbSingleton.currentStep);
        }
        bbSingleton.currentStep++;
     }
    if(1<<bbSingleton.currentStep <= bbSingleton.view.cv_nmemb){
        if (bbSingleton.reqOrder == BB_TOTAL_ORDER){
            for (i = 0; i < MAX_MEMB; i++) {
                if (rcvdBatch[i][bbSingleton.currentWave] != NULL){
                    bqueueEnqueue(bbSingleton.batchesToDeliver, rcvdBatch[i][bbSingleton.currentWave]);
                }
            }
        }        
        else if (bbSingleton.reqOrder == BB_UNIFORM_TOTAL_ORDER){
            for (i = 0; i < MAX_MEMB; i++) {
                if (rcvdBatch[i][bbSingleton.currentWave-1] != NULL){ 
                    bqueueEnqueue(bbSingleton.batchesToDeliver, rcvdBatch[i][bbSingleton.currentWave-1]);
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
        bbProcessPendingSets();
    }

}

BbState bbProcessSet(BbState state, BbSharedMsg* pSharedMsg) {
/*     Algorithm 2
 6:    rcvdSet[set.wave,set.step] = true
 7:    foreach batch in set.batches do
 8:       rcvdBatch[set.wave].put(batch.sender,batch)

13:    end for
14:    processPendingSets()*/
    BbBatchInSharedMsg* pBatch = NULL;
    rcvdSet[pSharedMsg->msg.body.set.wave][pSharedMsg->msg.body.set.step] = true;
    for (   pBatch = getBatchInSharedMsg(pSharedMsg, pBatch, 1) ;
            pBatch != NULL ;
            pBatch = getBatchInSharedMsg(pSharedMsg, pBatch, 1)   ) {
        rcvdBatch[pSharedMsg->msg.body.set.wave][pBatch->batch->sender] = pBatch;
    }
    bbProcessPendingSets();
    return state;
}

BbState bbProcessViewChange(BbState state, BbSharedMsg* pSharedMsg) {

    nbRecoverRcvd = 0;
    cleanList(waitingSharedSets);
    bbSingleton.view = pSharedMsg->msg.body.recover.view;
    if (bbSingleton.initDone) {
        bbSingleton.viewId += 1;
    } else {
        bbSingleton.viewId = 0;
    }
    if (bbSingleton.view.cv_nmemb == 1) {
        if (bbSingleton.initDone) {
            waveMax = bbSingleton.currentWave;
            forceDeliver();
        } else {
            bbSingleton.initDone = true;
        }
    }
    else {
        waveMax = bbSingleton.currentWave;
        nbRecoverRcvd = 0;
        tOBroadcast_RECOVER();

    }
    return BB_STATE_VIEW_CHANGE;
}


BbState bbSaveSet(BbState state, BbSharedMsg* pSharedMsg) {
    if (pSharedMsg->msg.body.set.viewId == bbSingleton.viewId || !(bbSingleton.initDone)) {
        listAppend(waitingSharedSets, (void *) pSharedMsg);

    }
    return state;
}

void forceDeliver() {
    int i, j;

    if (bbSingleton.initDone) {


        if (bbSingleton.reqOrder == BB_UNIFORM_TOTAL_ORDER) {

            for (i = 0; i < MAX_MEMB; i++) {//foreach key in rcvdBatch[wave-1].keys() do
                if (rcvdBatch[i][bbSingleton.currentWave - 1] != NULL){ // TODO : METTRE A NULL
                        //O-deliver(rcvdBatch[wave-1].get(key).msgs)
                    bqueueEnqueue(bbSingleton.batchesToDeliver, rcvdBatch[i][bbSingleton.currentWave-1]);
                    }
            }
        }
        for (i = 0; i < MAX_MEMB; i++) {
            if (rcvdBatch[i][bbSingleton.currentWave] != NULL){ // TODO : METTRE A NULL
                    //O-deliver(rcvdBatch[wave].get(key).msgs)
                bqueueEnqueue(bbSingleton.batchesToDeliver, rcvdBatch[i][bbSingleton.currentWave]);
                }
        }
        //foreach key in rcvdBatch[wave].keys() do
        //si causal order -> suprrimer
        //if req_order != CAUSAL_ORDER or (reqOrder == CAUSAL_ORDER and delivBatch.get(key) != true) then
        //O-deliver(rcvdBatch[wave].get(key).msgs) A FAIRE TOUT LE TEMPS
        bqueueEnqueue(bbSingleton.batchesToDeliver, rcvdBatch[i][bbSingleton.currentWave]);
        if (bbSingleton.currentWave < waveMax) {

            //foreach key in rcvdBatch[waveMax].keys() do
            for (i = 0; i < MAX_MEMB; i++) {
                if (rcvdBatch[i][bbSingleton.currentWave] != NULL){ // TODO : METTRE A NULL
                        //O-deliver(rcvdBatch[waveMax].get(key).msgs) -> bqueueEnqueue
                    bqueueEnqueue(bbSingleton.batchesToDeliver, rcvdBatch[i][bbSingleton.currentWave]);
                    }

            }
        }
    }

    for (i = 0; i<MAX_MEMB; i++) {
        for (j = 0; j < WAVE_MAX; j++) {
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
        sharedSet->msg.len = offsetof(BbMsg,body.set.batches) + sharedSet->msg.body.set.batches[0].len;
    } else {
        sharedSet->msg.len = offsetof(BbMsg,body.set.batches);
    }
    bbSingleton.batchToSend = newEmptyBatchInNewSharedMsg(bbSingleton.batchMaxLen);//modif
    MUTEX_LOCK(bbSingleton.batchToSendMutex);
    pthread_cond_signal(&(bbSingleton.batchToSendCond));
/*
    sharedSet->msg.body.set.viewId = bbSingleton.viewId;
    sharedSet->msg.body.set.wave = bbSingleton.currentWave;
    sharedSet->msg.body.set.step = bbSingleton.currentStep;
*/
    // commWrite(leCommCorrespondantAuDestinataire, &(sharedSet->msg.body.set), sharedSet->msg.len);
    // Evidemment, il faut tester le code retour de commWritev
        struct iovec iov[MAX_MEMB];
        int iovcnt = 0;
        int rc;

        buildNewSet(&(sharedSet->msg.body.set), iov, &iovcnt);
        rc = commWritev(
                bbSingleton.commToViewMembers[rankIthAfterMe(1<<bbSingleton.currentStep)],
                iov,
                iovcnt);
        if (rc != sharedSet->msg.len) {
            bbErrorAtLine(EXIT_FAILURE,
                          errno,
                          __FILE__,
                          __LINE__,
                          "error on write to a successor",
                          1<<bbSingleton.currentStep);
        }

    deleteBbSharedMsg(sharedSet);

    bbSingleton.currentStep++;

}

address bbAddrPrec(address ad) {
    int i = 0;
    for(i=0; addrIsEqual(ad, bbSingleton.view.cv_members[i]) && i < MAX_MEMB + 1; i++) {
        //find pos of ad in current view
    }
    if(i==MAX_MEMB) {
        bbErrorAtLineWithoutErrnum(EXIT_FAILURE,
                                   __FILE__,
                                   __LINE__,
                                   "error to find address of predecessor, address didn't found");
    }
    
    if(i==0) {
        return bbSingleton.view.cv_members[bbSingleton.view.cv_nmemb - 1];
    } else {
        return bbSingleton.view.cv_members[i - 1];
    }
}

BbMsg * createSet(int waveNum) {
    BbMsg * set = NULL;
    
    int lenOfSet;
    int lenOfBatches;
    int processIndex;
    
    for(processIndex=0; processIndex<MAX_MEMB; processIndex++) {
        if(rcvdBatch[waveNum][processIndex] != NULL) {
            lenOfBatches += rcvdBatch[waveNum][processIndex]->batch->len;
        }
    }
    
    lenOfSet = offsetof(BbMsg, body.set.batches) + lenOfBatches;
    set = malloc(lenOfSet);
    
    //We fill the fields of set
    set->len = lenOfSet;
    set->type = BB_MSG_SET;
    set->body.set.viewId = bbSingleton.viewId;
    set->body.set.wave = waveNum;
    set->body.set.step = 0; //unused in case of RECOVER messages
    
    //We fill the batches of the set
    lenOfBatches = 0;
    for(processIndex=0; processIndex<MAX_MEMB; processIndex++) {
        if(rcvdBatch[waveNum][processIndex] != NULL) {
            memcpy((char*)&(set->body.set.batches)+lenOfBatches, rcvdBatch[waveNum][processIndex], rcvdBatch[waveNum][processIndex]->batch->len);
            lenOfBatches += rcvdBatch[waveNum][processIndex]->batch->len;
        }
    }
    
    return set;
}

void buildNewSet(BbMsg * pSet, struct iovec * iov, int * piovcnt) {

    int iovcnt = 0;
    int senderBatchToAdd = 0;
    int nbBatchesToAdd = ((1 << bbSingleton.currentStep) < bbSingleton.view.cv_nmemb - (1 << bbSingleton.currentStep) ?
                        1 << bbSingleton.currentStep :
                        bbSingleton.view.cv_nmemb - (1 << bbSingleton.currentStep));
    
    pSet->type = BB_MSG_SET;
    pSet->body.set.viewId = bbSingleton.viewId;
    pSet->body.set.wave = bbSingleton.currentWave;
    pSet->body.set.step = bbSingleton.currentStep;
    
    iov[iovcnt].iov_base = pSet;
    iov[iovcnt].iov_len = offsetof(BbSharedMsg, msg.body.set.batches);
    pSet->len = iov[iovcnt].iov_len;

    iovcnt++;
    
    int i;
    for(i=0, senderBatchToAdd=myAddress; i < nbBatchesToAdd ; i++, senderBatchToAdd = bbAddrPrec(senderBatchToAdd)) {
        int rank = addrToRank(senderBatchToAdd);
        if(rcvdBatch[rank][bbSingleton.currentWave] != NULL) {
            iov[iovcnt].iov_base = rcvdBatch[rank][bbSingleton.currentWave];
            iov[iovcnt].iov_len = rcvdBatch[rank][bbSingleton.currentWave]->batch->len;
            pSet->len += iov[iovcnt].iov_len;
            iovcnt++;
        }
    }

    *piovcnt = iovcnt;
}
