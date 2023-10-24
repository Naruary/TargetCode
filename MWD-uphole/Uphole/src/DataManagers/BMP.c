/*
;===============================================================================
;
; COPYRIGHT (c) 2013, Target Drilling Inc
; All rights are reserved.  Reproduction in whole or in part is prohibited
; without the prior written consent of the copyright holder.
;
;===============================================================================
;
; $Workfile: BMP.c $
;
; Author: WJK
; Last modified by: $Author: wKamienik $
; $Revision: 1 $
; $Date: 7/1/13 1:00p $
;
; Description:
;   This module provides BMP file access
;
;==============================================================================
*/
/*________________ Included Files ____________________________________________*/
#include <intrinsics.h>
#include <stm32f4xx.h>
#include <string.h>
#include <stdbool.h>
#include "portable.h"
#include "BMP.h"
#include "UI_DataStructures.h"

/*________________ Configuration Confirmation ________________________________*/
/*________________ Global Data Referenced ____________________________________*/
extern BMP_FILE_HEADER g_nDataBattery0Bmp;
extern BMP_FILE_HEADER g_nDataBattery25Bmp;
extern BMP_FILE_HEADER g_nDataBattery50Bmp;
extern BMP_FILE_HEADER g_nDataBattery75Bmp;
extern BMP_FILE_HEADER g_nDataBattery100Bmp;

extern BMP_FILE_HEADER g_nDataFileBmp;
extern BMP_FILE_HEADER g_nDataDiskBmp;
extern BMP_FILE_HEADER g_nDataConnectGoodBmp;
extern BMP_FILE_HEADER g_nDataConnectBadBmp;

//extern BMP_FILE_HEADER g_nDataRadioBmp;

/*________________ Constants _________________________________________________*/
#define BITS_IN_UINT32  32
#define BYTES_IN_UINT32  4

#define MONOCHROME_FILE_ID_0        'B'
#define MONOCHROME_FILE_ID_1        'M'
#define MONOCHROME_PLANE_COUNT      1
#define MONOCHROME_BITS_PER_PIXEL   1
#define NO_BMP_DATA_COMPRESSION     0
#define MONOCHROME_BMP_HEADER_SIZE  40

/*________________ Macros ____________________________________________________*/
/*________________ Data Declarations _________________________________________*/
/*________________ Function Prototypes _______________________________________*/
static BOOL validateImageConstants(const BMP_FILE_HEADER* pFile);
static BOOL getImageHeaderInfo(const BMP_FILE_HEADER* pFile);

/*________________ Data Definitions __________________________________________*/
static UIBITMAP m_resultBitmap;
static U_BYTE   m_nResultBitmapData[512];
static U_BYTE*  m_pBitmapData;

static const BMP_FILE_HEADER* g_nBMPData[MAX_BMP_IMAGES] = {
    &g_nDataBattery0Bmp,
    &g_nDataBattery25Bmp,
    &g_nDataBattery50Bmp,
    &g_nDataBattery75Bmp,
    &g_nDataBattery100Bmp,
    &g_nDataFileBmp,
    &g_nDataDiskBmp,
    &g_nDataConnectGoodBmp,
    &g_nDataConnectBadBmp,
};

/*________________ Code ______________________________________________________*/
void BMP_Init(void)
{

}

const UIBITMAP* GetBitmapFromImageData(BMP_IMAGES eImage)
{
    const BMP_FILE_HEADER *pBMPData = g_nBMPData[eImage];

    memset((void *)&m_resultBitmap, 0, sizeof(m_resultBitmap));
    memset((void *)&m_nResultBitmapData, 0, sizeof(m_nResultBitmapData));

    if(validateImageConstants(pBMPData) && getImageHeaderInfo(pBMPData))
    {
        U_INT32 nYposition = m_resultBitmap.nYSize;
        U_INT32 nXposition = 0;
        U_INT32 nScanLineBytes = 0;

        nScanLineBytes = m_resultBitmap.nXSize / BITS_IN_UINT32;
        if((m_resultBitmap.nXSize % BITS_IN_UINT32) != 0)
        {
            nScanLineBytes++;
        }

        nScanLineBytes *= BYTES_IN_UINT32;

        while(nYposition != 0)
        {
            memcpy((void *)&m_nResultBitmapData[nXposition], (const void *)&m_pBitmapData[(--nYposition) * nScanLineBytes], nScanLineBytes);
            nXposition += nScanLineBytes;
        }

        // Invert BMP data so black pixel is black.
        while(nXposition != 0)
        {
            --nXposition;
            m_nResultBitmapData[nXposition] = ~m_nResultBitmapData[nXposition];
        }

        m_resultBitmap.pData = &m_nResultBitmapData[0];
    }

    return &m_resultBitmap;
}

static BOOL validateImageConstants(const BMP_FILE_HEADER* pFile)
{
    BOOL bValid = true;

    // These are the constants that we are expecting in the file header.
    // If these elements are not as expected then something is wrong with
    // the included BMP file and rendering will fail.
    bValid &= pFile->aFileID[0] == MONOCHROME_FILE_ID_0;
    bValid &= pFile->aFileID[1] == MONOCHROME_FILE_ID_1;
    bValid &= pFile->nPlaneCount == MONOCHROME_PLANE_COUNT;
    bValid &= pFile->nBitPerPixel == MONOCHROME_BITS_PER_PIXEL;
    bValid &= pFile->nCompressionMethod == NO_BMP_DATA_COMPRESSION;
    bValid &= pFile->nHeaderSize == MONOCHROME_BMP_HEADER_SIZE;

    return bValid;
}

static BOOL getImageHeaderInfo(const BMP_FILE_HEADER* pFile)
{
    BOOL bValid = true;

    m_resultBitmap.nXSize = pFile->nBitmapWidth;
    m_resultBitmap.nYSize = pFile->nBitmapHeight;
    m_resultBitmap.nNumData = pFile->nImageSizeBytes;

    m_pBitmapData = (U_BYTE *)((U_INT32)pFile + pFile->nImageDataOffset);

    bValid &= m_resultBitmap.nXSize != 0;
    bValid &= m_resultBitmap.nYSize != 0;
    bValid &= m_resultBitmap.nNumData != 0;

    return bValid;
}
