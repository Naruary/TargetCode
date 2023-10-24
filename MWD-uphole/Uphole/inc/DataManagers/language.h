/*
;===============================================================================
;
; COPYRIGHT (c) 2014, Target Drilling Inc
; All rights are reserved.  Reproduction in whole or in part is prohibited
; without the prior written consent of the copyright holder.
;
;===============================================================================
;
; $Workfile: Language.h $
;
; Author: CAW
; Last modified by: $Author: $
; $Revision: $
; $Date: $
;
; Description:
;   This is the prototype file for languages.
;
;===============================================================================
*/
#ifndef LANGUAGE_H
#define LANGUAGE_H

#include "portable.h"
//
// The available User Interface displayed language selections must be a
// contiguous integer range beginning with 0 and ending with INVALID_LANGUAGE.
// These enum values may be used for integer math and loop and array indexing.
// The integer value 0 must select English.
//
typedef enum
{
    USE_ENGLISH = 0,
    USE_SPANISH,
    USE_GERMAN,
    USE_FRENCH,
    USE_ITALIAN,
    USE_CHINESE,
    USE_JAPANESE,
    USE_ICONS,
    INVALID_LANGUAGE
} LANGUAGE_SETTING;

#define FIRST_LANGUAGE    (U_INT32)USE_ENGLISH
#define MAX_INTL_LANGUAGE ((U_INT32)INVALID_LANGUAGE - 1)

/*________________ Function prototypes _______________________________________*/
#ifdef __cplusplus
extern "C" {
#endif

//BOOL SetCurrentLanguage(LANGUAGE_SETTING eSetting);
//LANGUAGE_SETTING CurrentLanguage(void);

#ifdef __cplusplus
}
#endif

#endif
