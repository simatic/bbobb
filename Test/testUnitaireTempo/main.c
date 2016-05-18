/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: terag
 *
 * Created on May 16, 2016, 6:09 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "bbSharedMsg.h"
#include "bbMsg.h"

/*
 * 
 */
int main(int argc, char** argv) {
    
    int size = 100;
    BbBatch* defaultTestBatch = calloc(1,size);
    defaultTestBatch->sender = 255;
    defaultTestBatch->len = size;
    printf("sizeof defaultTestBatch : %ld\n", sizeof(defaultTestBatch->len));

    int len = sizeof(BbMsg.body.set)+3*size;
    BbMsg* msgSet = malloc(len);
    msgSet->len = len;
    msgSet->type = BB_MSG_SET;
    msgSet->body.set.step = 42;
    msgSet->body.set.wave = 43;
    memcpy(msgSet->body.set.batches,defaultTestBatch,size);
    
    return (EXIT_SUCCESS);
}

