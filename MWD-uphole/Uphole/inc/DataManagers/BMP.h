/*
;===============================================================================
;
; COPYRIGHT (c) 2013, Target Drilling Inc
; All rights are reserved.  Reproduction in whole or in part is prohibited
; without the prior written consent of the copyright holder.
;
;===============================================================================
;
; $Workfile: BMP.h $
;
; Author: WJK
; Last modified by: $Author: wKamienik $
; $Revision: 1 $
; $Date: 7/1/13 1:00p $
;
; Description:
;   Header File for BMP.c.
;==============================================================================
*/
#ifndef BMP_H
#define BMP_H

/*________________ Included files ____________________________________________*/
#include "portable.h"

#include "UI_DataStructures.h"

/*________________ Configuration _____________________________________________*/
/*________________ Constants _________________________________________________*/
/*________________ Macros ____________________________________________________*/
/*________________ Data declarations _________________________________________*/
#pragma pack (1)
typedef struct __BMP_FILE_HEADER__
{
    U_BYTE aFileID[2];
    U_INT32 nFileSize;
    U_INT32 nFileReserved;
    U_INT32 nImageDataOffset;
    U_INT32 nHeaderSize;
    U_INT32 nBitmapWidth;
    U_INT32 nBitmapHeight;
    U_INT16 nPlaneCount;
    U_INT16 nBitPerPixel;
    U_INT32 nCompressionMethod;
    U_INT32 nImageSizeBytes;
} BMP_FILE_HEADER;
#pragma pack ()

typedef enum __BMP_IMAGES__
{
    BMP_BATTERY_0,
    BMP_BATTERY_25,
    BMP_BATTERY_50,
    BMP_BATTERY_75,
    BMP_BATTERY_100,
    BMP_FILE,
    BMP_DISK,
    BMP_CONNECT_GOOD,
    BMP_CONNECT_BAD,
    MAX_BMP_IMAGES// <---- Must be the LAST entry
} BMP_IMAGES;

/*________________ Global data referenced ____________________________________*/
/*________________ Function prototypes _______________________________________*/
#ifdef __cplusplus
extern "C" {
#endif

    void BMP_Init(void);
    const UIBITMAP* GetBitmapFromImageData(BMP_IMAGES eImage);

#ifdef __cplusplus
}
#endif

#endif
