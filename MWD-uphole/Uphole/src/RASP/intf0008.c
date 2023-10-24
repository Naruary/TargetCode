/*!
********************************************************************************
*       @brief      Provides prototypes for functions contained in intf0008.c
*                   supporting the "Downhole Status Communicator" interface.
*       @file       Uphole/src/RASP/intf0008.c
*       @author     Walter Rodrigues
*       @date       January 2016
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <string.h>
#include "portable.h"
#include "rtc.h"
#include "RASP.h"
// #include "wdt.h"
//#include "SecureParameters.h"
#include "UtilityFunctions.h"
#include "RecordManager.h"
#include "SysTick.h"
#include "intf0008.h"

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

///@brief
const CMD_VALIDATION g_statusCmdVal0008[] =
{
        { 0xFF, NULL },                       // DownHole ON
        { 0xFF, NULL },                       // DownHole OFF
};

//static U_INT16 ON_TIME = 0;
//static U_INT16 OFF_TIME = 0;
//static BOOL DOWNHOLE_ON_STATUS = FALSE;
//static BOOL DOWNHOLE_OFF_STATUS = FALSE;
//static REAL32 DOWNHOLE_POWER = 0.0;
//static TIME_LR tDownholeStatus = 0;

static U_INT16 nDownholeOnTime = 0;
static U_INT16 nDownholeOffTime = 0;
static BOOL bDownholeOnStatus = FALSE;
static BOOL bDownholeOfStatus = FALSE;
static TIME_LR tDownholeStatus = 0;
REAL32 DownholeVoltage;

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//


/*!
********************************************************************************
*       @details
*******************************************************************************/

void SetDownholeOnTimeStatus(U_BYTE *pHeader, U_BYTE *pData, U_INT16 nDataLen)
{
    nDownholeOnTime = GetUnsignedShort(&pData[0]);
    nDownholeOffTime = 0;
    DownholeVoltage = GetREAL32(&pData[2]);

    bDownholeOnStatus = TRUE;
    bDownholeOfStatus = FALSE;

    tDownholeStatus = ElapsedTimeLowRes(0);
//    SetDownholeBatteryLife(DownholeVoltage);
}


/*!
********************************************************************************
*       @details
*******************************************************************************/

void SetDownholeOffTimeStatus(U_BYTE *pHeader, U_BYTE *pData, U_INT16 nDataLen)
{
    nDownholeOffTime = GetUnsignedShort(&pData[0]);
    nDownholeOnTime = 0;
    DownholeVoltage = GetREAL32(&pData[2]);

    bDownholeOfStatus = TRUE;
    bDownholeOnStatus = FALSE;

    tDownholeStatus = ElapsedTimeLowRes(0);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
 ;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 ; Function:
 ;   DiagnosticHandler()
 ;
 ; Description:
 ;   This function implements the RASP interface 0007 (Update Diagnostic Data).
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

void DownholeStatusHandler(U_BYTE *pHeader, U_BYTE *pData, U_INT16 nDataLen)
{
    switch (pHeader[CMD_ID_POS])
    {
    case DOWNHOLE_ON_CONNECTED:
        SetDownholeOnTimeStatus(pHeader, pData, nDataLen);
        break;

    case DOWNHOLE_OFF_DISCONNECTED:
        SetDownholeOffTimeStatus(pHeader, pData, nDataLen);
        break;

    default:
        break;


    }
}


/*!
********************************************************************************
*       @details
*******************************************************************************/

U_INT16 GetDownholeStatusOnTime(void)
{
  return nDownholeOnTime;
}


/*!
********************************************************************************
*       @details
*******************************************************************************/

U_INT16 GetDownholeStatusOffTime(void)
{
  return nDownholeOffTime;
}


/*!
********************************************************************************
*       @details
*******************************************************************************/

BOOL GetDownholeOnStatus(void)
{
  return bDownholeOnStatus;
}


/*!
********************************************************************************
*       @details
*******************************************************************************/

BOOL GetDownholeOffStatus(void)
{
  return bDownholeOfStatus;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void SetDownholeOnStatus(BOOL Status)
{
  bDownholeOnStatus = Status;
}


/*!
********************************************************************************
*       @details
*******************************************************************************/

void SetDownholeOffStatus(BOOL Status)
{
  bDownholeOfStatus = Status;
}


/*!
********************************************************************************
*       @details
*******************************************************************************/

TIME_LR GetDownholeStatusTimer(void)
{
  return tDownholeStatus;
}


/*!
********************************************************************************
*       @details
*******************************************************************************/

INT16 GetDownholeBatteryLife(void)
{
	return (INT16)DownholeVoltage;
//  return ((INT16)(DOWNHOLE_POWER*100));
}


/*!
********************************************************************************
*       @details
*******************************************************************************/
//void SetDownholeBatteryLife(REAL32 value)
//{
  // Nothing to do;
//}
