/*!
********************************************************************************
*       @brief      Provides prototypes for functions contained in intf0007.c
*                   supporting the "Update Diagnostic Downhole" interface.
*       @file       Uphole/inc/RASP/intf0007.h
*       @author     Walter Rodrigues
*       @date       January 2016
*       @copyright  COPYRIGHT (c) 2016 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef INTF0007_H
#define INTF0007_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"
#include "rasp.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#define UPDATE_DIAG_DATA_SET       0


//============================================================================//
//      GLOBAL DATA                                                           //
//============================================================================//

extern const CMD_VALIDATION g_diagCmdVal0007[];
extern TIME_LR tUpdateDownHole;
extern TIME_LR tUpdateDownHoleSuccess;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    ///@brief
    ///@param
    ///@return
    void DiagnosticHandler(U_BYTE* pHeader, U_BYTE* pData, U_INT16 nDataLen);

    ///@brief
    ///@param
    ///@return
    void UpdateDiagnosticDataInDownholeRequest(void);

    ///@brief
    ///@param
    ///@return
    TIME_LR GetUpdateDownHoleTimer(void);

    ///@brief
    ///@param
    ///@return
    TIME_LR GetUpdateDownHoleSuccessTimer(void);

#ifdef __cplusplus
}
#endif
#endif