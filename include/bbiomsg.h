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

/**
 * @brief This file gives function to send and receive Msg
 * @file bbiomsg.h
 * @author Damien GRAUX
 * @date 30 may 2012
 */

#ifndef _BBIOMSG_H
#define _BBIOMSG_H

#include "address.h"
#include "management_addr.h"
#include "comm.h"
#include "msg.h"
#include "advanced_struct.h"
#include "connect.h"
#include "bbSingleton.h"
#include "bbSharedMsg.h"

/**
 * @brief Fun used to listen on a trComm
 * @param[in] aComm id of the connection to listen
 * @return A pointer of womim
 * @note If an error occured, returns a Msg with the MType DEFAULT
 * @warning Do not forget to free after the Msg returned
 */
BbSharedMsg * bbReceive(trComm * aComm);

#endif /* _BBIOMSG_H */
