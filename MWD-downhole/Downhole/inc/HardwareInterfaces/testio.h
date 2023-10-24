/*******************************************************************************
*       @brief      Contains header information for the test io.
*       @file       Downhole/inc/HardwareInterfaces/testio.h
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef TESTIO_H
#define TESTIO_H

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

    void TESTIOInitPins(void);
    void TESTIOToggle(void);
    void TESTIOSetState(BOOL bState);
	BOOL WAKEIORead(void);

#ifdef __cplusplus
}
#endif

#endif // TESTIO_H
