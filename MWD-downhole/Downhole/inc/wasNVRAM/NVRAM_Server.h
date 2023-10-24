/*!
********************************************************************************
*       @brief      This is the prototype file for NVRAM_Server.c.
*                   Documented by Josh Masters Nov. 2014
*       @file       Downhole/inc/NVRAM/NVRAM_Server.h
*       @author     Bill Kamienik
*       @date       July 2013
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef NVRAM_SERVER_H
#define NVRAM_SERVER_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

typedef void (*CLIENT_CALLBACK)(void);
typedef void (*APPLICATION_CALLBACK)(void);

typedef enum
{
    NO_READWRITE,
    READ_NV,
    WRITE_NV,
}TRANSFER_MODE;

typedef enum {
//    NVRAM_CLIENT_ERROR_LOG,
    NVRAM_CLIENT_SECURE_PARAMETERS,
    NVRAM_CLIENT_SURVEY_VALUES,
}NVRAM_CLIENT_ID;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    ///@brief  
    ///@param  
    ///@return
    void InitNVRAM_Server(void);
    
    ///@brief  
    ///@param  
    ///@return
    void NVRAM_Server(void);

    ///@brief  
    ///@param  
    ///@return
    void ReadFromNVRAM(U_BYTE* pData, U_INT16 nLength, U_INT32 nOffset, CLIENT_CALLBACK pfClient);
    
    ///@brief  
    ///@param  
    ///@return
    void WriteToNVRAM(U_BYTE* pData, U_INT16 nLength, U_INT32 nOffset, CLIENT_CALLBACK pfClient);

    ///@brief  
    ///@param  
    ///@return
    void ReleaseNVRAM(void);

    ///@brief  
    ///@param  
    ///@return
    BOOL NVRAM_ClientValidation(NVRAM_CLIENT_ID eClient);

#ifdef __cplusplus
}
#endif
#endif