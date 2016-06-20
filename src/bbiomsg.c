/**
 Trains Protocol: Middleware for Uniform and Totally Ordered Broadcasts
 Copyright: Copyright (C) 2010-2012
 Contact: michel.simatic@telecom-sudparis.eu

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 3 of the License, or any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 USA

 Developer(s): Michel Simatic, Arthur Foltz, Damien Graux, Nicolas Hascoet, Nathan Reboud
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <pthread.h>
#include <string.h>
#include <stddef.h>

#include "bbError.h"
#include "bbiomsg.h"
#include "bbMsg.h"

BbSharedMsg * bbReceive(trComm * aComm){
  BbSharedMsg * msgExt = NULL;
  int nbRead, nbRead2;
  int len;

  nbRead = commReadFully(aComm, &len, sizeof(len));
  if (nbRead == sizeof(len)) {
    msgExt = newBbSharedMsg(len);
    msgExt->msg.len = len;
    nbRead2 = commReadFully(aComm,(char*)&(msgExt->msg.type), (len - nbRead));
    if (nbRead2 == len - nbRead) {
      return msgExt;
    } else {
      deleteBbSharedMsg(msgExt);
      return NULL;
    }
  }
  return NULL;
}
