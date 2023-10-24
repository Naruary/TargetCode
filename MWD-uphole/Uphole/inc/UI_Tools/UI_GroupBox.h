/*******************************************************************************
*       @brief      This header file contains callable functions and public
*                   data for the Group Box pseudo class.
*       @file       Uphole/inc/UI_Tools/UI_GroupBox.h
*       @date       August 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef UI_GROUP_BOX_H
#define UI_GROUP_BOX_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"
#include "UI_ScreenUtilities.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#define MAX_FIELDS 10

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

typedef struct _DisplayField
{
    TXT_VALUES Label;
    char* (*GetFormattedValue)(struct _DisplayField* field);
    union
    {
        INT16 (*int16)(void);
        U_INT16 (*uint16)(void);
        INT32 (*int32)(void);
        U_INT32 (*uint32)(void);
        REAL32 (*real32)(void);
    };
} DisplayField;

typedef struct _GroupBox
{
    TXT_VALUES Title;
    RECT Area;
    DisplayField Fields[MAX_FIELDS];
} GroupBox;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	char* DisplayInt16Value(DisplayField* field);
	char* DisplayInt16ValueXXX(DisplayField* field);   // whs 3Dec2021 added below 3 lines to put xxx in Check Survey panel 
	char* DisplayIntU16ValueXXX(DisplayField* field);         
	char* DisplayUint16Value(DisplayField* field);
	char* DisplaySurveyInt16Value(DisplayField* field);
	char* DisplayUint32Value(DisplayField* field);
	char* DisplayReal32Value(DisplayField* field);
	void GroupBoxPaint(GroupBox* box);
#ifdef __cplusplus
}
#endif
#endif
