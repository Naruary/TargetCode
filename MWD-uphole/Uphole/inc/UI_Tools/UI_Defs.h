/*******************************************************************************
*       @brief      Includes are the proper include files to use the UI
*       @file       Uphole/inc/UI_Tools/UI_Defs.h
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef UI_DEFS_H_
#define UI_DEFS_H_

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <string.h>
#include "portable.h"
#include "lcd.h"
#include "timer.h"
#include "PeriodicEvents.h"
#include "UI_DataStructures.h"
#include "textStrings.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

//menu sizes
//#define TEXT_SIZE_OFFSET                4    // offset for alphabet text
//#define DECIMAL_OFFSET                  3    // offset for a decimal number
//#define NEGATIVE_NUM_OFFSET             3    // offset for a negative number
//#define NEGATIVE_SIGN_COL_OFFSET        8    // offset for a negative sign
//#define MAX_TEXT_LEN                    60   // max string length
//
//#define MIN_ROW_COL                     0    // minimum row and column

/*
//increment values
#define INCR_BY_ONE 1
#define INCR_BY_FIVE 5

//Rectangle
#define RECT_FILLED 0x1
*/
// Line and character drawing options
#define F_BOLD          0x01    // Make the character bold
#define F_UNDERLINE     0x02    // Underline the character
#define F_FIT_ON_ONE_LINE   0x04    // Fit only that which can be displayed on a single line
#define F_TWODIGITS     0x08    // Make the number two digits (1 becomes 01)
#define F_ALTFONT       0x10    // Use the 11x11 Alternate font
#define F_NORENDER      0x20    // Calculate only, no draw
#define F_ERASE         0x40    // Erase the icon space
#define F_BIGNUMBERS    0x80    // Giant Sized Numeric Characters 0-9

/*
#define LTA_DEFAULT     0x00    // LongToAscii options - default
#define LTA_COMMA       0x01    // LongToAscii options - use commas
//
// Box Drawing Options - Used in DrawRect
//
#define EXCLUSIVE 1  // Exclude boundries
#define INCLUSIVE 0  // Include boundries

#define SD_CARD_ICON_SIZE 9
*/

//============================================================================//
//      MACROS                                                                //
//============================================================================//

//#define NUMBER_OF_TABS                  ((TAB5-TAB1)+1)
#define NUMBER_OF_MENU_POSN             ((LABEL10-LABEL1)+1)

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	void M_Turn_LCD_On_And_Reset_Timer(void);

#ifdef __cplusplus
}
#endif
#endif
