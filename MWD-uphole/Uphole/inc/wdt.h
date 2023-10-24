/*******************************************************************************
*       @brief      Header File for system WDT functions.
*       @file       Uphole/inc/wdt.h
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef WDT_H
#define WDT_H

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

typedef enum
{
    WDT_20MS_TIMEOUT_VALUE,
    WDT_MAX_TIMEOUT_VALUE,
    WDT_MAX_LIST
}WDT_PARAMETERS;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	void StartIWDT(void);
	void SetWatchdogTimer(WDT_PARAMETERS nParam);
	void KickWatchdog(void);

#ifdef __cplusplus
}
#endif
#endif
