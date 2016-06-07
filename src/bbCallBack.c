/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "bbCallBack.h"
#include "bbComm.h"
#include "bbSingleton.h"
#include "../include/bbSignalArrival.h"
#include "bqueue.h"

void bbCallbackCircuitChange(circuitView *pcv){
    bbSingleton.view = *pcv;
    connectToViewMembers(pcv);
    if (addrIsNull(pcv->cv_joined)) {
        // A process is gone
        if (addrIsMine(pcv->cv_members[0])) {
            // As current process is first member, it is responsible to 
            // bbOBroadcast this departure information
            bbSignalArrivalDepartures(AM_DEPARTURE, pcv);
        }        
    } else {
        // A process has arrived
        if (addrIsMine(pcv->cv_joined)){
            // As current process is the joining process, it is responsible to 
            // bbOBroadcast this arrival information
            // If it was another process which bbOBroadcast this arrival
            // information, there would be a risk that this information is
            // stored in a batch which is transmitted thanks to RECOVER
            // messages, thus not seen by our starting process (as initDone
            // might still be false)
            bbSignalArrivalDepartures(AM_ARRIVAL, pcv);
        }
    }
}

void bbCallbackODeliver(address sender, t_typ messageType, message * mp) {
    BbMsg * msg = (BbMsg*)(mp->payload);
    BbSharedMsg * sharedMsg = newBbSharedMsg(msg->len);
    memcpy(&(sharedMsg->msg), msg, msg->len);
    
    bqueueEnqueue(bbSingleton.msgQueue, (void*)sharedMsg);
}
