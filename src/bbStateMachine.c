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
#include "bbSignalArrival.h"
// VAR STATICS setrecovered[], nbSetRcvd], waitingset
// VAR EXTERNES view, sizeView, viewId, wavemax

// fonctions à faire: sizeview, newview
//A ajouter = remplir waiting set, queue, etc
static unsigned char waveMax;
BbBatchInSharedMsg* rcvdBatch[NB_WAVE][MAX_MEMB];
static int nbRecoverRcvd; 
static bool rcvdSet[NB_WAVE][NB_STEP];
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
  /* BB_STATE_START              */ { bbError,                   bbError,                   bbProcessViewChange },
  /* BB_STATE_VIEW_CHANGE        */ { bbProcessRecover,          bbSaveSet,                 bbProcessViewChange }
};

char *state2str[] = {
    "BB_STATE_ALONE",
    "BB_STATE_SEVERAL",
    "BB_STATE_START",
    "BB_STATE_VIEW_CHANGE"
};

char *msg2str[]= {
    "BB_MSG_RECOVER",
    "BB_MSG_SET",
    "BB_MSG_VIEW_CHANGE"
};

void forceDeliver();
void sendBatchForStep0();

#define PREV_WAVE(wave) (wave > 0 ? wave - 1 : NB_WAVE - 1)
#define NEXT_WAVE(wave) ((wave + 1) % NB_WAVE)

