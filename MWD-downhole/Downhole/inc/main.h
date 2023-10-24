/*******************************************************************************
*       @brief      Provides prototypes for public functions in main.c.
*       @file       Downhole/inc/main.h
*       @date       July 2013
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef MAIN_H
#define MAIN_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#define FALSE       0
#define TRUE        (!(FALSE)) //lint !e506

#define OFF FALSE
#define ON  TRUE

#ifndef NULL   // Assume that if NULL is already defined that it is defined correctly
#define NULL   (void *)0
#endif

typedef char                CHAR;
typedef unsigned char       BOOL;       //  8 bits, unsigned
typedef signed char         BYTE;       //  8 bits, signed
typedef signed short int    INT16;      // 16 bits, signed
typedef signed long int     INT32;      // 32 bits, signed
typedef long long           INT64;      // 64 bits, signed
typedef unsigned char       U_BYTE;     //  8 bits, unsigned
typedef unsigned short int  U_INT16;    // 16 bits, unsigned
typedef unsigned long int   U_INT32;    // 32 bits, unsigned
typedef unsigned long long  U_INT64;    // 64 bits, unsigned
typedef float               REAL32;     // 32 bits, floating point
typedef double              REAL64;     // 64 bits, floating point
typedef U_INT32 			TIME_RT;

extern TIME_RT tTimePoweredUp;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

extern volatile BOOL ProcessedCommsMessageFlag;

#ifdef __cplusplus
}
#endif
#endif
