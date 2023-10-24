/*!
********************************************************************************
*       @brief      Contains prototypes for the functions in NVRAM_Driver.h
*                   Documented by Josh Masters Dec. 2014
*       @file       Uphole/inc/NVRAM/NVRAM_Driver.h
*       @author     Bill Kamienik
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef NVRAM_DRIVER_H
#define NVRAM_DRIVER_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

// The maximum allowable read/write bytes per transfer
#define MAX_READ_TRANSFER_SIZE    512
#define MAX_WRITE_TRANSFER_SIZE   512

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

typedef void (*EEPROM_CALLBACK)(U_INT16* pNumTransferred, U_BYTE* nDummy);
typedef void (*EEPROM_ERROR_CALLBACK)(void);
typedef U_INT32 EEPROM_LENGTH;  // This defines the address length of the EEPROM

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    ///@brief  
    ///@param  
    ///@return 
    void WriteNVData(EEPROM_LENGTH* pNVAddr, U_BYTE** pDataPtr, U_INT16* pLength, EEPROM_CALLBACK pCallBack);
    
    ///@brief  
    ///@param  
    ///@return 
    void ReadNVData(EEPROM_LENGTH* pNVAddr, U_BYTE** pDataPtr, U_INT16* pLength, EEPROM_CALLBACK pCallBack);

#ifdef __cplusplus
}
#endif
#endif