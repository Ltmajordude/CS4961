/*
** $Id: $
**
** Subsystem: cFE Application Template (QQ) Application
**
** Author: 
**
** Notes:
**
** $Log: $
**
*/

/*
** Required header files...
*/
#include "qq_app.h"
#include <string.h>

/*
** QQ global data...
*/
QQ_AppData_t QQ_AppData;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* QQ_AppMain() -- Application entry point and main process loop   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void QQ_AppMain(void)
{
    int32 Status; 

    /*
    ** Register application...
    */
    CFE_ES_RegisterApp();


    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(TST_QQ_APPMAIN_PERF_ID);


    /*
    ** Perform application specific initialization
    ** If the Initialization fails, set the RunStatus to 
    ** CFE_ES_APP_ERROR and the App will not enter the RunLoop
    */
    Status = QQ_AppInit();

    if (Status != CFE_SUCCESS)
    {
        QQ_AppData.RunStatus= CFE_ES_APP_ERROR;
    }

    /*
    ** Application Main Loop. Call CFE_ES_RunLoop to check for changes
    ** in the Applications status. If there is a request to kill this 
    ** App, it will be passed in through the RunLoop call.
    */
    while (CFE_ES_RunLoop (&QQ_AppData.RunStatus == TRUE)
    {
        /*
        ** Performance Log Exit Stamp
        */
        CFE_ES_PerfLogExit(TST_QQ_APPMAIN_PERF_ID);
        
        /*
        ** Wait for the next Software Bus message...
        */
        Status = CFE_SB_RcvMsg(&QQ_AppData.MsgPtr,
                                 QQ_AppData.CmdPipe,
                                  CFE_SB_PEND_FOREVER);
        /*
        ** Performance Log Entry Stamp
        */
        CFE_ES_PerfLogEntry(TST_QQ_APPMAIN_PERF_ID);

        /* Check the return status from the Software Bus*/

        if (Status == CFE_SUCCESS)
        {
            /*
            ** Process Software Bus message. If there are fatal errors
            ** in command processing the command can alter the global
            ** RunStatus variable to exit the main event loop.
            */
            QQ_AppPipe(QQ_AppData.MsgPtr);

            /*
            ** Update the Critical Data Store. Because this data is only updated 
            ** in one command, this could be moved to the command processing function.
            ** in command processing the command can alter the global
            ** RunStatus variable to exit the main event loop.
            */
            CFE_ES_CopyToCDS(QQ_AppData.CDSHandle, &QQ_AppData.WorkingCriticalData);
        }
        else
        {
           /* This is an example of exiting on an error.
           ** Note that a SB read error is not always going to
           ** result in an app quitting.
           */

            CFE_EVS_SendEvent(&QQ_PIPE_ERR_EID,CFE_EVS_ERROR, 
                              "QQ: SB Pipe Read Error, QQ App Will Exit");

            QQ_AppData.RunStatus= CFE_ES_APP_ERROR;
        }
    }
    /*
    ** Performance Log Exit Stamp
    */
    CFE_ES_PerfLogExit(TST_QQ_APPMAIN_PERF_ID);

    /*
    ** Exit the Application
    */
 CFE_ES_ExitApp(QQ_AppData.RunStatus);

} /* End of QQ_AppMain() */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* QQ_AppInit() -- QQ initialization                         */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void QQ_AppInit(void)
{
    int32     Status;
    int32     ResetType;
    uint32    ResetSubType;

    ResetType = CFE_ES_GetResetType(&ResetSubType);

    /*
    ** For a PowerOn Reset, initialize the Critical variables
    ** If it is a Processor Reset, these variables will be restored
    ** from the Critical Data Store later in the function
    */
    if (ResetType == CFE_ES_POWER_ON)
    {
        QQ_AppData.RunStatus= CFE_ES_APP_ERROR;
        QQ_AppData.WorkingCriticalData.DataPtOne    = 1;
        QQ_AppData.WorkingCriticalData.DataPtTwo    = 2;
        QQ_AppData.WorkingCriticalData.DataPtThree  = 3;
        QQ_AppData.WorkingCriticalData.DataPtFour   = 4;
        QQ_AppData.WorkingCriticalData.DataPtFive   = 5;
    }

QQ_AppData.RunStatus= CFE_ES_APP_RUN;
    /*
    ** Initialize app command execution counters...
    */
    QQ_AppData.CmdCounter = 0;
    QQ_AppData.ErrCounter = 0;

    /*
    ** Initialize app configuration data...
    */
    strcpy(QQ_AppData.PipeName, "QQ_CMD_PIPE");

    QQ_AppData.PipeDepth = QQ_PIPE_DEPTH;

    QQ_AppData.LimitHK   = QQ_LIMIT_HK;
    QQ_AppData.LimitCmd  = QQ_LIMIT_CMD;

    /*
    ** Initialize event filter table...
    */
    QQ_AppData.EventFilters[0].EventID = QQ_PROCCES_INF_EID;
    QQ_AppData.EventFilters[0].Mask    = CFE_EVS_EVERY_FOURTH_TIME;
    QQ_AppData.EventFilters[1].EventID = QQ_RESET_INF_EID;
    QQ_AppData.EventFilters[1].Mask    = CFE_EVS_NO_FILTER;
    QQ_AppData.EventFilters[2].EventID = QQ_CC1_INF_EID;
    QQ_AppData.EventFilters[2].Mask    = CFE_EVS_EVERY_OTHER_TWO;
    QQ_AppData.EventFilters[3].EventID = QQ_LEN_ERR_EID;
    QQ_AppData.EventFilters[3].Mask    = CFE_EVS_FIRST_8_STOP;


    /*
    ** Register event filter table...
    */
    Status = CFE_EVS_Register(QQ_AppData.EventFilters,
                     4,
                     CFE_EVS_BINARY_FILTER);

if ( Status != CFE_SUCCESS )
    {
       CFE_ES_WriteToSysLog("QQ App: Error Registering Events, RC = 0x%08X\n", Status);
       return ( Status );
    }
              
    /*
    ** Initialize housekeeping packet (clear user data area).
    */
    CFE_SB_InitMsg(&QQ_AppData.HkPacket, 
                   QQ_HK_TLM_MID, 
                   sizeof(QQ_HkPacket_t), TRUE);
   
    /*
    ** Create Software Bus message pipe.
    */
    Status = CFE_SB_CreatePipe(&QQ_AppData.CmdPipe, 
                                QQ_AppData.PipeDepth, 
                                QQ_AppData.PipeName);
    if ( Status != CFE_SUCCESS )
    {
       /*
       ** Could use an event at this point
       */
       CFE_ES_WriteToSysLog("QQ App: Error Creating SB Pipe, RC = 0x%08X\n", Status);
       return ( Status );
    }

    /*
    ** Subscribe to Housekeeping request commands
    */
    Status = CFE_SB_Subscribe(QQ_SEND_HK_MID,QQ_AppData.CmdPipe);
    if ( Status != CFE_SUCCESS )
    {
       CFE_ES_WriteToSysLog("QQ App: Error Subscribing \
                            to HK Request, RC = 0x%08X\n", 
                            Status);
       return ( Status );
    }

    /*
    ** Subscribe to QQ ground command packets
    */
    Status = CFE_SB_Subscribe(QQ_CMD_MID,QQ_AppData.CmdPipe);
    if ( Status != CFE_SUCCESS )
    {
       CFE_ES_WriteToSysLog("QQ App: Error Subscribing to QQ \
                             Command, RC = 0x%08X\n", Status);
       return ( Status );
    }

    /*
    ** Register tables with cFE and load default data
    */
    Status = CFE_TBL_Register(&QQ_AppData.TblHandles[0], 
                               "MyFirstTable",
                              sizeof(QQ_MyFirstTable_t), 
                               CFE_TBL_OPT_DEFAULT,
                              QQ_FirstTblValidationFunc);
    if ( Status != CFE_SUCCESS )
    {
       CFE_ES_WriteToSysLog("QQ App: Error Registering \
                             Table 1, RC = 0x%08X\n", Status);
       return ( Status );
    }
    else
    {
       Status = CFE_TBL_Load(QQ_AppData.TblHandles[0], 
       CFE_TBL_SRC_FILE, QQ_FIRST_TBL_DEFAULT_FILE);
    }
    
    Status = CFE_TBL_Register(&QQ_AppData.TblHandles[1], "MySecondTable",
                              sizeof(QQ_MySecondTable_t), CFE_TBL_OPT_DEFAULT,
                              QQ_SecondTblValidationFunc);
    if ( Status != CFE_SUCCESS )
    {
       CFE_ES_WriteToSysLog("QQ App: Error Registering Table 2, RC = 0x%08X\n", Status);
       return ( Status );
    }
    else
    {
      Status = CFE_TBL_Load(QQ_AppData.TblHandles[1], CFE_TBL_SRC_FILE, QQ_SECOND_TBL_DEFAULT_FILE);
    }
                 

    /*
    ** Create and manage the Critical Data Store 
    */
    Status = CFE_ES_RegisterCDS(&QQ_AppData.CDSHandle, sizeof(QQ_CdsDataType_t), QQ_CDS_NAME);
    if(Status == CFE_SUCCESS)
    {
       /* 
       ** Setup Initial contents of Critical Data Store 
       */
       CFE_ES_CopyToCDS(QQ_AppData.CDSHandle, &QQ_AppData.WorkingCriticalData);
       
    }
    else if(Status == CFE_ES_CDS_ALREADY_EXISTS)
    {
       /* 
       ** Critical Data Store already existed, we need to get a copy 
       ** of its current contents to see if we can use it
       */
       Status = CFE_ES_RestoreFromCDS(&QQ_AppData.WorkingCriticalData, QQ_AppData.CDSHandle);
       if(Status == CFE_SUCCESS)
       {
          /*
          ** Perform any logical verifications, if necessary, to validate data 
          */
          CFE_ES_WriteToSysLog("QQ App CDS data preserved\n");
       }
       else
       {
          /* 
          ** Restore Failied, Perform baseline initialization 
          */
          QQ_AppData.WorkingCriticalData.DataPtOne   = 1;
          QQ_AppData.WorkingCriticalData.DataPtTwo   = 2;
          QQ_AppData.WorkingCriticalData.DataPtThree = 3;
          QQ_AppData.WorkingCriticalData.DataPtFour  = 4;
          QQ_AppData.WorkingCriticalData.DataPtFive  = 5;
          CFE_ES_WriteToSysLog("Failed to Restore CDS. Re-Initialized CDS Data.\n");
       }
    }
    else 
    {
       /* 
       ** Error creating my critical data store 
       */
       CFE_ES_WriteToSysLog("QQ: Failed to create CDS (Err=0x%08x)", Status);
       return(Status);
    }

    /*
    ** Application startup event message.
    */
    CFE_EVS_SendEvent(QQ_INIT_INF_EID,
                     CFE_EVS_INFORMATION, 
                     "QQ: Application Initialized");
                         
    return(CFE_SUCCESS);

} /* End of QQ_AppInit() */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* QQ_AppPipe() -- Process command pipe message                   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void QQ_AppPipe(CFE_SB_MsgPtr_t msg)
{
    CFE_SB_MsgId_t MessageID;
    uint16 CommandCode;

    MessageID = CFE_SB_GetMsgId(msg);
    switch (MessageID)
    {
        /*
        ** Housekeeping telemetry request...
        */
        case QQ_SEND_HK_MID:
            QQ_HousekeepingCmd(msg);
            break;

        /*
        ** QQ ground commands...
        */
        case QQ_CMD_MID:

            CommandCode = CFE_SB_GetCmdCode(msg);
            switch (CommandCode)
            {
                case QQ_NOOP_CC:
                    QQ_NoopCmd(msg);
                    break;

                case QQ_RESET_CC:
                    QQ_ResetCmd(msg);
                    break; 

                case QQ_PROCESS_CC: 
                    QQ_RoutineProcessingCmd(msg); 
                    break;

                default:
                    CFE_EVS_SendEvent(QQ_CC1_ERR_EID, CFE_EVS_ERROR,
                     "Invalid ground command code: ID = 0x%X, CC = %d",
                                      MessageID, CommandCode);
                    break;
            }
            break;

        default:

            CFE_EVS_SendEvent(QQ_MID_ERR_EID, CFE_EVS_ERROR,
                             "Invalid command pipe message ID: 0x%X",
                              MessageID);
            break;
    }

    return;

} /* End of QQ_AppPipe() */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* QQ_HousekeepingCmd() -- On-board command (HK request)           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void QQ_HousekeepingCmd(CFE_SB_MsgPtr_t msg)
{
    uint16 ExpectedLength = sizeof(QQ_NoArgsCmd_t); 
    uint16 i;

    /*
    ** Verify command packet length...
    */
    if (QQ_VerifyCmdLength(msg, ExpectedLength))
    {
       /*
        ** Get command execution counters...
        */
        QQ_AppData.HkPacket.CmdCounter = QQ_AppData.CmdCounter;
        QQ_AppData.HkPacket.ErrCounter = QQ_AppData.ErrCounter;

        /*
        ** Send housekeeping telemetry packet...
        */
        CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &QQ_AppData.HkPacket);
        CFE_SB_SendMsg((CFE_SB_Msg_t *) &QQ_AppData.HkPacket);

        /** Manage any pending table loads, validations, etc.   */
        for (i=0; i<QQ_NUM_TABLES; i++)
        {
            CFE_TBL_Manage(QQ_AppData.TblHandles[i]); 
        }        


        /*
        ** This command does not affect the command execution counter...
        */
    }

    return;

} /* End of QQ_HousekeepingCmd() */



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* QQ_NoopCmd() -- QQ ground command (NO-OP)                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void QQ_NoopCmd(CFE_SB_MsgPtr_t msg)
{
    uint16 ExpectedLength = sizeof(QQ_NoArgsCmd_t);

    /*
    ** Verify command packet length...
    */
    if (QQ_VerifyCmdLength(msg, ExpectedLength))
    {
        QQ_AppData.CmdCounter++;

        CFE_EVS_SendEvent(QQ_NOOP_INF_EID, CFE_EVS_INFORMATION,
                         "No-op command");
    }

    return;

} /* End of QQ_NoopCmd() */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* QQ_ResetCmd() -- QQ ground command (reset counters)        */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void QQ_ResetCmd(CFE_SB_MsgPtr_t msg)
{
    uint16 ExpectedLength = sizeof(QQ_NoArgsCmd_t); 

    /*
    ** Verify command packet length...
    */
    if (QQ_VerifyCmdLength(msg, ExpectedLength))
    {
        QQ_AppData.CmdCounter = 0;
        QQ_AppData.ErrCounter = 0;

        CFE_EVS_SendEvent(QQ_RESET_INF_EID, CFE_EVS_INFORMATION,
                         "Reset Counters command");
    }

    return;

} /* End of QQ_ResetCmd() */



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* QQ_RoutineProcessingCmd() -- QQ ground command (Process command)*/
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void QQ_RoutineProcessingCmd(CFE_SB_MsgPtr_t msg)
{
    uint16 ExpectedLength = sizeof(QQ_NoArgsCmd_t);
    QQ_MyFirstTable_t    *MyFirstTblPtr;
    QQ_MySecondTable_t   *MySecondTblPtr;

    /*
    ** Verify command packet length
    */
    if (QQ_VerifyCmdLength(msg, ExpectedLength))
    {
        /* Obtain access to table data addresses */
        CFE_TBL_GetAddress((void *)&MyFirstTblPtr, 
                            QQ_AppData.TblHandles[0]);
        CFE_TBL_GetAddress((void *)&MySecondTblPtr, 
                            QQ_AppData.TblHandles[1]);
        
        /* Perform routine processing accessing table data via pointers */
        /*                            .                                 */
        /*                            .                                 */
        /*                            .                                 */
        
        /* Once completed with using tables, release addresses          */
        CFE_TBL_ReleaseAddress(QQ_AppData.TblHandles[0]);
        CFE_TBL_ReleaseAddress(QQ_AppData.TblHandles[1]);
        
        /*
        ** Update Critical variables. These variables will be saved
        ** in the Critical Data Store and preserved on a processor reset.
        */
        QQ_AppData.WorkingCriticalData.DataPtOne++;
        QQ_AppData.WorkingCriticalData.DataPtTwo++;
        QQ_AppData.WorkingCriticalData.DataPtThree++;
        QQ_AppData.WorkingCriticalData.DataPtFour++;
        QQ_AppData.WorkingCriticalData.DataPtFive++;
        
        CFE_EVS_SendEvent(QQ_PROCCESS_INF_EID,CFE_EVS_INFORMATION,
                         "QQ: Routine Processing Command");

    }

    return;


} /* End of QQ_RoutineProcessingCmd() */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* QQ_VerifyCmdLength() -- Verify command packet length            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

