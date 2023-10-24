/*!
********************************************************************************
*       @brief      This module provides support and configuration for the 
*                   STM32 RTC.  Documented by Josh Masters Dec. 2014
*       @file       Uphole/src/RASP/intf0002.c
*       @author     Bill Kamienik
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"
#include "RASP.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#define SET_RTCLK     0
#define GET_RTCLK     1

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

///@brief  
const CMD_VALIDATION g_aCmdVal0002[] =
{
    {0, NULL},                          // Set RTC
    {0, NULL},                          // Get RTC
};

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   Interface0002Handler()
;
; Description:
;   This function implements the RASP interface 0002 (RT Clock).
;
; Parameters:
;   pHeader => pointer to the message ID byte array.
;   pData => pointer to the message data byte array.
;   nDataLen => the number of bytes in the message data byte array.
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void Interface0002Handler(U_BYTE* pHeader, U_BYTE* pData, U_INT16 nDataLen)
{
    switch (pHeader[CMD_ID_POS])
    {
        case SET_RTCLK:
            break;

        case GET_RTCLK:
            break;

        default:
            break;
    }
}