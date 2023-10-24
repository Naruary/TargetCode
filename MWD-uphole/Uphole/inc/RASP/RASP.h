/*!
********************************************************************************
*       @brief      This file provides the external interface to RASP.
*                   Documented by Josh Masters Dec. 2014
*       @file       Uphole/inc/RASP/RASP.h
*       @author     Bill Kamienik
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef RASP_H
#define RASP_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"
#include "timer.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

// The present implementation of RASP only uses 4 bytes in the header
#define RASP_HEADER_LEN 3
#define MAX_RASP_LENGTH 255

#define MSG_BUF_LEN (MAX_RASP_LENGTH - RASP_HEADER_LEN)

// Return status codes
#define ACCEPTED        0x0
#define REJECTED        0x1
#define INVALID_CMD     0x2
#define NOT_AVAILABLE   0x3
#define NA_IN_THIS_MODE 0x4
#define NOT_APPLICABLE  0x5
#define MEDIA_REMOVED   0x6

// definition of the header bytes positions
#define CMD_ID_POS 2            // the command ID position

#define INTF_0000 (U_INT16)0x0000
#define INTF_0000_CMD_00 (U_BYTE)0x00
#define INTF_0000_CMD_01 (U_BYTE)0x01

#define INTF_0001 (U_INT16)0x0001
#define INTF_0001_CMD_00 (U_BYTE)0x00
#define INTF_0001_CMD_01 (U_BYTE)0x01

#define INTF_0002 (U_INT16)0x0002
#define INTF_0002_CMD_00 (U_BYTE)0x00
#define INTF_0002_CMD_01 (U_BYTE)0x01

#define INTF_0003 (U_INT16)0x0003
#define INTF_0003_CMD_00 (U_BYTE)0x00
#define INTF_0003_CMD_01 (U_BYTE)0x01
#define INTF_0003_CMD_02 (U_BYTE)0x02
#define INTF_0003_CMD_03 (U_BYTE)0x03
#define INTF_0003_CMD_04 (U_BYTE)0x04
#define INTF_0003_CMD_05 (U_BYTE)0x05


typedef enum
{
    NONE,
    SYSTEM,
    REAL_TIME_CLOCK,
    SENSOR_MANAGER,
    HOLE_MANAGER,
    CALIBRATION_MANAGER,
    PC_HOLE_MANAGER,
    DIAGNOSTIC_HANDLER,
    DOWNHOLE_STATUS,
} RASP_INTERFACE;

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

//
// The RASP_FUNCTION type identifies functions that the RASP manager can call
// in place of ProcessRASP() while waiting for a defered reply to be completed.
//
typedef void (*RASP_FUNCTION)(void);

typedef void (*RASP_CALLBACK_TX)(void);

typedef struct
{
    U_BYTE nExpectedDataSize;           // the expected size of the incoming data portion of the message
                                        // if the data size is not equal to this value, the message
                                        // will be rejected by RASP.  Set this value to 0xFF to
                                        // disable the checking for variable length data situations.
    BOOL (*pCmdValidationFunction)(U_BYTE* pHeader, U_BYTE* pData, U_INT16 nDataLen);
                                        // pointer to a command validation function
                                        // set this to NULL to disable the command validation
                                        // function call
}CMD_VALIDATION;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    ///@brief  
    ///@param  
    ///@return
    void InitRASP(void);
    
    ///@brief  
    ///@param  
    ///@return
    void RASPManager(void);
    
    ///@brief  
    ///@param  
    ///@return
    void ProcessRASP(void);
    
    ///@brief  
    ///@param  
    ///@return
    void RASPRequest(RASP_INTERFACE nInterface, const U_BYTE nCommand, const U_BYTE *pData, U_INT16 nDataLen);
    
    ///@brief  
    ///@param  
    ///@return
    void RASPReply(const U_BYTE *pHeader, U_BYTE nRespCode, const U_BYTE *pData, U_INT16 nDataLen);
    
    ///@brief  
    ///@param  
    ///@return
    void RASPReplyNoError(const U_BYTE *pHeader, const U_BYTE *pData, U_INT16 nDataLen);

    ///@brief  
    ///@param  
    ///@return
    void ServiceRxRASP(U_BYTE nRxChar);
    
    ///@brief  
    ///@param  
    ///@return
    void ArmResetReceiveSMAfterTx(void);

#ifdef __cplusplus
}
#endif
#endif