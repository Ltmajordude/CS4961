/*
** $Id: $
**
** Purpose:  cFE Application "template" (QQ) header file
**
** Author:   
**
** Notes:
**
** $Log: $
**
*/

/*************************************************************************/

/*
** Ensure that header is included only once...
*/
#ifndef _qq_app_
#define _qq_app_

/*
** Required header files...
*/
#include "cfe.h"
#include "app_msgids.h"
#include "app_perfids.h"
/*************************************************************************/

/*
** Event message ID's...
*/
#define QQ_INIT_INF_EID       1  /* start up message "informational" */

#define QQ_NOOP_INF_EID       2  /* processed command "informational" */
#define QQ_RESET_INF_EID      3
#define QQ_PROCCESS_INF_EID   4

#define QQ_MID_ERR_EID     5    /* invalid command packet "error" */
#define QQ_CC1_ERR_EID     6
#define QQ_LEN_ERR_EID     7
#define QQ_PIPE_ERR_EID    8

#define QQ_EVT_COUNT       8    /* count of event message ID's */

/*
** QQ command packet command codes...
*/
#define QQ_NOOP_CC         0    /* no-op command */
#define QQ_RESET_CC        1    /* reset counters */
#define QQ_PROCCESS_CC     2    /* Perform Routing Proccessing */


#define QQ_NUM_TABLES      2    /* Number of Tables used by Application */

#define QQ_PIPE_DEPTH      12   /* Depth of the Command Pipe for Application */
#define QQ_LIMIT_HK        2    /* Limit of HouseKeeping Requests on Pipe for Application */
#define QQ_LIMIT_CMD       4    /* Limit of Commands on pipe for Application */

/* Define filenames of default data images for tables */
#define QQ_FIRST_TBL_DEFAULT_FILE  "RAM:/FirstTblDef.dat"
#define QQ_SECOND_TBL_DEFAULT_FILE "RAM:/SecondTblDef.dat"


/* Define Application defined error code numbers for validation errors */
#define QQ_TBL_1_ELEMENT_OUT_OF_RANGE_ERR_CODE    1
#define QQ_TBL_2_ELEMENT_OUT_OF_RANGE_ERR_CODE   -1


#define QQ_TBL_ELEMENT_1_MAX   10
#define QQ_TBL_ELEMENT_3_MAX   20


/*** Definition of Table Data Structures*/

typedef struct
{ 
  uint8                 TblElement1; 
  uint16                TblElement2; 
  uint32                TblElement3; 
 
} QQ_MyFirstTable_t; 

 typedef struct
{
  int32                 TblElement1; 
  int16                 TblElement2;  
  int8                  TblElement3; 
} QQ_MySecondTable_t;


/*** Type definition Critical Data Store*/

typedef struct
{ 
  uint32                DataPtOne; 
  uint32                DataPtTwo;
  uint32                DataPtThree;
  uint32                DataPtFour;
  uint32                DataPtFive;
} QQ_CdsDataType_t;

*/*//*************************************************************************/

/*
** Type definition (generic "no arguments" command)...
*/
typedef struct
{
  uint8                 CmdHeader[CFE_SB_CMD_HDR_SIZE];

} QQ_NoArgsCmd_t;

/*************************************************************************/

/*
** Type definition (QQ housekeeping)...
*/
typedef struct
{
  uint8                 TlmHeader[CFE_SB_TLM_HDR_SIZE];

  /*
  ** Command interface counters...
  */
  uint8                 CmdCounter;
  uint8                 ErrCounter;

} QQ_HkPacket_t;

/*************************************************************************/

/*
** Type definition (QQ app global data)...
*/
typedef struct
{
  /*
  ** Command interface counters...
  */
  uint8                 CmdCounter;
  uint8                 ErrCounter;

  /*
  ** Housekeeping telemetry packet...
  */
  QQ_HkPacket_t         HkPacket;

  /*
  ** Operational data (not reported in housekeeping)...
  */
  CFE_SB_MsgPtr_t       MsgPtr;
  CFE_SB_PipeId_t       CmdPipe;

  /*
  ** Run Status variable used in the main processing loop
  */
  uint32                RunStatus;
  

  /*
  ** Operational data (not reported in housekeeping)...
  */
  QQ_CdsDataType_t      WorkingCriticalData; /* Define a copy of the critical data thatcan be */                                 
                                             /* used during Application execution */

  CFE_ES_CDSHandle_t    CDSHandle;		/* Handle to CDS Memory block */

  /*
  ** Initialization data (not reported in housekeeping)...
  */
  char                  PipeName[16];
  uint16                PipeDepth;

  uint8                 LimitHK;
  uint8                 LimitCmd;

  CFE_EVS_BinFilter_t   EventFilters[QQ_EVT_COUNT]; 
  CFE_TBL_Handle_t      TblHandles[QQ_NUM_TABLES];

} QQ_AppData_t;

/*************************************************************************/

/*
** Local function prototypes...
**
** Note: Except for the entry point (QQ_AppMain), these
**       functions are not called from any other source module.
*/
void QQ_AppMain(void);
void QQ_AppInit(void);
void QQ_AppPipe(CFE_SB_MsgPtr_t msg);

void QQ_HousekeepingCmd(CFE_SB_MsgPtr_t msg);

void QQ_NoopCmd(CFE_SB_MsgPtr_t msg);
void QQ_ResetCmd(CFE_SB_MsgPtr_t msg);
void QQ_RoutineProcessingCmd(CFE_SB_MsgPtr_t msg); 

boolean QQ_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 ExpectedLength); 

int32 QQ_FirstTblValidationFunc(void *TblData); 
int32 QQ_SecondTblValidationFunc(void *TblData);


/*************************************************************************/

#endif /* _qq_app_ */

/************************/
/*  End of File Comment */
/************************/