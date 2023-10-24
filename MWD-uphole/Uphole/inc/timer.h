/*******************************************************************************
*       @brief      Header File for system Timer functions.
*       @file       Uphole/inc/timer.h
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef TIMER_H
#define TIMER_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

// the main (low res) timer period is exactly 1 ms.
#define START_LOW_RES_TIMER ((TIME_LR) 0)
#define MILLI_SECOND ((TIME_LR)1)
#define TWO_MILLI_SECONDS ((TIME_LR)2)
#define FOUR_MILLI_SECONDS ((TIME_LR)4)
#define FIVE_MILLI_SECONDS ((TIME_LR)5)
#define TEN_MILLI_SECONDS ((TIME_LR)10)
#define TWENTY_FIVE_MILLI_SECONDS ((TIME_LR)25)
#define FIFTY_MILLI_SECONDS ((TIME_LR)50)
#define HUNDRED_MILLI_SECONDS ((TIME_LR)100)
#define TWO_HUNDRED_MILLI_SECONDS ((TIME_LR)200)
#define THREE_HUNDRED_MILLI_SECONDS ((TIME_LR)300)
#define FOUR_HUNDRED_MILLI_SECONDS ((TIME_LR)400)

#define HALF_SECOND             ((TIME_LR)500)
#define THREE_QUARTER_SECOND    ((TIME_LR)750)

#define ONE_SECOND   ((TIME_LR)1000)
#define TWO_SECOND   ((TIME_LR)ONE_SECOND*(TIME_LR)2)
#define THREE_SECOND ((TIME_LR)ONE_SECOND*(TIME_LR)3)
#define FOUR_SECOND  ((TIME_LR)ONE_SECOND*(TIME_LR)4)
#define FIVE_SECOND  ((TIME_LR)ONE_SECOND*(TIME_LR)5)
#define SIX_SECOND   ((TIME_LR)ONE_SECOND*(TIME_LR)6)

#define TEN_SECOND    ((TIME_LR)ONE_SECOND*(TIME_LR)10)
#define TWENTY_SECOND ((TIME_LR)ONE_SECOND*(TIME_LR)20)
#define THIRTY_SECOND ((TIME_LR)ONE_SECOND*(TIME_LR)30)

#define ONE_MINUTE   ((TIME_LR)ONE_SECOND*(TIME_LR)60)
#define TWO_MINUTE   ((TIME_LR)2 * ONE_MINUTE)
#define FIVE_MINUTE  ((TIME_LR)5 * ONE_MINUTE)
#define TENTH_HOUR   ((TIME_LR)6 * ONE_MINUTE)
#define TEN_MINUTE   ((TIME_LR)10 * ONE_MINUTE)

#define CYCLE_TIME_MSEC 10  //Number of ms per cycle
#define TRIGGER_TIME_NOW 0

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

typedef U_INT32 TIME_LR;       /* used for Low Resolution Timer */

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    void Delay5us(void);
    void DelayHalfSecond(void);

#ifdef __cplusplus
}
#endif
#endif
