/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "bbCallBack.h"
#include "bbSingleton.h"
#include "bqueue.h"

void bbCallbackCircuitChange(circuitView *pcv){
    BbSharedMsg *sharedMsg = newBbSharedMsg(sizeof(BbMsg));
    sharedMsg->msg.len  = sizeof(BbMsg);
    sharedMsg->msg.type = BB_MSG_VIEW_CHANGE;
    sharedMsg->msg.body.viewChange.view = *pcv;
    bqueueEnqueue(bbSingleton.msgQueue, (void*)sharedMsg);
}

void bbCallbackODeliver(address sender, t_typ messageType, message * mp) {
    BbMsg * msg = (BbMsg*)(mp->payload);
    BbSharedMsg * sharedMsg = newBbSharedMsg(msg->len);
    memcpy(&(sharedMsg->msg), msg, msg->len);    
    bqueueEnqueue(bbSingleton.msgQueue, (void*)sharedMsg);
}
