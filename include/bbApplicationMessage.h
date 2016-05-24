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

/**
 * @brief Request for a pointer on a new message with a payload of size @a payloadSize
 * @param[in] payloadSize Size requested for the @a payload field of the returned message
 * @return pointer on a message upon successful completion, or NULL if an error occurred 
 * (in which case, @a trErrno is set appropriately)
 */
message *bbNewmsg(int payloadSize);

/**
 * @brief o-broadcast of message @a mp
 * @param[in] messageTyp This parameter is a @a t_typ field greater or equal to @a FIRST_VALUE_AVAILABLE_FOR_MESS_TYP which can be used by the application  arbitrarily. The intent is that it could be used to name different kinds of data messages so they can be differentiated without looking into the body of the message.
 * @param[in] mp Message to be o-broadcasted
 * @return 0 upon successful completion, or -1 if an error occurred (in which case, @a trErrno is set appropriately)
 */
int bbOBroadcast(t_typ messageTyp, message *mp);

/**
 * @brief Function (to be executed by a thread) responsible for delivering messages stored in
 * @a bbSingleton.batchesToDeliver to application layer
 * @param[in] null Unused parameter
 */
void *bbODeliveries(void *null);
#endif /* BBAPPLICATIONMESSAGE_H */

