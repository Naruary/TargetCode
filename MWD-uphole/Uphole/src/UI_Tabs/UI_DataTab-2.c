/*!
********************************************************************************
*       @brief      This file contains the implementation for the Data
*                   tab on the Uphole LCD screen.
*       @file       Uphole/src/UI_Tabs/UI_DataTab.c
*       @author     Josh Masters
*       @date       July 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <stdio.h>
#include <stdlib.h>
#include "intf0003.h"
#include "lcd.h"
#include "Manager_DataLink.h"
#include "RASP.h"
#include "TextStrings.h"
#include "UI_Alphabet.h"
#include "UI_ScreenUtilities.h"
#include "UI_LCDScreenInversion.h"
#include "UI_Frame.h"
#include "UI_api.h"
#include "UI_BooleanField.h"
#include "UI_FixedField.h"
#include "UI_RecordDataPanel.h"
#include "UploadingPanel.h"
#include "UI_SurveyEditPanel.h"
#include "SecureParameters.h"
#include "WaitingForSurveyPanel.h"
#include "UI_DeleteLastSurveyDecisionPanel.h"
#include "UI_BranchPointSetDecisionPanel.h"
#include "DeleteLastSurveySuccessPanel.h"
#include "DeleteLastSurveyNotSuccessPanel.h"
#include "BranchPointSetSuccessPanel.h"
#include "Compass_Panel.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#define NUM_ROWS    16
#define ROW_HEIGHT  15

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

///@brief  
///@param  
///@return 
static PANEL* Data_CurrentState(void);

///@brief  
///@param  
///@return 
static MENU_ITEM* GetDataMenuItem(TAB_ENTRY* tab, U_BYTE index);

///@brief  
///@param  
///@return 
static U_BYTE GetDataMenuSize(TAB_ENTRY* tab);

///@brief  
///@param  
///@return 
static void DataTabPaint(TAB_ENTRY* tab);

///@brief  
///@param  
///@return 
static void DataTabShow(TAB_ENTRY* tab);

///@brief  
///@param  
///@return 
static void DataTabRefresh(TAB_ENTRY* tab);

///@brief  
///@param  
///@return 
static void DataKeyPressed(TAB_ENTRY* tab, BUTTON_VALUE key);

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

///@brief
const TAB_ENTRY DataTab = {&TabFrame2, TXT_DATA, ShowTab, GetDataMenuItem, GetDataMenuSize, DataTabPaint, DataTabShow, DataTabRefresh, DataKeyPressed};

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*!
********************************************************************************
*       @details
*******************************************************************************/

static PANEL* Data_CurrentState(void)
{
  if(getSurveyEditPanelActive())
  {
    return &SurveyEditPanel;
  }
  else
  {
    switch(GetLoggingState())
    {
    case WAITING_FOR_UPLOAD:
      return &Uploading;
      break;
    case DELETE_LAST_SURVEY_SUCCESS:
      return &DeleteLastSurveySuccessPanel;
      break;
    case DELETE_LAST_SURVEY_NOT_SUCCESS:
      return &DeleteLastSurveyNotSuccessPanel;;
      break;
    case BRANCH_POINT_SET_SUCCESS:
      return &BranchPointSetSuccessPanel;
      break;
    case DELETE_LAST_SURVEY:
      if (getDeleteLastSurveyDecisionPanelActive())
      {
        return &DeleteLastSurvey_DecisionPanel;
      }
      break;
    case BRANCH_POINT_SET:
      if (getBranchPointSetDecisionPanelActive())
      {
        return &BranchPointSet_DecisionPanel;
      }
      break;
     case COMPASS_LOGGING:
      if (getCompassDecisionPanelActive())
      {
        return &Compass_DecisionPanel;
      }
      break;
    }
  }
  return &RecordDataPanel;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static MENU_ITEM* GetDataMenuItem(TAB_ENTRY* tab, U_BYTE index)
{
    if (index < tab->MenuSize(tab))
    {
        return Data_CurrentState()->MenuItem(index);
    }
    return NULL;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static U_BYTE GetDataMenuSize(TAB_ENTRY* tab)
{
    return Data_CurrentState()->MenuCount;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void DataTabPaint(TAB_ENTRY* tab)
{
    Data_CurrentState()->Paint(tab);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void DataTabShow(TAB_ENTRY* tab)
{
    if (Data_CurrentState()->Show)
    {
        Data_CurrentState()->Show(tab);
    }
    PaintNow(&HomeFrame);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void DataTabRefresh(TAB_ENTRY* tab)
{
    if (Data_CurrentState()->TimerElapsed)
    {
        Data_CurrentState()->TimerElapsed(tab);
    }
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void DataKeyPressed(TAB_ENTRY* tab, BUTTON_VALUE key)
{
    if (Data_CurrentState()->KeyPressed)
    {
        Data_CurrentState()->KeyPressed(tab, key);
    }
}