/*******************************************************************************
*       @brief      Header File for crc.c.
*       @file       Uphole/inc/crc.h
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef CRC_H
#define CRC_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	BOOL CalculateCRC(U_BYTE *pData, U_INT16 nLength, U_INT32 *nResultCRC);
	void ResetCRC(U_INT32 *pCRC);
	void CRC_CalculateOnByte(U_INT32 *pCRC, U_BYTE nData);

#ifdef __cplusplus
}
#endif
#endif
