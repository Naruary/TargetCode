/*!
********************************************************************************
*       @brief      This module contains the code to store/retrieve secure
*                   parameters to/from NV Memory.
*
*                   Currently the NV Ram will be used to store two copies of:
*                       Unit Identification data
*                       Unit Configuration data
*                       Operational Status data
*                       Time Meter data
*                       User Defined text data
*
*                   Integer values are stored in Little-Endian format.
*                   Documented by Josh Masters Nov. 2014
*       @file       Downhole/src/DataManagers/SecureParameters.c
*       @author     Bill Kamienik
*       @date       July 2013
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <stdlib.h>
#include <string.h>
#include "portable.h"
//#include "CommDriver_SPI.h"
#include "crc.h"
#include "InterruptEnabling.h"
#include "main.h"
#include "NVRAM_Server.h"
#include "SysTick.h"
#include "RealTimeClock.h"
#include "SecureParameters.h"
#include "wdt.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

enum
{
    IMAGE1,
    IMAGE2,
    MAX_IMAGES
};

enum
{
    GET_IMAGE_1,
    VERIFY_IMAGE_1,
    GET_IMAGE_2,
    VERIFY_IMAGE_2,
    ACCEPT_IMAGE,
    SU_EMPTY
};

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//
//
// The following structures define the data in each of the NVRAM storage units.
// The data in each storage unit is always read and written as a complete unit.
// All fields in each storage unit must be as tightly packed as possible.
//
// IMPORTANT!!
// In order to use external tools to upload and interpret the NVRAM structures
// we need to assure that the CRC locations are always deterministic.  Always
// make a data structure's definition an even number of bytes so that alignment
// bytes are not unexpectedly inserted and offset the CRC address.
// Also make sure multi-byte parameters, E.G., U_INT16, start on an even address.
//
#pragma pack(2)

//
// The IDENT_STRUCT stores values that identify an individual device.  These
// values are entered during production and are not expected to change during
// the life of the unit.
//                                                                              // Byte
typedef struct                                                                  // Count
{
    union
    {
        struct
        {
            unsigned bNotUsed_00                : 1;            //LSB of U_INT32
            unsigned bNotUsed_01                : 1;
            unsigned bNotUsed_02                : 1;
            unsigned bNotUsed_03                : 1;
            unsigned bNotUsed_04                : 1;
            unsigned bNotUsed_05                : 1;
            unsigned bNotUsed_06                : 1;
            unsigned bNotUsed_07                : 1;

            unsigned bNotUsed_08                : 1;
            unsigned bNotUsed_09                : 1;
            unsigned bNotUsed_10                : 1;
            unsigned bNotUsed_11                : 1;
            unsigned bNotUsed_12                : 1;
            unsigned bNotUsed_13                : 1;
            unsigned bNotUsed_14                : 1;
            unsigned bNotUsed_15                : 1;

            unsigned bNotUsed_16                : 1;
            unsigned bNotUsed_17                : 1;
            unsigned bNotUsed_18                : 1;
            unsigned bNotUsed_19                : 1;
            unsigned bNotUsed_20                : 1;
            unsigned bNotUsed_21                : 1;
            unsigned bNotUsed_22                : 1;
            unsigned bNotUsed_23                : 1;

            unsigned bNotUsed_24                : 1;
            unsigned bNotUsed_25                : 1;
            unsigned bNotUsed_26                : 1;
            unsigned bNotUsed_27                : 1;
            unsigned bNotUsed_28                : 1;
            unsigned bNotUsed_29                : 1;
            unsigned bNotUsed_30                : 1;
            unsigned bNotUsed_31                : 1;            //MSB of U_INT32
        } AsBits;
        U_INT32 AsWord;                                         //(same size: 4)    4
    } Flags;

    U_INT32 nInitialTOD; /*must be on even boundary*/                           //  4    8
    U_INT32 nReserved_0;                                                        //  4   12

    U_BYTE sModelNum[MAX_MODEL_NUM_BYTES+1];        // Null-terminated string.  // 21   33
    U_BYTE sSerialNum[MAX_SERIAL_NUM_BYTES+1];      // Null-terminated string.  // 21   54
    U_BYTE sDeviceOwner[MAX_DEVICE_OWNER_BYTES+1];  // Null-terminated string.  // 21   75
    U_BYTE nProductId;                                                          //  1   76

    U_BYTE nBuffer[8];                       // for future expansion if needed  //  8   84
                                                                                //----
} IDENT_STRUCT;                                                                 // 84

//
// The CONFIG_STRUCT stores values that identify the current configuration of
// the device.  These values are modified through the User Interface or by the
// remote reception of data.  These values are, therefore, expected to change
// infrequently and at irregular intervals.
//
typedef struct
{
    union
    {
        struct
        {
            unsigned bAzimuthReversal           : 1;
            unsigned bPitchReversal             : 1;
            unsigned bRollReversal              : 1;
            unsigned bRollRange360              : 1;
            unsigned bGamma                     : 1;
            unsigned bDownholeDeepSleep         : 1;
            unsigned bGammaMonitor              : 1;
            unsigned bNotUsed_07                : 1;

            unsigned bNotUsed_08_31             :24;
        } AsBits;
        U_INT32 AsWord;                                               //(same size: 4)   4
    } Flags;

    INT16   nDownholeOffTime;                                                   //  2
    INT16   nDownholeOnTime;                                                    //  2

    REAL32   nBatteryInputVoltage;                                               //  2

    TIME_RT nSoftIronCalTimestamp;                                              //  4    8
    TIME_RT nHardIronCalTimestamp;                                              //  4   12

    INT16 nAzimuthOffset;                                                       //  2
    INT16 nPitchOffset;                                                         //  2
    INT16 nRollOffset;                                                          //  2

    U_BYTE nBuffer[18];                      // for future expansion if needed  // 22   44
                                                                                //----
} CONFIG_STRUCT;                                                                // 44

