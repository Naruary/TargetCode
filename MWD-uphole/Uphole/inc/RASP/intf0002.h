/*!
********************************************************************************
*       @brief      Provides prototypes for functions contained in intf0002.c 
*                   supporting the "RT Clock" interface.
*                   Documented by Josh Masters Dec. 2014
*       @file       Uphole/inc/RASP/intf0002.h
*       @author     Bill Kamienik
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef INTF0002_H
#define INTF0002_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"
#include "rasp.h"

//============================================================================//
//      GLOBAL DATA                                                           //
//============================================================================//

extern const CMD_VALIDATION g_aCmdVal0002[];

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    ///@brief  
    ///@param  
    ///@return 
    void Interface0002Handler(U_BYTE* pHeader, U_BYTE* pData, U_INT16 nDataLen);

#ifdef __cplusplus
}
#endif
#endif