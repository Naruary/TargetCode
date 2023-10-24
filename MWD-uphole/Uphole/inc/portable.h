/*******************************************************************************
*       @brief      This file contains data types which, when used by all
*                   systems, will eliminate all confusion that C introduces with
*                   respect to data size.  Thrown in for good measure are some
*                   macros which perform the same function but are implemented
*                   differently for specific processor types. Finally, there are
*                   some common constants (YES, NO, true, false, etc.)
*       @file       Uphole/inc/portable.h
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef PORTABLE_H
#define PORTABLE_H

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#ifndef NULL   // Assume that if NULL is already defined that it is defined correctly
 #define NULL   (void *)0
#endif

#define BITS_IN_BYTE 8

typedef     char                CHAR;
typedef     unsigned char       BOOL;       //  8 bits, unsigned
typedef     signed char         BYTE;       //  8 bits, signed
typedef     short int           INT16;      // 16 bits, signed (explicitly)
typedef     signed long int     INT32;      // 32 bits, signed
typedef     long long           INT64;      // 64 bits, signed
typedef     unsigned char       U_BYTE;     //  8 bits, unsigned
typedef     unsigned short int  U_INT16;    // 16 bits, unsigned
typedef     unsigned long int   U_INT32;    // 32 bits, unsigned
typedef     unsigned long long  U_INT64;    // 64 bits, unsigned
typedef     float               REAL32;     // 32 bits, floating point
typedef     double              REAL64;     // 64 bits, floating point

//============================================================================//
//      MACROS                                                                //
//============================================================================//

// integer angles are saved with an implied decimal, so
// 3600 is equal to 360.0 degrees
typedef     INT16    ANGLE_TIMES_TEN;
#define THREE_SIXTY_DEGREES	360
#define THREE_SIXTY_TIMES_TEN	(THREE_SIXTY_DEGREES * 10)
#define ONE_EIGHTY_DEGREES	(THREE_SIXTY_DEGREES / 2)
#define ONE_EIGHTY_DEGREES_x10	(THREE_SIXTY_DEGREES * 5)
#define PI 3.14159265

// Find number of elements in array
#define M_NumElements(x)  (sizeof(x) / sizeof(x[0]))
#define COUNTOF(x)        M_NumElements(x)


//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif // PORTABLE_H
