/*!
********************************************************************************
*       @brief      Header file for PCHoleInterface.c. 
*                   Documented by Josh Masters Dec. 2014
*       @file       Uphole/inc/RASP/PCHoleInterface.h
*       @author     Chris Walker
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef PCHOLEINTERFACE_H
#define PCHOLEINTERFACE_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"
#include "rasp.h"
#include "RecordManager.h"

//============================================================================//
//      GLOBAL DATA                                                           //
//============================================================================//

extern const CMD_VALIDATION g_pcCmdValidation[];

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    ///@brief  
    ///@param  
    ///@return
    void PCHoleInterface(U_BYTE* pHeader, U_BYTE* pData, U_INT16 nDataLen);

#ifdef __cplusplus
}
#endif
#endif