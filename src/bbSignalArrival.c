/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

#include "bbSignalArrival.h"
#include "bbApplicationMessage.h"

void bbSignalArrivalDepartures(char typ, circuitView *pcv){
  message *mp = bbNewmsg(sizeof(circuitView));

  mp->header.typ = typ;
  *((circuitView*)(mp->payload)) = *pcv;

  bbOBroadcast(typ, mp);
}
