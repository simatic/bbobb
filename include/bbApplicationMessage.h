/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   bbApplicationMessage.h
 * Author: simatic
 *
 * Created on 24 mai 2016, 16:29
 */

#ifndef BBAPPLICATIONMESSAGE_H
#define BBAPPLICATIONMESSAGE_H

#include "applicationMessage.h"

/**
 * @brief Request for a pointer on a new message with a payload of size @a payloadSize
 * @param[in] payloadSize Size requested for the @a payload field of the returned message
 * @return pointer on a message upon successful completion, or NULL if an error occurred 
 * (in which case, @a trErrno is set appropriately)
 */
message *bbNewmsg(int payloadSize);

/**
 * @brief o-broadcast of message @a mp (NB : this is the function to be used by application layer)
 * @param[in] messageTyp This parameter is a @a t_typ field greater or equal to @a FIRST_VALUE_AVAILABLE_FOR_MESS_TYP which can be used by the application  arbitrarily. The intent is that it could be used to name different kinds of data messages so they can be differentiated without looking into the body of the message.
 * @param[in] mp Message to be o-broadcasted
 * @return 0 upon successful completion, or -1 if an error occurred (in which case, @a trErrno is set appropriately)
 */
int bbOBroadcast(t_typ messageTyp, message *mp);

/**
 * @brief o-broadcast of message @a mp, but does not make any check on @messageTyp (NB : must only be used inside BBOBB layer)
 * @param[in] messageTyp This parameter is a @a t_typ field greater or equal to @a FIRST_VALUE_AVAILABLE_FOR_MESS_TYP which can be used by the application  arbitrarily. The intent is that it could be used to name different kinds of data messages so they can be differentiated without looking into the body of the message.
 * @param[in] mp Message to be o-broadcasted
 * @return 0 upon successful completion, or -1 if an error occurred (in which case, @a trErrno is set appropriately)
 */
int bbOBroadcastWithoutMessageTypCheck(t_typ messageTyp, message *mp);

/**
 * @brief Function (to be executed by a thread) responsible for delivering messages stored in
 * @a bbSingleton.batchesToDeliver to application layer
 * @param[in] null Unused parameter
 */
void *bbODeliveries(void *null);

/* It would be more logical to have the following two type in Trains applicationMessage.h
   even though there are used only by BBOBB code */

/** 
 * @brief Type of function used by tOBroadcast_RECOVER() when it needs to allocate a message
 * @note As tOBroadcast_RECOVER() is called with functions defined in applicationMessage.h, it
 *       would be more logicial to have this type defined in applicationMessage.h. But, it is
 *       required only in BBOBB. This is why we have it here.
 */
typedef  message* (*AllocateMessageFunction)(int);

/** 
 * @brief Type of function used by tOBroadcast_RECOVER() when it needs to process a message
 * @note As tOBroadcast_RECOVER() is called with functions defined in applicationMessage.h, it
 *       would be more logicial to have this type defined in applicationMessage.h. But, it is
 *       required only in BBOBB. This is why we have it here.
 */
typedef  int (*ProcessMessageFunction)(t_typ, message*);


#endif /* BBAPPLICATIONMESSAGE_H */

