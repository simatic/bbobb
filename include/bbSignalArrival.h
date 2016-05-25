/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/**
 * @brief Manage the messages which participate to the protocol on an applicative level
 * @file bbSignalArrival.h
 * @author Michel SIMATIC
 * @date 25 may 2016
 */

#ifndef BBSIGNALARRIVAL_H
#define BBSIGNALARRIVAL_H

#include "trains.h"

/**
 * @brief Adds one application message to wagon @a w for arrived or gone process @a ad (this is determined by @a typ variable)
 * @param[in] typ Type of message to add in @a header.typ of the message
 * @param[in] pcv pointer on the circuit view to be bbOBroadcast
*/
void bbSignalArrivalDepartures(char typ, circuitView *pcv);

#endif /* BBSIGNALARRIVAL_H */