//
// The OPSTATE_STRUCT stores values that identify the operational state of the
// device at the time that power was removed from the unit.  These values
// are used to restore the operational state of the unit when power is restored
// to the unit.  In particular, the time stamp field is used to calculate the
// duration of any period during which power is not supplied to the unit.  This
// storage unit is updated every two minutes, but only while the blower is on.
//
typedef struct
{
    union
    {
        struct
        {
            unsigned bStartHole                 : 1;
            unsigned bStopHole                  : 1;
            unsigned bLoggingActive             : 1;
            unsigned bNotUsed_03                : 1;
            unsigned bNotUsed_04                : 1;
            unsigned bNotUsed_05                : 1;
            unsigned bNotUsed_06                : 1;
            unsigned bStateModemIsOff           : 1;

            unsigned bNotUsed_08_31             :24;
        } AsBits;
        U_INT32 AsWord;                                         //(same size: 4)    4
    } Flags;

    TIME_RT tTimeStamp;                                                         //  4   8

    U_BYTE nBuffer[36];                      // for future expansion if needed  // 36  44
                                                                                //----
} OPSTATE_STRUCT;                                                               // 44

//
// The METERS_STRUCT stores values that provide a compliance history of the
// device.  This storage unit is updated every six minutes but only when the
// blower is on.
//
typedef struct
{
    union
    {
        struct
        {
            unsigned bNotUsed_00                : 1;
            unsigned bNotUsed_01                : 1;
            unsigned bNotUsed_02                : 1;
            unsigned bNotUsed_03                : 1;
            unsigned bNotUsed_04                : 1;
            unsigned bNotUsed_05                : 1;
            unsigned bNotUsed_06                : 1;
            unsigned bNotUsed_07                : 1;

            unsigned bNotUsed_08_31             :24;
        } AsBits;
        U_INT32 AsWord;                                         //(same size: 4)    4
    } Flags;

    U_INT32 nMachineTime;                                                       //  4   8

    U_BYTE nBuffer[36];                      // for future expansion if needed  // 36  44
                                                                                //----
} METERS_STRUCT;                                                                // 44

typedef struct                                                                  // Count
{
    U_BYTE sUserLine[MAX_USER_LINES][MAX_USER_DATA_BYTES + 1];     // Null-terminated string.  // 21   21
    U_BYTE nPadding;                                                            //  1   64

    U_BYTE nBuffer[8];                       // for future expansion if needed  //  8   70
                                                                                //----
} USER_DEF_STRUCT;                                                              // 70

typedef struct
{
    U_BYTE nBuffer[20]; /* for future expansion if needed */                    // 20
                                                                                //---
} UNUSED_STRUCT;                                                                // 20

//
// The following structure is the image of all NV data storage units.
// The Chk field following each storage unit stores the CRC for that unit.
// Note that the Chk fields are not referenced in the code by name.  They are
// assumed to follow the structure that they are calculated from and are
// accessed by offset from the beginning of that structure.
//
typedef struct
{
    IDENT_STRUCT   Iden;                                                  //  84
    U_INT32        nIdenChk;                                              //   4
                                                                          //----
} NV_IDENTITY_STRUCT;                                                     //  88

typedef struct
{
    CONFIG_STRUCT  Cnfg;                                                  //  44
    U_INT16        nCnfgChk;                                              //   4
                                                                          //----
} NV_CONFIG_STRUCT;                                                       //  48

typedef struct
{
    OPSTATE_STRUCT OpSt;                                                  //  44
    U_INT16        nOpStChk;                                              //   4
                                                                          //----
} NV_OPSTATE_STRUCT;                                                      //  48

typedef struct
{
    METERS_STRUCT  Mtrs;                                                  //  44
    U_INT32        nMtrsChk;                                              //   4
                                                                          //----
} NV_METERS_STRUCT;                                                       //  48

typedef struct
{
    USER_DEF_STRUCT User;                                                 //  64
    U_INT32 nUserChk;                                                     //   4
                                                                          //----
} NV_USER_DEF_STRUCT;                                                     //  68

//
// Restore default field packing.
//
#pragma pack()

//
// The following structure stores information used to locate the different
// copies of a specific NV data storage unit.
//
// The pDefault field provides the address of the default values structure for
// the storage unit.
//
// The pData field provides the address of the RAM copy of the storage unit.
//
// The nImage1 and nImage2 fields provide the dummy address of the storage unit
// within each NVRAM image.  Only those bits of these dummy addresses that are
// within the EEPROM address range are used.
//
// The nLen field provides the number of bytes in the storage unit and its CRC.
//
typedef struct
{
    U_BYTE* pDefault;
    U_BYTE* pData;
    U_INT32 nImage[MAX_IMAGES];
    U_INT16 nLen;
} NVLOCATE_STRUCT;

typedef struct
{
    // NV data storage unit update flag array.
    // An NV data storage unit is written into NVRAM whenever SUMonitor()
    // finds that the update flag for that storage unit has been set.  The update
    // handler clears the update flag prior to beginning the update process.
    unsigned bNVUpdate                  : 1;                            //  :

    // If a storage unit is found to be corrupted and cannot be repaired, then the
    // corresponding bit within the following field will be set.  Updating of a
    // storage unit will not be allowed while its corrupt SU bit is set.  All
    // corrupt SU bits are cleared by rebooting.  The corrupt SU bit for a specific
    // storage unit is cleared when default values are loaded into that storage unit.
    unsigned bCorruptSU                 : 1;                            //  :

    // Indicates that the Storage Unit has been initialized
    unsigned bInitialized               : 1;                            //  :

    // Indicates that the Storage Unit is empty if both are TRUE
    unsigned bImage1Empty               : 1;                            //  :
    unsigned bImage2Empty               : 1;                            //  :

    // Indicates that current step through the Storage Unit initialization
    unsigned eCurrentInitStep           : 3;                            //  :

    unsigned nUnused8_31                : 24;                           //  :
} NV_STORAGE_STATUS;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

