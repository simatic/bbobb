/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   bbComm.h
 * Author: terag
 *
 * Created on April 21, 2016, 11:37 AM
 */

#ifndef BBCOMM_H
#define BBCOMM_H

#include "bbStateMachine.h"
#include "bbMsg.h"
#include "bbSharedMsg.h"
#include "comm.h"
#include "common.h"

#include <sys/uio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief thread used to manage connection
 * @note it creates new bbConnectionMgt threads
 * @param null
 * @return null
 */
void * waitCommForAccept(void *);

/**
 * @brief thread used to manage a port which receives msg
 * @note initializes a port, read on it and do enQueue of datas read
 * @param null
 * @return null
 */
void * bbConnectionMgt(void * arg);

/**
 * @brief get data on aComm
 * @note uses in bbConnectionMgt
 * @param aComm, Comm to read
 * @return read @a BbSharedMsg
 */
BbSharedMsg * bbReceive(trComm * aComm);

/**
 * @brief Interval to have between Trains ports and BBOBB ports
 */
#define INTERVAL_BETWEEN_TRAINS_AND_BBOBB_PORTS 100

/**
 * @brief Returns the socket port on which BBOBB must accept connections
 * @return Pointer to a string containing the socket port.
 * @note This function reads @a TRAINS_PORT environment variable
 * <ul>
 * <li>If \a NULL is returned, \a errno is positionned.</li>
 * <li>When there is a connect timeout, \a errno is ETIMEDOUT. When the \a hostname/\a port is unreachable, errno is \a ECONNREFUSED or \a ENETUNREACH.</li>
 * </ul>
 */
char *bbGetLocalPort();

/**
 * @brief Connect to members of view @a pcv
 * @param[in] pcv pointer to a view
 * @note Updates contents of @a bbSingleton.commToOtherViewMembers
 */
void connectToViewMembers(circuitView *pcv);

/**
 * @brief function uses to compare two view
 * @param view1 to compare
 * @param view2 to compare
 * @return true if view1 and view2 are equals, false if not
 */
bool isViewEqual(circuitView * view1, circuitView * view2);


#ifdef __cplusplus
}
#endif

#endif /* BBCOMM_H */

