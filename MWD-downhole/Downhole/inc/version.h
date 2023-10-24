/*******************************************************************************
*       @brief      Defines the string used to indicate the current version of
*                   the software.
*       @file       Downhole/inc/DataManagers/version.h
*       @date       July 2013
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef VERSION_H
#define VERSION_H

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
//#define VERSION "3.3"
#define VERSION "3.4" //ZD 7July2023 Chnaged for new board design Battery Voltage Detector

// So that others can allocate a buffer to hold the SW Version the length is.
#define MAX_VERSION_LEN     7   //Including terminating NULL

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	const char* GetSWVersion(void);

#ifdef __cplusplus
}
#endif

#endif
