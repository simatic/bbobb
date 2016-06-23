/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   bbDumpMsg.h
 * Author: simatic
 *
 * Created on 23 juin 2016, 09:05
 */

#ifndef _BB_DUMP_MSG_H
#define _BB_DUMP_MSG_H

#include "applicationMessage.h"
#include "bbMsg.h"

void bbDumpBatch(BbBatch *pBatch, unsigned int currentIndent);

void bbDumpBbMsg(BbMsg *pBbMsg, unsigned int currentIndent);

#endif /* _BB_DUMP_MSG_H */