///@brief
///@param
///@return
static BOOL checkForEmptySU(U_BYTE *pData, U_INT16 nLen);

///@brief
///@param
///@return
static void setStorageUnitChecksum(void);

///@brief
///@param
///@return
static void updateSecureParamsImage2(void);

///@brief
///@param
///@return
static void updateSecureParamsDone(void);

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//
//
// To coax the linker into managing our NVRAM memory space, it is necessary to
// declare dummy memory structures in an unused address range.  These structures
// will mirror the contents of NVRAM and will be used to calculate the NVRAM
// address (a byte offset from the beginning of NVRAM) of each element within
// NVRAM.  The base address choosen for the dummy memory section is assumed to
// be on an even 16-bit boundary.  This allows the dummy address to be truncated
// to a 16-bit value in which all of the bits that are outside of the EEPROM
// address range are assured to be zero.
//
// The dummy memory sections will be in "bss".  A linker directive must assign
// the section base address, limit the size of the section to the size of the
// EEPROM, and set the NOCLEAR attribute for the section.  Just to catch the
// error of attempting to initialize anything in this memory, a dummy section in
// "data" is also defined with a size of zero to insure a linker error.  The
// following pragmas select the names of the appropriate dummy sections that are
// defined in the linker directive file.
//
// Separate the Identity and Time Meters from the other data so that changes to
// other storage units will not mandate defaulting the Identity and Time Meters.
//

#pragma section="SECURE_PARAM_BBRAM"
__no_init static NV_IDENTITY_STRUCT m_NVIdentImage1 @ "SECURE_PARAM_BBRAM";
__no_init static NV_CONFIG_STRUCT m_NVConfigImage1 @ "SECURE_PARAM_BBRAM";
__no_init static NV_OPSTATE_STRUCT m_NVOpStateImage1 @ "SECURE_PARAM_BBRAM";
__no_init static NV_METERS_STRUCT m_NVMetersImage1 @ "SECURE_PARAM_BBRAM";
__no_init static NV_USER_DEF_STRUCT m_NVUserImage1 @ "SECURE_PARAM_BBRAM";

#pragma section="SECURE_PARAM_FRAM"
__no_init static NV_IDENTITY_STRUCT m_NVIdentImage2 @ "SECURE_PARAM_FRAM";
__no_init static NV_CONFIG_STRUCT m_NVConfigImage2 @ "SECURE_PARAM_FRAM";
__no_init static NV_OPSTATE_STRUCT m_NVOpStateImage2 @ "SECURE_PARAM_FRAM";
__no_init static NV_METERS_STRUCT m_NVMetersImage2 @ "SECURE_PARAM_FRAM";
__no_init static NV_USER_DEF_STRUCT m_NVUserImage2 @ "SECURE_PARAM_FRAM";

//
// A copy of the NV data storage units is maintained in RAM.
//
static NV_IDENTITY_STRUCT   m_NVIdent;
static NV_CONFIG_STRUCT     m_NVConfig;
static NV_OPSTATE_STRUCT    m_NVOpState;
static NV_METERS_STRUCT     m_NVMeters;
static NV_USER_DEF_STRUCT   m_NVUser;

//
// Default values are provided for all fields of all storage units.
//

static const IDENT_STRUCT m_NVIdentDefault =
{
    FALSE,                               // Unused Bit 0
    FALSE,                               // Unused Bit 1
    FALSE,                               // Unused Bit 2
    FALSE,                               // Unused Bit 3
    FALSE,                               // Unused Bit 4
    FALSE,                               // Unused Bit 5
    FALSE,                               // Unused Bit 6
    FALSE,                               // Unused Bit 7

    FALSE,                               // Unused Bit 8
    FALSE,                               // Unused Bit 9
    FALSE,                               // Unused Bit 10
    FALSE,                               // Unused Bit 11
    FALSE,                               // Unused Bit 12
    FALSE,                               // Unused Bit 13
    FALSE,                               // Unused Bit 14
    FALSE,                               // Unused Bit 15

    FALSE,                               // Unused Bit 16
    FALSE,                               // Unused Bit 17
    FALSE,                               // Unused Bit 18
    FALSE,                               // Unused Bit 19
    FALSE,                               // Unused Bit 20
    FALSE,                               // Unused Bit 21
    FALSE,                               // Unused Bit 22
    FALSE,                               // Unused Bit 23

    FALSE,                               // Unused Bit 24
    FALSE,                               // Unused Bit 25
    FALSE,                               // Unused Bit 26
    FALSE,                               // Unused Bit 27
    FALSE,                               // Unused Bit 28
    FALSE,                               // Unused Bit 29
    FALSE,                               // Unused Bit 30
    FALSE,                               // Unused Bit 31

    0,                                  // Initial TOD
    0,                                  // Session ID

    DEFAULT_MODEL_NUM,                  // Model Number
    DEFAULT_SERIAL_NUM,                 // Serial Number
    DEFAULT_DEVICE_OWNER,               // Device Owner
    (U_BYTE)PROD_ID_BDL,                // Product ID - set to a valid device
                                        // so production diagnostics will
                                        // function properly.
    {0}                                 //lint !e651 Expansion buffer
};

static const CONFIG_STRUCT m_NVConfigDefault =
{
    FALSE,                               // Unused Bit 0
    FALSE,                               // Unused Bit 1
    FALSE,                               // Unused Bit 2
    FALSE,                               // Unused Bit 3
    FALSE,                               // Unused Bit 4
    FALSE,                               // Unused Bit 5
    FALSE,                               // Unused Bit 6
    FALSE,                               // Unused Bit 7

    0,                                   // Unused Bits 8_31

    MODEM_OFF_TIME,                      // Default Off Time
    MODEM_ON_TIME,                       // Default ON Time

    0.0,                                 // Battery Voltage

    0,                                   // Reserved 0
    0,                                   // Reserved 1

    {0}                                  //lint !e651 Expansion buffer
};

