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
    batch.len=sizeof(int)+sizeof(address)+sizeof(message);   
    batch.sender= sender;
    batch.messages[]=messages[]; 
    
    return batch;
}

BbMsg initBbMsg(){
    BbMsg msg;
    msg.len=sizeof(BbMsg);
    msg.type; //A DEF
    msg.body=NULL; //A DEF
  return msg;
}

BbMsg newBbMsg (BB_MType mtype, address sender, circuitView view, bool initDone, unsigned char wave, unsigned char step){
    BbMsg msg = initBbMsg();

    switch(mtype){
        case BB_MSG_RECOVER :
            msg.len=sizeof(unsigned int)+sizeof(BB_MType)+sizeof(BbRecover);//sizeof(address)+sizeof(circuitView)+sizeof(bool)+sizeof(int)+sizeof(BbSet);
            msg.type=mtype;
            msg.body.sender=sender;
            msg.body.view=view;
            msg.body.initDone=initDone;
            msg.body.nbSets=0;
            msg.body.sets[]=NULL;
            break;
        case BB_MSG_VIEW_CHANGE  :
            msg.len=sizeof(unsigned int)+sizeof(BB_MType)+sizeof(BbViewChange);
            msg.type=mtype;
            msg.body.view=view;
            break;
        case BB_MSG_SET :
            msg.len=sizeof(unsigned int)+sizeof(BB_MType)+sizeof(BbSet);
            msg.type=mtype;
            msg.body.wave=wave;
            msg.body.step=step;
            break;
  return(msg);
}

BbSharedMsg initBbSharedMsg () {
    BbSharedMsg smsg;
    
    smsg.prefix;
    smsg.msg=initBbMsg();
    
    return smsg;
}


BbSharedMsg newBbSharedMsg (BB_MType mtype, address sender, circuitView view, bool initDone, unsigned char wave, unsigned char step, ControlSharing prefix){
    BbSharedMsg smsg=initSharedMsg();
    
    smsg.prefix=prefix;
    smsg.msg=newBbMsg(BB_MType mtype, address sender, circuitView view, bool initDone, unsigned char wave, unsigned char step);


  return(msg);

}
