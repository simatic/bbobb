/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "bbCallBack.h"
#include "bbComm.h"
#include "bbSingleton.h"
#include "bbSignalArrival.h"

void callbackCircuitChange(circuitView *pcv){
    bbSingleton.view = *pcv;
    connectToViewMembers(pcv);
    if (addrIsMine(pcv->cv_members[0])){
        // As current process is first member, it is responsible to bbOBroadcast
        // view changes
        bbSignalArrivalDepartures(
            addrIsNull(pcv->cv_departed) ? AM_ARRIVAL : AM_DEPARTURE,
            pcv);
    }
}

void CallBackODeliver(address sender, t_typ messageType, message * mp) {
    
}