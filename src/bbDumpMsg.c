/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include "bbDumpMsg.h"

char *typ2str[] = {
  "AM_ARRIVAL",
  "AM_DEPARTURE",
  "AM_TERMINATE",
  "AM_RESERVED_FOR_FUTURE_USE_1",
  "AM_RESERVED_FOR_FUTURE_USE_2"
};

#define INDENT_MAX  128
#define INDENT_STEP "  "

char *bbIndent(unsigned int currentIndent) {
    static char s[INDENT_MAX];
    int i;
    strncpy(s, "", INDENT_MAX);
    for (i = 0 ; i < currentIndent ; ++i) {
        strncat(s, INDENT_STEP, INDENT_MAX);
    }
    return s;
}

void bbHexdump(char *pc, int size, unsigned int currentIndent) {
    int len = 0;
    while (len < size) {
        if (len % 16 == 0) {
            printf("%s", bbIndent(currentIndent));                
        }
        printf("%2X ", *(pc + len));
        if ((len != 0) && (len % 4 == 3)) {
            printf("  ");
        }
        if (len % 16 == 15) {
            printf("\n");
        }
        len++;
    }
    if (len % 16 != 0) {
        printf("\n");        
    }
}

void bbDumpView(circuitView *pv, unsigned int currentIndent) {
    int i;
    if (addrIsNull(pv->cv_joined)){
        printf("%sDeparted = #%02d\n", bbIndent(currentIndent), addrToRank(pv->cv_departed));
    } else {
        printf("%sJoined   = #%02d\n", bbIndent(currentIndent), addrToRank(pv->cv_joined));
    }
    printf("%snmemb    = %02d\n", bbIndent(currentIndent), pv->cv_nmemb);
    printf("%smemb     = ", bbIndent(currentIndent));
    for (i = 0 ; i < pv->cv_nmemb ; ++i) {
        printf("#%02d", addrToRank(pv->cv_members[i]));
        if (i + 1 < pv->cv_nmemb) {
            printf(", ");
        }
    }
    printf("\n");
}

void bbDumpMessage(message* pm, unsigned int currentIndent) {
    printf("%slen = %d\n", bbIndent(currentIndent), pm->header.len);    
    printf("%styp = ", bbIndent(currentIndent));
    if (pm->header.typ < FIRST_VALUE_AVAILABLE_FOR_MESS_TYP) {
        printf("%s\n", typ2str[pm->header.typ]);
        if ((pm->header.typ == AM_ARRIVAL) || (pm->header.typ == AM_DEPARTURE)) {
            printf("%sPayload = View\n", bbIndent(currentIndent));
            bbDumpView((circuitView*)(pm->payload), currentIndent+1);
        } else {
            printf("%sEmpty payload\n", bbIndent(currentIndent));
        }
    } else {
        printf("APPLICATIVE_TYPE (value = %d)\n", pm->header.typ);
        if (pm->header.len == offsetof(message, payload)) {
            printf("%sEmpty payload\n", bbIndent(currentIndent));
        } else {
            printf("%sPayload dump in hex\n", bbIndent(currentIndent));
            bbHexdump((char*) (pm->payload), payloadSize(pm), currentIndent + 1);
        }
    }
}

void bbDumpBatch(BbBatch *pBatch, unsigned int currentIndent) {
    printf("%slen    = %d\n", bbIndent(currentIndent), pBatch->len);    
    printf("%ssender = #%02d\n", bbIndent(currentIndent), addrToRank(pBatch->sender));
    if (pBatch->len == offsetof(BbBatch, messages)) {
        printf("%sNo messages\n", bbIndent(currentIndent));        
    } else {
        int i;
        message *pm;
        for (i = 0, pm = pBatch->messages;
                (char*)pm - (char*)pBatch < pBatch->len ;
                ++i, pm = (message*) (((char*) pm) + pm->header.len)) {
            printf("%smessage[%d]\n", bbIndent(currentIndent), i);
            bbDumpMessage(pm, currentIndent + 1);
        }
    }
}

void bbDumpSet(BbMsg *pSet, unsigned int currentIndent){
    printf("%slen    = %d\n", bbIndent(currentIndent), pSet->len);    
    printf("%swave   = %d\n", bbIndent(currentIndent), pSet->body.set.wave);
    printf("%sstep   = %d\n", bbIndent(currentIndent), pSet->body.set.step);
    printf("%sviewId = %d\n", bbIndent(currentIndent), pSet->body.set.viewId);
    if (pSet->len == offsetof(BbSet,batches)) {
        printf("%sNo batches\n", bbIndent(currentIndent));
    } else {
        int i;
        BbBatch *pBatch;
        for (i = 0, pBatch = pSet->body.set.batches;
                (char*)pBatch - (char*)pSet < pSet->len ;
                ++i, pBatch = (BbBatch*) (((char*) pBatch) + pBatch->len)) {
            printf("%sbatch[%d]\n", bbIndent(currentIndent), i);
            bbDumpBatch(pBatch, currentIndent + 1);
        }
    }
}

void bbDumpRecover(BbMsg *pRecover, unsigned int currentIndent){
    int i;
    BbSetInRecover *pSetInRecover;
    printf("%slen      = %d\n", bbIndent(currentIndent), pRecover->len);
    printf("%sinitDone = %s\n", bbIndent(currentIndent), (pRecover->body.recover.initDone?"true":"false"));
    printf("%ssender   = #%02d\n", bbIndent(currentIndent), addrToRank(pRecover->body.recover.sender));
    printf("%snbSets   = %hhd\n", bbIndent(currentIndent), pRecover->body.recover.nbSets);
    for (i = 0 , pSetInRecover = pRecover->body.recover.sets ;
         i < pRecover->body.recover.nbSets ;
         ++i, pSetInRecover = (BbSetInRecover*)(((char*)pSetInRecover)+pSetInRecover->len)){
        printf("%sset[%d]\n", bbIndent(currentIndent), i);        
        bbDumpSet((BbMsg*)pSetInRecover, currentIndent+2);
    }
    printf("%sviewId   = %hhd\n", bbIndent(currentIndent), pRecover->body.recover.viewId);
    printf("%sview\n", bbIndent(currentIndent));
    bbDumpView(&(pRecover->body.recover.view), currentIndent+1);
}

void bbDumpViewChange(BbMsg *pViewChange, unsigned int currentIndent){
    printf("%slen      = %d\n", bbIndent(currentIndent), pViewChange->len);
    printf("%sview\n", bbIndent(currentIndent));
    bbDumpView(&(pViewChange->body.viewChange.view), currentIndent+1);
}

void bbDumpBbMsg(BbMsg *pBbMsg, unsigned int currentIndent) {
    switch(pBbMsg->type){
        case BB_MSG_RECOVER:
            printf("BB_MSG_RECOVER\n");
            bbDumpRecover(pBbMsg, currentIndent);
            break;
        case BB_MSG_SET:
            printf("BB_MSG_SET\n");
            bbDumpSet(pBbMsg, currentIndent);
            break;
        case BB_MSG_VIEW_CHANGE:
            printf("BB_MSG_VIEW_CHANGE\n");
            bbDumpViewChange(pBbMsg, currentIndent);
            break;
        default:
            fprintf(stderr,
                    "%s:%d : Must dump a message with type %d which is unknown\n",
                    __FILE__,
                    __LINE__,
                    pBbMsg->type);
            abort();
    }
    printf("\n");
}