static const OPSTATE_STRUCT m_NVOpStateDefault =
{
    FALSE,                               // Unused Bit 0
    FALSE,                               // Unused Bit 1
    FALSE,                               // Unused Bit 2
    FALSE,                               // Unused Bit 3
    FALSE,                               // Unused Bit 4
    FALSE,                               // Unused Bit 5
    FALSE,                               // Unused Bit 6
    FALSE,                               // Unused Bit 7

    0,                                   // Unused Bits 8_31

    1,                                   // Timestamp

    {0}                                  //lint !e651 Expansion buffer
};

static const METERS_STRUCT m_NVMetersDefault =
{
    FALSE,                               // Unused Bit 0
    FALSE,                               // Unused Bit 1
    FALSE,                               // Unused Bit 2
    FALSE,                               // Unused Bit 3
    FALSE,                               // Unused Bit 4
    FALSE,                               // Unused Bit 5
    FALSE,                               // Unused Bit 6
    FALSE,                               // Unused Bit 7

    0,                                   // Unused Bits 8_31

    {0}                                  //lint !e651 Expansion buffer
};

static const USER_DEF_STRUCT m_NVUsersDefault =
{
    {DEFAULT_USER_DATA,                  // User Data Line 1
     DEFAULT_USER_DATA,                  // User Data Line 2
     DEFAULT_USER_DATA},                 // User Data Line 3
    (U_BYTE) 0,                          // Padding
    { 0 }                                //lint !e651 Expansion buffer
};

//
// The NV data image consists of a number of checksum protected storage units.
// An array of NV data storage unit information is provided to allow access to
// each storage unit in turn using an array index.
//
// Caution: If the following 'm_NVLocate[]' data structure is modified, it could
// impact the correct operation of default EEPROM image generation during the
// MAKE process.
//
static const NVLOCATE_STRUCT m_NVLocate[] =
{
    /**************************************************************************
        IDENT_STRUCT must be the first storage unit in this array!
    **************************************************************************/
    {
        (U_BYTE*)&m_NVIdentDefault, (U_BYTE*)&m_NVIdent.Iden,
        {(U_INT32)&m_NVIdentImage1.Iden, (U_INT32)&m_NVIdentImage2.Iden},
        sizeof(IDENT_STRUCT) + NVDB_CHECKSUM_SIZE
    },
    {
        (U_BYTE*)&m_NVConfigDefault, (U_BYTE*)&m_NVConfig.Cnfg,
        {(U_INT32)&m_NVConfigImage1.Cnfg, (U_INT32)&m_NVConfigImage2.Cnfg},
        sizeof(CONFIG_STRUCT) + NVDB_CHECKSUM_SIZE
    },
    {
        (U_BYTE*)&m_NVOpStateDefault, (U_BYTE*)&m_NVOpState.OpSt,
        {(U_INT32)&m_NVOpStateImage1.OpSt, (U_INT32)&m_NVOpStateImage2.OpSt},
        sizeof(OPSTATE_STRUCT) + NVDB_CHECKSUM_SIZE
    },
    {
        (U_BYTE*)&m_NVMetersDefault, (U_BYTE*)&m_NVMeters.Mtrs,
        {(U_INT32)&m_NVMetersImage1.Mtrs, (U_INT32)&m_NVMetersImage2.Mtrs},
        sizeof(METERS_STRUCT) + NVDB_CHECKSUM_SIZE
    },
    {
        (U_BYTE*) &m_NVUsersDefault, (U_BYTE*) &m_NVUser.User,
        {(U_INT32) &m_NVUserImage1.User, (U_INT32) &m_NVUserImage2.User},
        sizeof(USER_DEF_STRUCT) + NVDB_CHECKSUM_SIZE
    },
};
#define NUM_NV_STORAGE_UNITS (sizeof(m_NVLocate)/sizeof(NVLOCATE_STRUCT))

// Warning! The number of storage units must not exceed 32 due to the
// number of bits available in m_bfCorruptSU.
#if (NV_INVALID_SU > 32)
    #error Too many storage units!
#endif

static NV_STORAGE_STATUS m_NVStorageUnitStatus[NUM_NV_STORAGE_UNITS];

//
// The update index identifies the storage unit that the update process is
// currently working with.
//
static U_BYTE m_nUpdateIdx = NV_SU_IDENT;

//
// We support accessing a single byte of the EEPROM from RASP, bypassing
// the NVRAM Server FOR DIAGNOSTIC USE ONLY.  The following variables
// facilitate this functionality.
//

static APPLICATION_CALLBACK m_pfAppCallback;

