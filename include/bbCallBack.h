/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   bbCallBack.h
 * Author: terag
 *
 * Created on May 23, 2016, 2:34 PM
 */

#ifndef BBCALLBACK_H
#define BBCALLBACK_H

/** 
 * @brief Callback function called by bbobb middleware when there is a change in circuit members
 */
extern CallbackCircuitChange theBbCallbackCircuitChange;

/** 
 * @brief Callback function called by bbobb middleware when it is ready to o-deliver a message 
 * to the application layer
 */
extern CallbackODeliver theBbCallbackODeliver;




#endif /* BBCALLBACK_H */

