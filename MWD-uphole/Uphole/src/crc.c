/*******************************************************************************
*       @brief      This module provides CRC functionallity.
*       @file       Uphole/src/crc.c
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <stm32f4xx.h>
#include <stdbool.h>
#include "portable.h"
#include "crc.h"

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
BOOL CalculateCRC(U_BYTE *pData, U_INT16 nLength, U_INT32 *nResultCRC)
{
	U_INT32 *pData32;
	if(((nLength % 4) != 0) || (((U_INT32)pData % 4) != 0))
	{
		return false;
	}
	pData32 = (U_INT32 *)pData;
	nLength /= 4;
	CRC_ResetDR();
	*nResultCRC = CRC_CalcBlockCRC((uint32_t *)pData32, nLength);
	return true;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void ResetCRC(U_INT32 *pCRC)
{
	CRC_ResetDR();
	*pCRC = CRC_GetCRC();
}

/*******************************************************************************
*       @details
*******************************************************************************/
void CRC_CalculateOnByte(U_INT32 *pCRC, U_BYTE nData)
{
	*pCRC = CRC_CalcCRC((U_INT32)nData);
}