static BOOL m_bInitSecureParametersDone = FALSE;

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
;   SecureParametersInitialized()
;
; Description:
;   Returns the status of Secure Parameters initialization.
;
; Returns:
;   Returns TRUE if Secure Parameters is initialized; FALSE otherwise
;
; Reentrancy:
;   No
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
BOOL SecureParametersInitialized(void)
{
    return m_bInitSecureParametersDone;
}// End SecureParametersInitialized()

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   initSecureParametersFromNVRAM()
;
; Description:
;   This function will initialize module level variables and load and validate
;   all secure NV Data.
;
; Reentrancy:
;   No
;
; Assumptions:
;   The Watchdog is running.
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
static void initSecureParametersFromNVRAM(void)
{
    static BOOL bRunOnce = FALSE;

    U_INT32 nIndex;

    if(!bRunOnce)
    {
        const NV_STORAGE_STATUS defaultNVStatus = {FALSE, FALSE, FALSE, FALSE, FALSE, 0, 0};

        //
        // Set the update flag for each storage unit FALSE.
        //
        for(nIndex = 0; nIndex < NUM_NV_STORAGE_UNITS; nIndex++)
        {
            m_NVStorageUnitStatus[nIndex] = defaultNVStatus;
        }

        bRunOnce = TRUE;
    }
    else
    {
        for(nIndex = 0; nIndex < NUM_NV_STORAGE_UNITS; nIndex++)
        {
            if(!m_NVStorageUnitStatus[nIndex].bInitialized)
            {
                break;
            }
        }

        if(nIndex >= NUM_NV_STORAGE_UNITS)
        {
            m_bInitSecureParametersDone = TRUE;
            return;
        }

        switch(m_NVStorageUnitStatus[nIndex].eCurrentInitStep)
        {
          case GET_IMAGE_1:
            {
                ReadFromNVRAM(m_NVLocate[nIndex].pData, m_NVLocate[nIndex].nLen,
                             m_NVLocate[nIndex].nImage[IMAGE1], updateSecureParamsDone);

                m_NVStorageUnitStatus[nIndex].eCurrentInitStep = VERIFY_IMAGE_1;
            }
            break;
          case VERIFY_IMAGE_1:
            {
                U_INT32 nCalcCRC;

                if((CalculateCRC(m_NVLocate[nIndex].pData, m_NVLocate[nIndex].nLen, &nCalcCRC) == TRUE) && (nCalcCRC == 0))
                {
                    m_NVStorageUnitStatus[nIndex].eCurrentInitStep = ACCEPT_IMAGE;
                }
                else
                {
                    // If the storage unit from Image 1 is NOT good, i.e. bad CRC.  Then flag it for an update.
                    // If both were bad then we can fix that after verifying image 2.

                    if(checkForEmptySU(m_NVLocate[nIndex].pData, m_NVLocate[nIndex].nLen))
                    {
                        m_NVStorageUnitStatus[nIndex].bImage1Empty = TRUE;
                    }

                    m_NVStorageUnitStatus[nIndex].eCurrentInitStep = GET_IMAGE_2;
                }
            }
            break;
          case GET_IMAGE_2:
            {
                ReadFromNVRAM(m_NVLocate[nIndex].pData, m_NVLocate[nIndex].nLen,
                             m_NVLocate[nIndex].nImage[IMAGE2], updateSecureParamsDone);

                m_NVStorageUnitStatus[nIndex].eCurrentInitStep = VERIFY_IMAGE_2;
            }
            break;
          case VERIFY_IMAGE_2:
            {
                U_INT32 nCalcCRC;

                if((CalculateCRC(m_NVLocate[nIndex].pData, m_NVLocate[nIndex].nLen, &nCalcCRC) == TRUE) && (nCalcCRC == 0))
                {
                    WriteToNVRAM(m_NVLocate[nIndex].pData, m_NVLocate[nIndex].nLen,
                                m_NVLocate[nIndex].nImage[IMAGE1], updateSecureParamsDone);

                    m_NVStorageUnitStatus[nIndex].eCurrentInitStep = ACCEPT_IMAGE;
                }
                else
                {
                    if(checkForEmptySU(m_NVLocate[nIndex].pData, m_NVLocate[nIndex].nLen))
                    {
                        m_NVStorageUnitStatus[nIndex].bImage2Empty = TRUE;
                    }

                    if(m_NVStorageUnitStatus[nIndex].bImage1Empty)
                    {
                        (void)memcpy(m_NVLocate[nIndex].pData, m_NVLocate[nIndex].pDefault, m_NVLocate[nIndex].nLen);

                        // Module Variable is updated so that it is correct for setStorageUnitChecksum();
                        m_nUpdateIdx = nIndex;

                        setStorageUnitChecksum();

                        WriteToNVRAM(m_NVLocate[nIndex].pData, m_NVLocate[nIndex].nLen,
                                     m_NVLocate[nIndex].nImage[IMAGE1], updateSecureParamsDone);

                        m_NVStorageUnitStatus[nIndex].eCurrentInitStep = SU_EMPTY;
                    }
                    else
                    {
                        m_NVStorageUnitStatus[nIndex].bCorruptSU = TRUE;
                        m_NVStorageUnitStatus[nIndex].bInitialized = TRUE;
                        //ErrorState(ERR_NVRAM);
                    }
                }
            }
            break;
          case ACCEPT_IMAGE:
            {
                m_NVStorageUnitStatus[nIndex].bNVUpdate = TRUE;
                m_NVStorageUnitStatus[nIndex].bInitialized = TRUE;
            }
            break;
          case SU_EMPTY:
            {
                WriteToNVRAM(m_NVLocate[nIndex].pData, m_NVLocate[nIndex].nLen,
                             m_NVLocate[nIndex].nImage[IMAGE2], updateSecureParamsDone);

                m_NVStorageUnitStatus[nIndex].bInitialized = TRUE;
            }
            break;
          default:
            //ErrorState(ERR_SOFTWARE);
            break;
        }
    }
}// End initSecureParametersFromNVRAM()

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   checkForEmptySU()
;
; Description:
;   Provided the start address of the storage unit and the storage unit size,
;   this function determines if the storage unit is empty.
;
; Parameters:
;   pData   - The address of the data to be checked.
;   nLen    - The amount of data to be checked.
;
; Returns:
;   Returns FALSE if the storage unit is NOT empty, TRUE if it is empty.
;
; Reentrancy:
;   No.
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
static BOOL checkForEmptySU(U_BYTE *pData, U_INT16 nLen)
{
    U_INT32 nIndex = 0;

    while(nIndex < nLen)
    {
        if(pData[nIndex++] != 0xFF)
        {
            return FALSE;
        }
    }

    return TRUE;
}// End checkForEmptySU()

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   setStorageUnitChecksum()
;
; Description:
;   This function clears the update flag for the currently selected storage unit
;   and then calculates and stores a new CRC for the RAM copy of the
;   storage unit.
;
; Reentrancy:
;   No.
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
static void setStorageUnitChecksum(void)
{
    U_INT32 nChkSum;
    U_INT16 nChkLen;

    //
    // The current storage unit has been changed.  Clear the update flag for
    // that storage unit.  Do this before calculating the new CRC.
    //
    m_NVStorageUnitStatus[m_nUpdateIdx].bNVUpdate = FALSE;

    //
    // Calculate and store the new CRC.
    //
    nChkLen = m_NVLocate[m_nUpdateIdx].nLen - NVDB_CHECKSUM_SIZE;
    (void)CalculateCRC(m_NVLocate[m_nUpdateIdx].pData, nChkLen, &nChkSum);

    (void)memcpy(&m_NVLocate[m_nUpdateIdx].pData[nChkLen], (U_BYTE*)&nChkSum, NVDB_CHECKSUM_SIZE);

}// End setStorageUnitChecksum()

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   ServiceNVRAMSecureParameters()
;
; Description:
;   This routine is called periodically by the NVRAM manager to detect if a
;   direct write to NVRAM has been requested or if a secure parameter storage
;   unit has been changed.
;
;   If there is a direct write, then this routine instructs the NVRAM server to
;   perform the write, sets the callback function, and returns.
;
;   If there is no direct write request, then this routine looks for a storage
;   unit that needs to be updated.  If none is found, then this routine will
;   return.
;
;   If a storage unit needs to be updated, then this routine updates the storage
;   unit checksum, instructs the NVRAM server to write image 1 of the storage
;   unit, and sets the callback function to updateSecureParamsImage2().
;
; Reentrancy:
;   No.
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
// no longer call this from NVRAM_Server
void ServiceNVRAMSecureParameters(void)
{
    U_BYTE i;

    if(NVRAM_ClientValidation(NVRAM_CLIENT_SECURE_PARAMETERS))
    {
        if (!SecureParametersInitialized())
        {
            initSecureParametersFromNVRAM();
            return;
        }
    }
    else
    {
        return;
    }

    //
    // Start with the first storage unit and examine the update flag for
    // each storage unit in turn.  Stop as soon as a storage unit is
    // found that needs updating or when all storage units have been
    // checked.
    for (i = 0; i < NUM_NV_STORAGE_UNITS; i++)
    {
        if(m_NVStorageUnitStatus[m_nUpdateIdx].bNVUpdate)
        {
            // If a SU is dual corrupted, then a bit in m_bfCorruptSU
            // that represents the SU will be set.
            // If set then deny access to the storage unit until a
            // RASP call from intf0096 is called to correct the SU.

            if(m_NVStorageUnitStatus[m_nUpdateIdx].bCorruptSU)
            {
                m_NVStorageUnitStatus[m_nUpdateIdx].bNVUpdate = FALSE;
            }
            else
            {
                break;
            }
        }

        if (++m_nUpdateIdx >= NUM_NV_STORAGE_UNITS)
        {
            m_nUpdateIdx = 0;
        }
    }

    //
    // Return if none of the storage units needs to be updated.
    //
    if (i >= NUM_NV_STORAGE_UNITS)
    {
        return;
    }

    //
    // Calculate and store the new checksum. Clear the update flag.
    //
    setStorageUnitChecksum();

    // Prepare to have the storage unit written into NVRAM. Write the
    // entire IMAGE1. IMAGE2 will be written upon callback of completion
    // of writing IMAGE1.
    WriteToNVRAM(m_NVLocate[m_nUpdateIdx].pData, m_NVLocate[m_nUpdateIdx].nLen,
                 m_NVLocate[m_nUpdateIdx].nImage[IMAGE1], updateSecureParamsImage2);

}// End ServiceNVRAMSecureParameters()

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   updateSecureParamsImage2()
;
; Description:
;   This function prepares the NVRAM manager to write the current storage unit
;   into the other NVRAM image.
;
;   If the current storage unit has been changed while the target NVRAM image
;   was being written, then the NVRAM write process is aborted and will start
;   again the next time the Daily Values client is serviced. The target NVRAM
;   image will not be switched.
;
;   If the current storage unit did not change while the target NVRAM image
;   was being written, then the target NVRAM image is switched and a write to
;   that NVRAM image is initiated.
;
;   The NVRAM image to be written is set elsewhere so that we keep one good
;   image even if its data are old. It is only switched here if the write to the
;   target NVRAM image was completed with valid data.
;
; Reentrancy:
;   No.
;
; Assumptions:
;   This function has already been stored as the next action function.
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
static void updateSecureParamsImage2(void)
{
    //If the storage unit was changed while we were writing to the target image
    //in NVRAM, then quit and start again on the next time the Secure Parameter
    //client is serviced.
    if(m_NVStorageUnitStatus[m_nUpdateIdx].bNVUpdate)
    {
        updateSecureParamsDone();
    }
    //Otherwise update the other NVRAM image.
    else
    {
        //Now, we switch to image 2.
        WriteToNVRAM(m_NVLocate[m_nUpdateIdx].pData, m_NVLocate[m_nUpdateIdx].nLen,
                     m_NVLocate[m_nUpdateIdx].nImage[IMAGE2], updateSecureParamsDone);
    }
}// End updateSecureParamsImage2()

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   updateSecureParamsDone()
;
; Description:
;   This function confirms that the storage unit updating process has been
;   completed and releases NVRAM access.
;
;   If the current storage unit was NOT modified while the target NVRAM image
;   was being written, the target NVRAM image will be switched to the
;   other image. If the updated storage unit was the CONFIG storage unit, and a
;   new prescription was stored, then the new prescription process will also be
;   released.
;
;   If the the current storage unit was modified while the target NVRAM image
;   was being written, the target NVRAM image will NOT be switched. This
;   will ensure we keep one good image even if its data are old.
;
; Reentrancy:
;   No.
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
static void updateSecureParamsDone(void)
{
    ReleaseNVRAM ();

    if (m_pfAppCallback != NULL)
    {
        m_pfAppCallback ();
        m_pfAppCallback = NULL;
    }
}// End updateSecureParamsDone()

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   DefaultNVParams()
;
; Description:
;   Sets the contents of the RAM copy of the specified NVRAM storage unit to
;   default values and sets the update flag for that storage unit.
;
; Parameters:
;   nStorageUnit - The storage unit index value that selects which storage unit
;                  is to be set to default values.
;
; Reentrancy:
;   No
;
; Assumptions:
;   ServiceNVRAMSecureParameters() will be running periodically.
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void DefaultNVParams(U_BYTE nStorageUnit)
{
    const NVLOCATE_STRUCT* pNVLocate;
    U_INT32 nOldPSW;
    U_BYTE* pData;
    U_INT16 nLen;

    if (nStorageUnit < NV_INVALID_SU)
    {
        pNVLocate = &m_NVLocate[nStorageUnit];
        pData = pNVLocate->pData;
        nLen = pNVLocate->nLen - NVDB_CHECKSUM_SIZE;

        //
        // Copy the default values to the RAM copy.
        //
        nOldPSW = ReadPriorityStatusAndChange(BOOST_THREAD_PRIORITY);
        (void)memcpy(pData, pNVLocate->pDefault, nLen);

        //
        // Set the storage unit update flag TRUE.
        // Clear only the corresponding storage unit flag from the corrupt
        // bitfield.
        //
        m_NVStorageUnitStatus[nStorageUnit].bNVUpdate = TRUE;
        m_NVStorageUnitStatus[nStorageUnit].bCorruptSU = FALSE;

        RestorePriorityStatus(nOldPSW);
    }
    else
    {
        ;//ErrorState(ERR_SOFTWARE);
    }

}// End DefaultNVParams()

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   GetCorruptSUFlags()
;
; Description:
;   Return the value of the m_bfCorruptSU variable.
;
; Returns:
;   U_INT32 => The value of the m_bfCorruptSU variable.
;
; Reentrancy:
;   Yes
;
; Assumptions:
;   That validateStorageUnits() has been called already.
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
U_INT32 GetCorruptSUFlags(void)
{
    U_INT32 nResult;
    U_INT32 nIndex;

    for(nResult = 0, nIndex = 0; nIndex < NV_INVALID_SU;nIndex++)
    {
        nResult |= (m_NVStorageUnitStatus[nIndex].bCorruptSU << nIndex);
    }

    return nResult;

}// End GetCorruptSUFlags()

