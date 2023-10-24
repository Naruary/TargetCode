/*!
********************************************************************************
*       @brief      This is the prototype file for SecureParameters.c.
*       @file       Uphole/inc/DataManagers/SecureParameters.h
*       @author     Bill Kamienik
*       @author     Josh Masters
*       @date       December 2014
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
#include "rtc.h"
#include "timer.h"
#include "language.h"
#include "LoggingManager.h"

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

#define TIME_METER_SCALE        10 // 0.1 hour
#define TIME_METER_ROLLOVER     (100000 * TIME_METER_SCALE)
#define TENTH_HOURS_TO_MINUTES  6

#define MAX_USER_LINES          3
#define MAX_USER_DATA_BYTES     20
#define DEFAULT_USER_DATA       "********************"

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
    PROD_ID_MWD = 0x11,
    PROD_ID_EM  = 0x12,
} PRODUCT_ID_TYPE;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    ///@brief  
    ///@param  
    ///@return 
    BOOL SecureParametersInitialized(void);
    
    ///@brief  
    ///@param  
    ///@return 
    void ServiceNVRAMSecureParameters(void);
    
    ///@brief  
    ///@param  
    ///@return 
    void RepairCorruptSU(void);
    
    ///@brief  
    ///@param  
    ///@return 
    void DefaultNVParams(U_BYTE nStorageUnit);

    ///@brief  
    ///@param  
    ///@return 
    BOOL PendingSecureParameterWrite(void);

// BEGIN IDENT STRUCT ACCESSORS
    
    ///@brief  
    ///@param  
    ///@return 
    void SetSerialNumber(char* serialNumber);
    
    ///@brief  
    ///@param  
    ///@return 
    char* GetSerialNumber(void);

    ///@brief  
    ///@param  
    ///@return 
    void SetDeviceOwner(char* owner);
    
    ///@brief  
    ///@param  
    ///@return 
    char* GetDeviceOwner(void);
    
    ///@brief  
    ///@param  
    ///@return 
    void SetModelNumber(char* modelNumber);
    
    ///@brief  
    ///@param  
    ///@return 
    char* GetModelNumber(void);
    
    ///@brief  
    ///@param  
    ///@return 
    BOOL DomesticUnit(void);
    
    ///@brief  
    ///@param  
    ///@return 
    BOOL InternationalLanguage(void);

    ///@brief  
    ///@param  
    ///@return 
    void SetValidLanguage(LANGUAGE_SETTING eTestLanguage, BOOL bAvailable);
    
    ///@brief  
    ///@param  
    ///@return 
    BOOL IsValidLanguage(LANGUAGE_SETTING eTestLanguage);
    
    ///@brief  
    ///@param  
    ///@return 
    PRODUCT_ID_TYPE GetProductID(void);
    
    ///@brief  
    ///@param  
    ///@return 
    void SetProductID(PRODUCT_ID_TYPE nID);

// END IDENT STRUCT ACCESSORS

// BEGIN CONFIG STRUCT ACCESSORS

    ///@brief  
    ///@param  
    ///@return 
    BOOL SetCurrentLanguage(LANGUAGE_SETTING eSetting);
    
    ///@brief  
    ///@param  
    ///@return 
    LANGUAGE_SETTING CurrentLanguage(void);

    ///@brief  
    ///@param  
    ///@return 
    void SetBacklightAvailable(BOOL bState);
    
    ///@brief  
    ///@param  
    ///@return 
    BOOL GetBacklightAvailable(void);

    ///@brief  
    ///@param  
    ///@return 
    void SetBuzzerAvailable(BOOL bState);
    
    ///@brief  
    ///@param  
    ///@return 
    BOOL GetBuzzerAvailable(void);

    ///@brief  
    ///@param  
    ///@return 
    void SetDefaultPipeLength(INT16 length);
    
    ///@brief  
    ///@param  
    ///@return 
    INT16 GetDefaultPipeLength(void);

    ///@brief  
    ///@param  
    ///@return 
    void SetDeclination(INT16 value);
    
    ///@brief  
    ///@param  
    ///@return 
    INT16 GetDeclination(void);

    ///@brief  
    ///@param  
    ///@return 
    void SetDesiredAzimuth(INT16 value);
    
    ///@brief  
    ///@param  
    ///@return 
    INT16 GetDesiredAzimuth(void);
    
    ///@brief  
    ///@param  
    ///@return 
    void SetToolface(INT16 value);
    
    ///@brief  
    ///@param  
    ///@return 
    INT16 GetToolface(void);

    ///@brief  
    ///@param  
    ///@return 
    void SetCheckShot(BOOL value);
    
    ///@brief  
    ///@param  
    ///@return 
    BOOL GetCheckShot(void);
    
    ///@brief  
    ///@param  
    ///@return 
    void SetBoreholeName(char* value);
    
    ///@brief  
    ///@param  
    ///@return 
    char* GetBoreholeName(void);
    
// END CONFIG STRUCT ACCESSORS

    ///@brief  
    ///@param  
    ///@return 
    void SetLoggingState(STATE_OF_LOGGING newState);
    
    ///@brief  
    ///@param  
    ///@return 
    STATE_OF_LOGGING GetLoggingState(void);
    void SetDownholeOffTime(INT16 length);
    INT16 GetDownholeOffTime(void);
    void SetDownholeOnTime(INT16 length);
    INT16 GetDownholeOnTime(void);
    void SetDeepSleepMode(BOOL value);
    BOOL GetDeepSleepMode(void);
    void SetGammaOnOff(BOOL bState);
    BOOL GetGammaOnOff(void);
    REAL32 GetDownholeBatteryLife(void);
    void SetDownholeBatteryLife(REAL32);

    void Set90DegErr(INT16 value);
    INT16 Get90DegErr(void);
    void Set270DegErr(INT16 value);
    INT16 Get270DegErr(void);
    void SetMaxErr(INT16 value);
    INT16 GetMaxErr(void);
    
    INT16 n90err(void);
    INT16 n270err(void);
    INT16 nmaxerr(void);

// END OPSTATE STRUCT ACCESSORS

// BEGIN METERS STRUCT ACCESSORS
// END METERS STRUCT ACCESSORS

#ifdef __cplusplus
}
#endif
#endif