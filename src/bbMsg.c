/**
* @brief File holding the types' definition for the messages exchanged between processes
* @file bbMsg.c
* @author May CATTANT - Quentin MONNOT - Victor ROUQUETTE - Michel SIMATIC
* @date 23 february 2016
*/

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "bbMsg.h"
#include "address.h"
#include "bbSingleton.h"
#include "bbStateMachine.h"
#include "bbError.h"
#include "bbSharedMsg.h"

/*
BbBatch initBatch () {
    BbBatch batch;
    
    batch.len=sizeof(batch);   
    //batch.sender;  //A DEFINIR valeur par defaut
    //batch.messages[]=NULL; 
    return batch;
}


BbBatch newBbBatch (address sender, message messages[]) {
    BbBatch batch=initBatch();
    batch.len=sizeof(int)+sizeof(address)+sizeof(messages);   
    batch.sender= sender;
    batch.messages=messages[]; 
    
    return batch;
}
*/

message *firstMsgInBatch(BbBatch *b){
  if (b->len == sizeof(BbBatch))
    return NULL;
  else
    return b->messages;
}

message *nextMsgInBatch(BbBatch *b, message *mp){
  message *mp2 = (message*)((char*)mp + mp->header.len);
  if ((char*)mp2 - (char*)b >= b->len)
    return NULL;
  else
    return mp2;
}