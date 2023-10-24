/*!
********************************************************************************
*       @brief      Provides prototypes for functions contained in intf0004.c 
*                   supporting the "Start / Stop Hole" interface.
*                   Documented by Josh Masters Dec. 2014
*       @file       Uphole/inc/RASP/intf0004.h
*       @author     Bill Kamienik
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef INTF0004_H
#define INTF0004_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"
#include "rasp.h"
#include "RecordManager.h"

//============================================================================//
//      GLOBAL DATA                                                           //
//============================================================================//

extern const CMD_VALIDATION g_aCmdVal0004[];

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    ///@brief  
    ///@param  
    ///@return
    void Interface0004Handler(U_BYTE* pHeader, U_BYTE* pData, U_INT16 nDataLen);

#ifdef __cplusplus
}
#endif
#endif