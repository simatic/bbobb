/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <string.h>
#include <stdio.h>
#include "errorTrains.h"

#include "bbSingleton.h"
#include "bbStateMachine.h"
#include "bbMsg.h"

message *bbNewmsg(int payloadSize) {
    message *mp;
    MUTEX_LOCK(bbSingleton.batchToSendMutex);

    // We check that we have enough space for the message the caller wants to allocate
    while ((bbSingleton.batchToSend->batch->len + sizeof(messageHeader) + payloadSize
            > bbSingleton.batchMaxLen)
            && (bbSingleton.batchToSend->batch->len != sizeof(BbBatch))) {
        int rc = pthread_cond_wait(&(bbSingleton.batchToSendCond),
                                   &(bbSingleton.batchToSendMutex));
        if (rc < 0)
            ERROR_AT_LINE(EXIT_FAILURE, rc, __FILE__, __LINE__, "pthread_cond_wait");
    }

    mp = allocInBatchToSend(payloadSize);

    // MUTEX_UNLOCK will be done in bbOBroadcast
    //
    return mp;
}

int bbOBroadcast(t_typ messageTyp, message *mp) {

    if (messageTyp < FIRST_VALUE_AVAILABLE_FOR_MESS_TYP) {
        ERROR_AT_LINE_WITHOUT_ERRNUM(EXIT_FAILURE, __FILE__, __LINE__, "messageType is %d, thus a value which is lower than FIRST_VALUE_AVAILABLE_FOR_MESS_TYP (which is %d)", messageTyp, FIRST_VALUE_AVAILABLE_FOR_MESS_TYP);
    }
    mp->header.typ = messageTyp;

    if (bbSingleton.automatonState == BB_STATE_ALONE) {
        bqueueEnqueue(wagonsToDeliver, wagonToSend);
        wagonToSend = newWiw();
    }

    // Message is already in batchToSend. All we have to do is to unlock the mutex.
    MUTEX_UNLOCK(bbSingleton.batchToSendMutex);
    return 0;
}

void *bbODeliveries(void *null) {
    bool terminate = false;

    do {
        message *mp;
        BbBatchInSharedMsg *batchInSharedMsg = bqueueDequeue(bbSingleton.batchesToDeliver);
        BbBatch *batch = batchInSharedMsg->batch;

        //counters.wagons_delivered++;

        // We analyze all messages in this batch
        for (mp = firstMsgInBatch(batch); mp != NULL; mp = nextMsgInBatch(batch, mp)) {

            //counters.messages_delivered++;
            //counters.messages_bytes_delivered += payloadSize(mp);

            switch (mp->header.typ) {
                case AM_ARRIVAL:
                case AM_DEPARTURE:
                    (*(bbSingleton.callbackCircuitChange))((circuitView*)(mp->payload));
                    break;
                case AM_TERMINATE:
                    terminate = true;
                    break;
                default:
                    (*(bbSingleton.callbackODeliver))(batch->sender, mp->header.typ, mp);
                    break;
            }
        }
        deleteBatchInSharedMsg(batchInSharedMsg);
    } while (!terminate);

    return NULL;
}
