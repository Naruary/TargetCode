/*******************************************************************************
*       @brief      Header File for crc.c.
*       @file       Downhole/inc/crc.h
*       @date       July 2013
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef CRC_H
#define CRC_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "main.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    ///@brief
    ///@param
    ///@return
    BOOL CalculateCRC(U_BYTE *pData, U_INT16 nLength, U_INT32 *nResultCRC);

    ///@brief
    ///@param
    ///@return
    void ResetCRC(U_INT32 *pCRC);

    ///@brief
    ///@param
    ///@return
    void CRC_CalculateOnByte(U_INT32 *pCRC, U_BYTE nData);

#ifdef __cplusplus
}
#endif
#endif