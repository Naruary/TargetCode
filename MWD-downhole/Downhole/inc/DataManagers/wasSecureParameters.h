/*!
********************************************************************************
*       @brief      This is the prototype file for SecureParameters.c.
*                   Documented by Josh Masters Nov. 2014
*       @file       Downhole/inc/DataManagers/SecureParameters.h
*       @author     Bill Kamienik
*       @date       July 2013
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef SECURE_PARAMETERS_H
#define SECURE_PARAMETERS_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"
#include "NVRAM_Server.h"
//#include "rtc.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#define NVDB_CHECKSUM_SIZE       4
#define MAX_MODEL_NUM_BYTES     20
#define MAX_SERIAL_NUM_BYTES    20
#define MAX_DEVICE_OWNER_BYTES  20
#define DEFAULT_MODEL_NUM       "********************"
#define DEFAULT_SERIAL_NUM      "********************"
#define DEFAULT_DEVICE_OWNER    "********************"

#define MAX_USER_LINES          3
#define MAX_USER_DATA_BYTES     20
#define DEFAULT_USER_DATA       "********************"

#define MODEM_OFF_TIME          120
#define MODEM_ON_TIME           60

//
// NV data storage unit index values.
//
#define NV_SU_IDENT         0
#define NV_SU_CONFIG        1
#define NV_SU_OPSTATE       2
#define NV_SU_METERS        3
#define NV_SU_USER          4
#define NV_INVALID_SU       5

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

typedef enum
{
    PROD_ID_BDL = 0x10,
    PROD_ID_MWD = 0x11
} PRODUCT_ID_TYPE; // RASP Product IDs

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    BOOL SecureParametersInitialized(void);
    void ServiceNVRAMSecureParameters(void);
    void RepairCorruptSU(void);
    void DefaultNVParams(U_BYTE nStorageUnit);
    BOOL PendingSecureParameterWrite(void);
#if NVRAM_NEW_WAY == 0
    void SetStartHoleFlag(BOOL bState);
    BOOL GetStartHoleFlag(void);
    void SetStopHoleFlag(BOOL bState);
    BOOL GetStopHoleFlag(void);
    void SetLoggingActiveFlag(BOOL bState);
    BOOL IsLoggingActive(void);
    BOOL GetModemOffState(void);
    void SecPar_SetModemOffState(BOOL);
    void SetDownholeOffTime(INT16);
    INT16 GetDownholeOffTime(void);
    void SetDownholeOnTime(INT16);
    INT16 GetDownholeOnTime(void);
    void SetDeepSleepMode(BOOL);
    BOOL GetDeepSleepMode(void);
    void SetGammaOnOff(BOOL);
    BOOL GetGammaOnOff(void);
    void SetGammaMonitor(BOOL);
    BOOL GetGammaMonitor(void);
#endif

#ifdef __cplusplus
}
#endif
#endif