/*!
********************************************************************************
*       @details
*******************************************************************************/

void RepairCorruptSU(void)
{
    for (U_INT32 nIndex = 0; nIndex < NV_INVALID_SU; nIndex++)
    {
        if(m_NVStorageUnitStatus[nIndex].bCorruptSU )
        {
            DefaultNVParams(nIndex);
        }
    }
}// End RepairCorruptSU()

// BEGIN IDENT STRUCT ACCESSORS
/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   SetNotUsed()
;
; Description:
;   Placeholder function used as the Set routine for unused configuration bits.
;
; Parameters:
;   This would be the flag to set if we were actually using it.
;
; Reentrancy:
;   Yes
;
; Assumptions:
;   None
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void SetNotUsed(BOOL bNotUsed)
{
    ;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   GetNotUsed()
;
; Description:
;   Placeholder function used as the Get routine for unused configuration bits.
;
; Returns:
;   BOOL => FALSE for flag not used.
;
; Reentrancy:
;   Yes
;
; Assumptions:
;   None
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
BOOL GetNotUsed(void)
{
    return FALSE;
}

// IDENT ACCESSORS BEGIN

/*!
********************************************************************************
*       @details
*******************************************************************************/

PRODUCT_ID_TYPE GetProductID(void)
{
    U_INT32 nOldPSW = ReadInterruptStatusAndDisable();
    PRODUCT_ID_TYPE nID = PROD_ID_MWD;
    RestoreInterruptStatus(nOldPSW);

    return nID;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void SetProductID(PRODUCT_ID_TYPE nID)
{
    U_INT32 nOldPSW = ReadInterruptStatusAndDisable();
    m_NVIdent.Iden.nProductId = PROD_ID_MWD;
    m_NVStorageUnitStatus[NV_SU_IDENT].bNVUpdate = TRUE;
    RestoreInterruptStatus(nOldPSW);
}

// IDENT ACCESSORS END



// OPSTATE ACCESSORS BEGIN
#if NVRAM_NEW_WAY == 0
/*!
********************************************************************************
*       @details
*******************************************************************************/
void SetStartHoleFlag(BOOL bState)
{
    U_INT32 nOldPSW = ReadInterruptStatusAndDisable();

    m_NVOpState.OpSt.Flags.AsBits.bStartHole = bState;
    m_NVStorageUnitStatus[NV_SU_OPSTATE].bNVUpdate = TRUE;

    RestoreInterruptStatus(nOldPSW);
}//end

/*!
********************************************************************************
*       @details
*******************************************************************************/
BOOL GetStartHoleFlag(void)
{
    BOOL bResult;

    U_INT32 nOldPSW = ReadInterruptStatusAndDisable();
    bResult = m_NVOpState.OpSt.Flags.AsBits.bStartHole;
    RestoreInterruptStatus(nOldPSW);

    return bResult;
}//end

/*!
********************************************************************************
*       @details
*******************************************************************************/
void SetStopHoleFlag(BOOL bState)
{
    U_INT32 nOldPSW = ReadInterruptStatusAndDisable();

    m_NVOpState.OpSt.Flags.AsBits.bStopHole = bState;
    m_NVStorageUnitStatus[NV_SU_OPSTATE].bNVUpdate = TRUE;

    RestoreInterruptStatus(nOldPSW);
}//end

/*!
********************************************************************************
*       @details
*******************************************************************************/
BOOL GetStopHoleFlag(void)
{
    BOOL bResult;

    U_INT32 nOldPSW = ReadInterruptStatusAndDisable();
    bResult = m_NVOpState.OpSt.Flags.AsBits.bStopHole;
    RestoreInterruptStatus(nOldPSW);

    return bResult;
}//end

/*!
********************************************************************************
*       @details
*******************************************************************************/
void SetLoggingActiveFlag(BOOL bState)
{
    U_INT32 nOldPSW = ReadInterruptStatusAndDisable();

    m_NVOpState.OpSt.Flags.AsBits.bLoggingActive = bState;
    m_NVStorageUnitStatus[NV_SU_OPSTATE].bNVUpdate = TRUE;

    RestoreInterruptStatus(nOldPSW);
}//end

/*!
********************************************************************************
*       @details
*******************************************************************************/
BOOL IsLoggingActive(void)
{
    BOOL bResult;

    U_INT32 nOldPSW = ReadInterruptStatusAndDisable();
    bResult = m_NVOpState.OpSt.Flags.AsBits.bLoggingActive;
    RestoreInterruptStatus(nOldPSW);

    return bResult;
}//end
#endif

/*!
********************************************************************************
*       @details
*******************************************************************************/
BOOL GetModemOffState(void)
{
    BOOL bResult;

    U_INT32 nOldPSW = ReadInterruptStatusAndDisable();
    bResult = m_NVOpState.OpSt.Flags.AsBits.bStateModemIsOff;
    RestoreInterruptStatus(nOldPSW);

    return bResult;
}//end

/*!
********************************************************************************
*       @details
*******************************************************************************/
void SecPar_SetModemOffState(BOOL bState)
{
    U_INT32 nOldPSW = ReadInterruptStatusAndDisable();

    m_NVOpState.OpSt.Flags.AsBits.bStateModemIsOff = bState;
    m_NVStorageUnitStatus[NV_SU_OPSTATE].bNVUpdate = TRUE;

    RestoreInterruptStatus(nOldPSW);
}//end

#if NVRAM_NEW_WAY == 0
/*!
********************************************************************************
*       @details
*******************************************************************************/
void SetDownholeOffTime(INT16 OffTime)
{
    U_INT32 nOldPSW = ReadInterruptStatusAndDisable();
    m_NVConfig.Cnfg.nDownholeOffTime = OffTime;
    m_NVStorageUnitStatus[NV_SU_CONFIG].bNVUpdate = TRUE;
    RestoreInterruptStatus(nOldPSW);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/
INT16 GetDownholeOffTime(void)
{
    INT16 value;
    U_INT32 nOldPSW = ReadInterruptStatusAndDisable();
    value = m_NVConfig.Cnfg.nDownholeOffTime;
    RestoreInterruptStatus(nOldPSW);
    return value;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/
void SetDownholeOnTime(INT16 OnTime)
{
    U_INT32 nOldPSW = ReadInterruptStatusAndDisable();
    m_NVConfig.Cnfg.nDownholeOnTime = OnTime;
    m_NVStorageUnitStatus[NV_SU_CONFIG].bNVUpdate = TRUE;
    RestoreInterruptStatus(nOldPSW);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/
INT16 GetDownholeOnTime(void)
{
	// should be in whole seconds when returned
    INT16 value;
    U_INT32 nOldPSW = ReadInterruptStatusAndDisable();
    value = m_NVConfig.Cnfg.nDownholeOnTime;
    RestoreInterruptStatus(nOldPSW);
    return value;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/
void SetDeepSleepMode(BOOL bState)
{
    U_INT32 nOldPSW = ReadInterruptStatusAndDisable();

    m_NVConfig.Cnfg.Flags.AsBits.bDownholeDeepSleep = bState;
    m_NVStorageUnitStatus[NV_SU_CONFIG].bNVUpdate = TRUE;

    RestoreInterruptStatus(nOldPSW);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/
BOOL GetDeepSleepMode(void)
{
    BOOL bResult;

    U_INT32 nOldPSW = ReadInterruptStatusAndDisable();
    bResult = m_NVConfig.Cnfg.Flags.AsBits.bDownholeDeepSleep;
    RestoreInterruptStatus(nOldPSW);

    return bResult;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/
void SetGammaOnOff(BOOL bState)
{
    U_INT32 nOldPSW = ReadInterruptStatusAndDisable();

    m_NVConfig.Cnfg.Flags.AsBits.bGamma = bState;
    m_NVStorageUnitStatus[NV_SU_CONFIG].bNVUpdate = TRUE;

    RestoreInterruptStatus(nOldPSW);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/
BOOL GetGammaOnOff(void)
{
    BOOL bResult;

    U_INT32 nOldPSW = ReadInterruptStatusAndDisable();
    bResult = m_NVConfig.Cnfg.Flags.AsBits.bGamma;
    RestoreInterruptStatus(nOldPSW);

    return bResult;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/
void SetGammaMonitor(BOOL bState)
{
    U_INT32 nOldPSW = ReadInterruptStatusAndDisable();

    m_NVConfig.Cnfg.Flags.AsBits.bGammaMonitor = bState;
    m_NVStorageUnitStatus[NV_SU_CONFIG].bNVUpdate = TRUE;

    RestoreInterruptStatus(nOldPSW);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/
BOOL GetGammaMonitor(void)
{
    BOOL bResult;

    U_INT32 nOldPSW = ReadInterruptStatusAndDisable();
    bResult = m_NVConfig.Cnfg.Flags.AsBits.bGammaMonitor;
    RestoreInterruptStatus(nOldPSW);

    return bResult;
}
#endif