boolean QQ_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 ExpectedLength)
{
    boolean result = TRUE;
    uint16 ActualLength = CFE_SB_GetTotalMsgLength(msg);

    /*
    ** Verify the command packet length...
    */
    if (ExpectedLength != ActualLength)
    {
        CFE_SB_MsgId_t MessageID = CFE_SB_GetMsgId(msg);
        uint16 CommandCode = CFE_SB_GetCmdCode(msg);

        CFE_EVS_SendEvent(QQ_LEN_ERR_EID, CFE_EVS_ERROR,
           "Invalid cmd pkt: ID = 0x%X,  CC = %d, Len = %d",
                          MessageID, CommandCode, ActualLength);
        result = FALSE;
        QQ_AppData.ErrCounter++;
    }

    return(result);

} /* End of QQ_VerifyCmdLength() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* QQ_FirstTblValidationFunc() -- Verify contents of First Table   */
/*                                buffer contents                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 QQ_FirstTblValidationFunc(void *TblData)
{
    int32              ReturnCode = CFE_SUCCESS;
    QQ_MyFirstTable_t *TblDataPtr = (QQ_MyFirstTable_t *)TblData;
    
    if (TblDataPtr->TblElement1 > QQ_TBL_ELEMENT_1_MAX)
    {
        /* First element is out of range, return an appropriate error code */
        ReturnCode = QQ_TBL_1_ELEMENT_OUT_OF_RANGE_ERR_CODE;
    }
    
    return ReturnCode;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* QQ_SecondTblValidationFunc() -- Verify contents of Second Table */
/*                                 buffer contents                 */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 QQ_SecondTblValidationFunc(void *TblData)
{
    int32               ReturnCode = CFE_SUCCESS;
    QQ_MySecondTable_t *TblDataPtr = (QQ_MySecondTable_t *)TblData;
    
    if (TblDataPtr->TblElement3 > QQ_TBL_ELEMENT_3_MAX)
    {
        /* Third element is out of range, return an appropriate error code */
        ReturnCode = QQ_TBL_2_ELEMENT_OUT_OF_RANGE_ERR_CODE;
    }
    
    return ReturnCode;
}


/************************/
/*  End of File Comment */
/************************/