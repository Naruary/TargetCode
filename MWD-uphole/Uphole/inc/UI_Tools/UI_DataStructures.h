/*******************************************************************************
*       @brief      Header File for the User Interface Subsystem that defines
*                   all the data structures and data types for the UI System.
*       @file       Uphole/inc/UI_Tools/UI_DataStructures.h
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef UI_DATA_STRUCTURES_H
#define UI_DATA_STRUCTURES_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"
#include "periodicEvents.h"

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

//typedef unsigned int BMDATA;
typedef U_INT32 BMDATA;

typedef struct
{
	U_BYTE      nXSize;         /* length of each scanline in pixels */
	U_BYTE      nYSize;         /* number of scanlines in the image */
	BMDATA      nFlags;         /* is first bit on or off */
	U_INT16     nNumData;       /* how many element in the data array */
	U_BYTE      nKern;          /* How many pixels wide is the visible portion of the character */
	const void *pData;          /* data */
}UIBITMAP;

typedef struct
{
    BMDATA      nFlags;          /* is first bit on or off */
    U_BYTE      nKern;
    const void *pData;           /* data */
}UIALPHABITMAP;

// A Point is a single row,col location on the screen
typedef struct __POINT__
{
    U_INT16 nRow;
    U_INT16 nCol;
}POINT;

//
// The rectangle is a basic unit of the UI system. It is
// expressed in absolute pixel coordinates.
typedef struct  __RECT__
{
    POINT ptTopLeft;
    POINT ptBottomRight;
}RECT;


//
// A Region is a space defined by four points
//
typedef struct __REGION__
{
    POINT ptTopLeft;
    POINT ptBottomLeft;
    POINT ptTopRight;
    POINT ptBottomRight;
}REGION;

//
// The HDC is our own Handle to a Device Context
//
typedef struct __HDC__
{
    // The rectangle that defines the boundries of the context
    RECT rctArea;
}HDC;


enum ANIMATION_STEPS
{
    ANIMATION1,
    ANIMATION2,
    ANIMATION3,
    ANIMATION4,
    MAX_ANIMATION_STEPS
};
#endif
