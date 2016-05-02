/**
* @brief File holding the types' definition for the messages exchanged between processes
* @file bbMsg.c
* @author May CATTANT - Quentin MONNOT - Victor ROUQUETTE - Michel SIMATIC
* @date 23 february 2016
*/

#include "bbMsg.h"

BbBatch initBatch () {
    BbBatch batch;
    
    batch.len=sizeof(batch);   
    batch.sender;  //A DEFINIR valeur par defaut
    batch.messages[]=NULL; 
    return batch;
}


BbBatch newBbBatch (address sender, message messages[]) {
    BbBatch batch=initBatch();
    batch.len=sizeof(int)+sizeof(address)+sizeof(messages);   
    batch.sender= sender;
    batch.messages=messages; 
    
    return batch;
}

BbMsg initBbMsg(){
    BbMsg msg;
    msg.len=sizeof(BbMsg);
    msg.type; //A DEF
    msg.body=NULL; //A DEF
  return msg;
}

BbMsg newBbMsg (BB_MType mtype, address sender){
  
        BbMsg msg = initBbMsg();

    switch(mtype){
        case BB_MSG_RECOVER :
            msg.len=sizeof(unsigned int)+sizeof(BB_MType)+sizeof(BbRecover);//sizeof(address)+sizeof(circuitView)+sizeof(bool)+sizeof(int)+sizeof(BbSet);
            msg.type=mtype;
            msg.body.recover.sender=sender;
            msg.body.recover.view=view;
            msg.body.recover.initDone=initDone;
            msg.body.recover.nbSets=0;
            msg.body.recover.sets[]=NULL;
            break;
        case BB_MSG_VIEW_CHANGE  :
            msg.len=sizeof(unsigned int)+sizeof(BB_MType)+sizeof(BbViewChange);
            msg.type=mtype;
            msg.body.viewChange.view=view;
            break;
        case BB_MSG_SET :
            msg.len=sizeof(unsigned int)+sizeof(BB_MType)+sizeof(BbSet);
            msg.type=mtype;
            msg.body.set.wave=wave;
            msg.body.set.step=step;
            break;
    }
  return(msg);

}

BbSharedMsg initBbSharedMsg () {
    BbSharedMsg smsg;
    
    smsg.prefix;
    smsg.msg=initBbMsg();
    
    return smsg;
}


BbSharedMsg newBbSharedMsg (BB_MType mtype, address sender, ControlSharing prefix){
    BbSharedMsg smsg=initBbSharedMsg();
    
    smsg.prefix=prefix;
    smsg.msg=newBbMsg(BB_MType mtype, address sender);


  return(smsg);

}