int bbAutomatonInit(){
    int i,j, error = 0;
    bbSingleton.automatonState = BB_STATE_START;
    waitingSharedSets = newList();
    for (i = 0; i < NB_WAVE ; i++) {
        for (j = 0; j < MAX_MEMB ; j++) {
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
  //printf("In state \"%s\", automaton receives message \"%s\"\n", state2str[bbSingleton.automatonState], msg2str[pSharedMsg->msg.type]);
  bbSingleton.automatonState = (*bbTransitions[bbSingleton.automatonState][pSharedMsg->msg.type])(bbSingleton.automatonState, pSharedMsg);
  //printf("   Next state = \"%s\"\n", state2str[bbSingleton.automatonState]);
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

    if (isViewEqual(&(pSharedMsg->msg.body.recover.view), &bbSingleton.view)) {
        
        nbRecoverRcvd++;
        if (pSharedMsg->msg.body.recover.initDone) {
            // We are sure this RECOVER message contains at least one set
            char* offset;
            offset = (char*) pSharedMsg + offsetof(BbSharedMsg, msg.body.recover.sets[0]);
            offset += pSharedMsg->msg.body.recover.sets[0].len + offsetof(BbSetInRecover, set);
            BbSet* pSet = (BbSet*) offset;
            BbBatchInSharedMsg* pBatch;

            if (!(bbSingleton.initDone)) {
                bbSingleton.viewId = pSharedMsg->msg.body.recover.viewId;
            }
            //nop(recoverRcvd[nbRecoverRcvd])->(body.recover.viewId)=pMsg->(body.recover.viewId);
            if (pSharedMsg->msg.body.recover.sets[0].set.wave >= bbSingleton.currentWave) {//a voir comment acceder au set
                //foreach batch in set1.batches do
                for (pBatch = getBatchInSharedMsg(pSharedMsg, NULL, 0) ;
                     pBatch != NULL ;
                     pBatch = getBatchInSharedMsg(pSharedMsg, pBatch, 0) ) {
                    if (rcvdBatch[pSharedMsg->msg.body.recover.sets[0].set.wave][pBatch->batch->sender] == NULL) {
                        rcvdBatch[pSharedMsg->msg.body.recover.sets[0].set.wave][pBatch->batch->sender] = pBatch;
                    } else {
                        deleteBatchInSharedMsg(pBatch);
                    }
                }
            }
            if (pSet->wave > bbSingleton.currentWave) {
                waveMax = pSet->wave;
            }
            for (pBatch = getBatchInSharedMsg(pSharedMsg, NULL, 1) ;
                pBatch != NULL;
                pBatch = getBatchInSharedMsg(pSharedMsg, pBatch, 1)) {
                if (rcvdBatch[pSet->wave][pBatch->batch->sender] == NULL) {
                    rcvdBatch[pSet->wave][pBatch->batch->sender] = pBatch;
                }

                else {
                    deleteBatchInSharedMsg(pBatch);
                }
            }
 

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
    for (myRank = 0 ; !addrIsMine(bbSingleton.view.cv_members[myRank]) ; myRank++) {
    }
    return addrToRank(bbSingleton.view.cv_members[(myRank+ithAfterMe)%bbSingleton.view.cv_nmemb]);
}

void bbProcessPendingSets() {
     
    int i;

    while ((bbSingleton.currentStep > 0) && (rcvdSet[bbSingleton.currentWave][bbSingleton.currentStep-1]) && (1<<bbSingleton.currentStep < bbSingleton.view.cv_nmemb)){
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
    if(1<<bbSingleton.currentStep >= bbSingleton.view.cv_nmemb){
        if (bbSingleton.reqOrder == TOTAL_ORDER){
            for (i = 0; i < MAX_MEMB; i++) {
                if (rcvdBatch[bbSingleton.currentWave][i] != NULL){
                    bqueueEnqueue(bbSingleton.batchesToDeliver, rcvdBatch[bbSingleton.currentWave][i]);
                }
            }
        }        
        else if (bbSingleton.reqOrder == UNIFORM_TOTAL_ORDER){
            for (i = 0; i < MAX_MEMB; i++) {
                if (rcvdBatch[PREV_WAVE(bbSingleton.currentWave)][i] != NULL){ 
                    bqueueEnqueue(bbSingleton.batchesToDeliver, rcvdBatch[PREV_WAVE(bbSingleton.currentWave)][i]);
                }
            }
        }
        for (i = 0; i < NB_STEP; i++) {
            rcvdSet[PREV_WAVE(bbSingleton.currentWave)][i] = false;            
        }
        for (i = 0; i < MAX_MEMB; i++) {
            rcvdBatch[PREV_WAVE(bbSingleton.currentWave)][i] = NULL;
        }
/**
59:       for each key in rcvdBatch[wave-1].keys() do
60:           free(rcvdBatch[wave-1].get(key)) 
61: 	 rcvdBatch[wave-1].remove(key)*/

        //delivBatch = [];
        bbSingleton.currentWave = NEXT_WAVE(bbSingleton.currentWave);
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
    BbBatchInSharedMsg* pBatch;
    rcvdSet[pSharedMsg->msg.body.set.wave][pSharedMsg->msg.body.set.step] = true;
    for (   pBatch = getBatchInSharedMsg(pSharedMsg, NULL, 0) ;
            pBatch != NULL ;
            pBatch = getBatchInSharedMsg(pSharedMsg, pBatch, 0)   ) {
        rcvdBatch[pSharedMsg->msg.body.set.wave][addrToRank(pBatch->batch->sender)] = pBatch;
    }
    if (pSharedMsg->msg.body.set.step == bbSingleton.currentStep) {
        bbSingleton.currentStep++;
    }
    bbProcessPendingSets();
    return state;
}

BbState bbProcessViewChange(BbState state, BbSharedMsg* pSharedMsg) {
    // Miscellaneous initializations
    nbRecoverRcvd = 0;
    cleanList(waitingSharedSets);
    if (bbSingleton.initDone) {
        bbSingleton.viewId += 1;

    } else {
        bbSingleton.viewId += 0;
    }

    // We take into account the new view
    bbSingleton.view = pSharedMsg->msg.body.viewChange.view;
    connectToViewMembers(&(bbSingleton.view));

    // We build a message for the application layer
    if (addrIsNull(bbSingleton.view.cv_joined)) {
        // A process is gone
        if (addrIsMine(bbSingleton.view.cv_members[0])) {
            // As current process is first member, it is responsible to 
            // bbOBroadcast this departure information
            bbSignalArrivalDepartures(AM_DEPARTURE, &(bbSingleton.view));
        }        
    } else {
        // A process has arrived
        if (addrIsMine(bbSingleton.view.cv_joined)){
            // As current process is the joining process, it is responsible to 
            // bbOBroadcast this arrival information
            // If it was another process which bbOBroadcast this arrival
            // information, there would be a risk that this information is
            // stored in a batch which is transmitted thanks to RECOVER
            // messages, thus not seen by our starting process (as initDone
            // might still be false)
            bbSignalArrivalDepartures(AM_ARRIVAL, &(bbSingleton.view));
        }
    }

    // Rest of processing depends if we are alone or  not
    if (bbSingleton.view.cv_nmemb == 1) {
        if (bbSingleton.initDone) {
            waveMax = bbSingleton.currentWave;
            forceDeliver();
        } else {
            bbSingleton.initDone = true;
        }
        // We deliver batchToSend if it is not empty
        MUTEX_LOCK(bbSingleton.batchToSendMutex);
        BbSharedMsg *sharedSet = bbSingleton.batchToSend->sharedMsg;
        if (sharedSet->msg.body.set.batches[0].len > sizeof (BbBatch)) {
            bqueueEnqueue(bbSingleton.batchesToDeliver, newBatchInSharedMsg(sharedSet->msg.body.set.batches, sharedSet));
            bbSingleton.batchToSend = newEmptyBatchInNewSharedMsg(bbSingleton.batchMaxLen);
        }
        MUTEX_UNLOCK(bbSingleton.batchToSendMutex);
        pthread_cond_signal(&(bbSingleton.batchToSendCond));
        return BB_STATE_ALONE;
    }
    else {
        waveMax = bbSingleton.currentWave;
        nbRecoverRcvd = 0;
        buildAndProcess_RECOVER(newmsg, oBroadcast);
        return BB_STATE_VIEW_CHANGE;
    }
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


        if (bbSingleton.reqOrder == UNIFORM_TOTAL_ORDER) {

            for (i = 0; i < MAX_MEMB; i++) {//foreach key in rcvdBatch[wave-1].keys() do
                if (rcvdBatch[PREV_WAVE(bbSingleton.currentWave)][i] != NULL){ // TODO : METTRE A NULL
                        //O-deliver(rcvdBatch[wave-1].get(key).msgs)
                    bqueueEnqueue(bbSingleton.batchesToDeliver, rcvdBatch[PREV_WAVE(bbSingleton.currentWave)][i]);
                    }
            }
        }
        for (i = 0; i < MAX_MEMB; i++) {
            if (rcvdBatch[bbSingleton.currentWave][i] != NULL){ // TODO : METTRE A NULL
                    //O-deliver(rcvdBatch[wave].get(key).msgs)
                bqueueEnqueue(bbSingleton.batchesToDeliver, rcvdBatch[bbSingleton.currentWave][i]);
                }
        }
        //foreach key in rcvdBatch[wave].keys() do
        //si causal order -> suprrimer
        //if req_order != CAUSAL_ORDER or (reqOrder == CAUSAL_ORDER and delivBatch.get(key) != true) then
        //O-deliver(rcvdBatch[wave].get(key).msgs) A FAIRE TOUT LE TEMPS
        if (bbSingleton.currentWave < waveMax) {

            //foreach key in rcvdBatch[waveMax].keys() do
            for (i = 0; i < MAX_MEMB; i++) {
                if (rcvdBatch[bbSingleton.currentWave][i] != NULL){ // TODO : METTRE A NULL
                        //O-deliver(rcvdBatch[waveMax].get(key).msgs) -> bqueueEnqueue
                    bqueueEnqueue(bbSingleton.batchesToDeliver, rcvdBatch[bbSingleton.currentWave][i]);
                    }

            }
        }
    } else {
        bbSingleton.currentWave = waveMax;
    }

    for (i = 0; i < NB_WAVE ; i++) {
        for (j = 0; j < MAX_MEMB ; j++) {
            rcvdBatch[i][j] = NULL;
        }
        for (j = 0; j < NB_STEP ; j++) {
            rcvdSet[i][j] = false;
        }
    }

    bbSingleton.initDone = true;
    if (   ((waveMax > bbSingleton.currentWave) && (waveMax > bbSingleton.currentWave + 2))
        || ((waveMax < bbSingleton.currentWave) && (waveMax + NB_WAVE > bbSingleton.currentWave + 2))){
        bbErrorAtLineWithoutErrnum(EXIT_FAILURE,
                __FILE__,
                __LINE__,
                "forceDeliver problem\n");

    }

    bbSingleton.currentWave = NEXT_WAVE(waveMax);
    printf("At the end of forceDeliver(), bbSingleton.currentWave = %d\n", bbSingleton.currentWave);
}

void sendBatchForStep0() {
    bbSingleton.currentStep = 0;

    MUTEX_LOCK(bbSingleton.batchToSendMutex);
    BbSharedMsg *sharedSet = bbSingleton.batchToSend->sharedMsg;
    if (sharedSet->msg.body.set.batches[0].len > sizeof (BbBatch)) {
        rcvdBatch[bbSingleton.currentWave][addrToRank(myAddress)] = newBatchInSharedMsg(sharedSet->msg.body.set.batches, sharedSet);
        sharedSet->msg.len = offsetof(BbMsg, body.set.batches) + sharedSet->msg.body.set.batches[0].len;
        printf("sharedSet->msg.len = %d\n", sharedSet->msg.len);
    } else {
        sharedSet->msg.len = offsetof(BbMsg, body.set.batches);
    }
    bbSingleton.batchToSend = newEmptyBatchInNewSharedMsg(bbSingleton.batchMaxLen); //modif
    MUTEX_UNLOCK(bbSingleton.batchToSendMutex);
    pthread_cond_signal(&(bbSingleton.batchToSendCond));

    sharedSet->msg.type = BB_MSG_SET;
    sharedSet->msg.body.set.viewId = bbSingleton.viewId;
    sharedSet->msg.body.set.wave = bbSingleton.currentWave;
    sharedSet->msg.body.set.step = bbSingleton.currentStep;

    int rc;
    rc = commWrite(
            bbSingleton.commToViewMembers[rankIthAfterMe(1)],
            &(sharedSet->msg.len),
            sharedSet->msg.len);
    if (rc != sharedSet->msg.len) {
        bbErrorAtLine(EXIT_FAILURE,
                errno,
                __FILE__,
                __LINE__,
                "error on write to a successor");
    }

    deleteBbSharedMsg(sharedSet);
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

BbMsg * createSet(unsigned char waveNum) {
    
    BbMsg * set = NULL;
    
    int lenOfSet;
    int processIndex;
    
    int lenOfBatches = 0;
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
            memcpy((char*)&(set->body.set.batches)+lenOfBatches, rcvdBatch[waveNum][processIndex]->batch, rcvdBatch[waveNum][processIndex]->batch->len);
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
    iov[iovcnt].iov_len = offsetof(BbMsg, body.set.batches);
    pSet->len = iov[iovcnt].iov_len;

    iovcnt++;
    
    int i;
    for(i=0, senderBatchToAdd=myAddress; i < nbBatchesToAdd ; i++, senderBatchToAdd = bbAddrPrec(senderBatchToAdd)) {
        int rank = addrToRank(senderBatchToAdd);
        if(rcvdBatch[bbSingleton.currentWave][rank] != NULL) {
            iov[iovcnt].iov_base = rcvdBatch[bbSingleton.currentWave][rank]->batch;
            iov[iovcnt].iov_len = rcvdBatch[bbSingleton.currentWave][rank]->batch->len;
            pSet->len += iov[iovcnt].iov_len;
            iovcnt++;
        }
    }

    *piovcnt = iovcnt;
}

void buildAndProcess_RECOVER(AllocateMessageFunction allocateMsg, ProcessMessageFunction processMsg) {
    BbMsg * fset = NULL;
    BbMsg * sset = NULL;
    
    if(bbSingleton.initDone) {
        fset = createSet(PREV_WAVE(bbSingleton.currentWave));
        sset = createSet(bbSingleton.currentWave);
    }
    
    int len = offsetof(BbMsg, body.recover.sets) + (bbSingleton.initDone ? fset->len + sset->len : 0);
    message *mp = (*allocateMsg)(len);
    
    BbMsg *msg = (BbMsg*)(mp->payload);
    msg->len = len;
    msg->type = BB_MSG_RECOVER;
    msg->body.recover.sender = myAddress;
    msg->body.recover.view = bbSingleton.view;
    msg->body.recover.initDone = bbSingleton.initDone;
    msg->body.recover.viewId = bbSingleton.viewId;
    if(bbSingleton.initDone) {
        msg->body.recover.nbSets = 2;
        memcpy(msg->body.recover.sets, fset, fset->len);
        memcpy((char*)msg->body.recover.sets + fset->len, sset, sset->len);       
    } else {
        msg->body.recover.nbSets = 0;
    }
    (*processMsg)(FIRST_VALUE_AVAILABLE_FOR_MESS_TYP, mp);
    
    free(fset);
    fset = NULL;
    free(sset);
    sset = NULL;
}