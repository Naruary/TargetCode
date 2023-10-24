/*!
********************************************************************************
*       @brief      Provides prototypes for functions contained in intf0007.c
*                   supporting the "Update Diagnostic Downhole" interface.
*       @file       Uphole/src/RASP/intf0007.c
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
#include "Manager_DataLink.h"
#include "FlashMemory.h"
#include "PeriodicEvents.h"
#include "UtilityFunctions.h"
#include "RecordManager.h"
#include "SysTick.h"
#include "LoggingManager.h"
#include "intf0007.h"
#include "wdt.h"
#include "UI_Frame.h"
#include "UI_DownholeTab.h"
#include "intf0008.h"



//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

TIME_LR tUpdateDownHole = 0;
TIME_LR tUpdateDownHoleSuccess = 0;

///@brief
const CMD_VALIDATION g_diagCmdVal0007[] =
{
        { 0xFF, NULL },                       // UPDATE_DIAG_DATA_SET
};

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//


/*!
********************************************************************************
*       @details
*******************************************************************************/

void UpdateDiagnosticDataInDownholeRequest(void)
{
    U_BYTE response[6];
    U_INT16 nUI16Data = 0;
    BOOL nBYTEData = 0;
    nUI16Data = (U_INT16)GetDownholeOffTime();
    memcpy(response, (const void *)&nUI16Data, 2);
    nUI16Data = (U_INT16)GetDownholeOnTime();
    memcpy(response+2, (const void *)&nUI16Data, 2);
    nBYTEData = GetDeepSleepMode();
    memcpy(response+4, (const void *)&nBYTEData, 1);
    nBYTEData = GetGammaOnOff();
    memcpy(response+5, (const void *)&nBYTEData, 1);
    RASPRequest(DIAGNOSTIC_HANDLER, 0x00, response, 6);
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

void DiagnosticHandler(U_BYTE *pHeader, U_BYTE *pData, U_INT16 nDataLen)
{
    switch (pHeader[CMD_ID_POS])
    {
    case UPDATE_DIAG_DATA_SET:
       if (pData[0] == ACCEPTED)
        {
            SetLoggingState(UPDATE_DOWNHOLE_SUCCESS);
            tUpdateDownHoleSuccess = ElapsedTimeLowRes(0);
            SetDownholeOnStatus(FALSE);
            SetDownholeOffStatus(FALSE);
            RepaintNow(&HomeFrame);
        }
       break;


    default:
        break;
    }
}



/*!
********************************************************************************
*       @details
*******************************************************************************/

TIME_LR GetUpdateDownHoleTimer(void)
{
  return tUpdateDownHole;
}


/*!
********************************************************************************
*       @details
*******************************************************************************/

TIME_LR GetUpdateDownHoleSuccessTimer(void)
{
  return tUpdateDownHoleSuccess;
}
