/*******************************************************************************
*       @brief      Header File for system WDT functions.
*       @file       Downhole/inc/wdt.h
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef WDT_H
#define WDT_H

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    ///@brief
    ///@param
    ///@return
    void StartIWDT(void);

    ///@brief
    ///@param
    ///@return
    void KickWatchdog(void);

#ifdef __cplusplus
}
#endif
#endif