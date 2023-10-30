/*******************************************************************************
*       @brief      Implementation file for the PC data transfer
*       @file       Uphole/inc/SerialProtocol/PCDataTransfer.h
*       @date       March 2019
*       @copyright  COPYRIGHT (c) 2019 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef PCDATA_TRANSFER_H
#define PCDATA_TRANSFER_H
#include "csvparser.h"
#define PCDT_COMM_BUFF_SIZE	255

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    void PCPORT_ReceiveData(U_BYTE nData);
    void PCPORT_StateMachine(void);
    void PCPORT_ReceiveDataUSB(void);
    void PCPORT_UPLOAD_StateMachine(void);
#ifdef __cplusplus
}
#endif

#endif // PCDATA_TRANSFER_H
