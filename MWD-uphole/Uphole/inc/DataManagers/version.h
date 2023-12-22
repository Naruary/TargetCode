/*******************************************************************************
*       @brief      Defines the string used to indicate the current version of
*                   the software.
*       @file       Uphole/inc/DataManagers/version.h
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef VERSION_H
#define VERSION_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

////////////////////////////////////////////////////////////////////////////////
// The displayed software version string contains up to 6 characters.
//
// "DX.YYp" is the form of the displayed version string when the source is
// unlabeled
// - "developer" version - non-reproducible.
//
// "AX.YYp" is the form of the displayed version string for source that is
// labeled under version control for routine baselines
// - "alpha" version - reproducible.
//
// "BX.YYp" is the form of the displayed version string for source that is
// labeled under version control for development and product assurance testing
// - "beta" version - reproducible.
//
// "VX.YYp" is the form of the displayed version string for source that is
// labeled under version control for production.  There are no minor increments
// in the formally released production version.  The minor increment is used
// during the V&V of a production release candidate so that once the testing is
// completed it can be easily demonstrated that the only difference between the
// tested and the final release versions is the internal version string, as
// viewed by a text difference report of the S-record files.
//
// An extra single whitespace before the closing quote is intentional in
// absence of the minor increment character.
////////////////////////////////////////////////////////////////////////////////
//#define VERSION "3.2" // 3 since major change in Graphic LCD like, Graphs, Analog meters
//#define VERSION "3.7" // after 3 months CLE changed most code, 2019-02-21
//#define VERSION "3.8" // First change in 4 months. Changed PCDataTransfer timer, 06/21/2021
//#define VERSION "4.1" // Chris is back. Added signal strength, no sleep in DH, 9/9/2021 
//#define VERSION "4.1.1" // ZD small update - implementation of keypad code changes - Making it so when user presses a button it won't show up twice 1/6/2023 
//#define VERSION "4.1.2" //ZD 6/5/2023 update to fix the 0 error that keeps reoccurring when deleting the last survey. Updated in the DeleteLastSurveyDecision.c file -> static void Yes_DeleteLastSurvey_Decision  The selectedSurveyRecord is being set to 0 if records are deleted. Therefore, RECORD_getSelectSurveyRecordNumber() will return 0 if records are deleted since selectedSurveyRecord.nRecordNumber will be 0 in this case The function RECORD_StoreSelectSurvey(U_INT32 index) stores the record of the index in selectedSurveyRecord. It is currently not being used before removing a record. Using this function just before removing a record should solve the problem.
//#define VERSION "4.1.3" // ZD 6/7/2023 this is for NON-XP Box 3 having issues with the time out popping up - lowered the awake time settings for that to pop up to 2 seconds since there is no way any user could get another show in that time. - this box also uses 15 debounce instead of just 10 like the rest in 4.1.2
//#define VERSION "4.1.4" //ZD 6/12/2023 Just to keep uniformity keeping the 15 debounce for all boxes may be the best option the lowering of awake time setting was also reverted back to 5 seconds. As well as instead going with an edit to the formula in the DownholeBatteryAndLife.c file is easier and seems to fix the countdown from every 5 seconds to every second by adding the increment operator ++. The Check Survey data also is now incremental just so they show up faster when the downhole is moved around.
//#define VERSION "4.2" //ZD 7/12/2023 New PCB Boards were made so we skipped to 4.2 from 4.1.4. In this update we also - Reverted back the incremental changing of the check survey as it seemed to have some issues where it wasn't seemingly accurate. The MEDEM_POWER_PIN was also set high from now found in the Modem Driver.c file Line 102. 4.2 will not work with the old uphole PCBs originally made from the Eagle files as they use a differing circuit.
//#define VERSION "4.2.1" //ZD 7/17/2023 This fixes the reset circuit on the new circuit boards also update to Box tab UI for the display turn off time 
//#define VERSION "4.2.4" //ZD 7/28/2023 New Downhole boards required new update for their Battery Voltage Test circuit splitting it into 2 Battery Voltages this update just reflects the display and getting of the new function implenetation of GetBattery2Voltage
//#define VERSION "4.2.5" //ZD 9/7/2023 Updated some changes for Doug making the toolface set with removed decimal. This update also allows numbers over 1000 shown in full on the Main UI page
//#define VERSION "4.2.7" //ZD 9/8/2023 Updated for the new circuit boards so no hardware modifications must be made. The Yitran must be grounded to turn on.
//#define VERSION "5.0.0" //ZD 10/16/2023 Tetsting
//#define VERSION "5.0.3" //ZD 12/14/2023 Fixes with the uploading of Data to the Uphole Box & the allowance of Both the Upload and the Connection to Modem
#define VERSION "5.0.4" //ZD 12/22/2023 Fixes the Records from having branch issues.
// 3.7, 3-21-2019, This version has new NV access and completely new
//  Target Protocol for serial, and also serial terminal access to PC logging.

// So that others can allocate a buffer to hold the SW Version the length is.
#define MAX_VERSION_LEN     7   //Including terminating NULL

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	const char* GetSWVersion(void);
	void SetDownholeSWVersion(char *string, U_BYTE length);
	char* GetDownholeSWVersion(void);
	void SetDownholeSWDate(char *string, U_BYTE length);
	char* GetDownholeSWDate(void);

#ifdef __cplusplus
}
#endif

#endif
