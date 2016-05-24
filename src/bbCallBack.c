/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "bbCallBack.h"
#include "bbComm.h"
#include "bbSingleton.h"

void callbackCircuitChange(circuitView *pcv){
    bbSingleton.view = *pcv;
    connectToViewMembers(pcv);
}

void CallBackODeliver(address sender, t_typ messageType, message * mp) {
    
}