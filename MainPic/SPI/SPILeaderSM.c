/****************************************************************************
 Module
   SPILeaderSM.c

 Revision
   1.0.1

 Description
   This is a template file for implementing flat state machines under the
   Gen2 Events and Services Framework.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 01/15/12 11:12 jec      revisions for Gen2 framework
 11/07/11 11:26 jec      made the queue static
 10/30/11 17:59 jec      fixed references to CurrentEvent in RunTemplateSM()
 10/23/11 18:20 jec      began conversion from SMTemplate.c (02/20/07 rev)
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "terminal.h"
#include "SPILeaderSM.h"
#include "../HALs/PIC32PortHAL.h"
#include "../HALs/PIC32_SPI_HAL.h"
#include "ES_Events.h" 
#include <xc.h>
#include <sys/attribs.h>

/*----------------------------- Module Defines ----------------------------*/

/*----------------------------- Module Types ------------------------------*/
// typedefs for the states
// State definitions for use with the query function

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/

bool InitializeSPI(void);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file
static SPILeaderSMState_t CurrentState;

// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitSPILeaderSM

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, sets up the initial transition and does any
     other required initialization for this state machine
 Notes

 Author
     J. Edward Carryer, 10/23/11, 18:55
****************************************************************************/
bool InitSPILeaderSM(uint8_t Priority)
{
    puts("Initializing SPILeaderSM...\r");
    ES_Event_t ThisEvent;

    MyPriority = Priority;
    // put us into the Initial PseudoState
    CurrentState = SPILeaderInitState;
    
    puts("...Done Initializing SPILeaderSM\r\n");
 
    // post the initial transition event
    ThisEvent.EventType = ES_INIT;
    if (ES_PostToService(MyPriority, ThisEvent) == true)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/****************************************************************************
 Function
     PostSPILeaderSM

 Parameters
     ES_Event_t ThisEvent , the event to post to the queue

 Returns
     boolean False if the Enqueue operation failed, True otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
 Andrew Sack
****************************************************************************/
bool PostSPILeaderSM(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunSPILeaderSM

 Parameters
   ES_Event_t : the event to process

 Returns
   ES_Event_t, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
  
 Notes
   uses nested switch/case to implement the machine.
 Author
 Andrew Sack
****************************************************************************/
ES_Event_t RunSPILeaderSM(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
    ES_Event_t PostEvent;
    
    switch (CurrentState)
    {
        case SPILeaderInitState:        // If current state is initial  State
        {
          if (ThisEvent.EventType == ES_INIT)    // only respond to ES_Init
          {
            if (true == InitializeSPI()) //Initialize SPI for display
            {
                //If successful switch state
                CurrentState = SPILeaderSendState;
                puts("...Done Initializing SPI1\r\n");
            }           
            else // SPI failed to initialize, throw error
            {
                puts("ERROR: Failed to Initialize SPI1\r\n");
                ReturnEvent.EventType = ES_ERROR;
            }  
          }
        }break;
        case SPILeaderSendState:
        {
            if (ThisEvent.EventType == SEND_SPI_COMMAND){
                printf("Sending %x\r\n",ThisEvent.EventParam);
                SPIOperate_SPI1_Send16(ThisEvent.EventParam);
                CurrentState = SPILeaderReceiveState;
            }
            if (ThisEvent.EventType == SPI_RESET){
                CurrentState = SPILeaderSendState;
            }
            
        }break;
        case SPILeaderReceiveState:
        {
            if (ThisEvent.EventType == SPI_RESPONSE_RECEIVED){
                if (ThisEvent.EventParam == 0x1111){
                    ES_Event_t NewEvent;
                    NewEvent.EventType   = SPI_TASK_COMPLETE;
                    printf("Success, posting to service\r\n");
                }
                else if (ThisEvent.EventParam == 0xAAAA){
                    ES_Event_t NewEvent;
                    NewEvent.EventType   = SPI_TASK_FAILED;
                    printf("Failure, posting to service\r\n");
                }
                CurrentState = SPILeaderSendState;
            }
            if (ThisEvent.EventType == SPI_RESET){
                CurrentState = SPILeaderSendState;
            }
        }break;
        default:
          ;
    }                                   // end switch on Current State
    
    return ReturnEvent;
}


/****************************************************************************
 Function
     QuerySPILeaderSM

 Parameters
     None

 Returns
     SPILeaderSMState_t The current state of the SPILeaderSM state machine

 Description
     returns the current state of the Drive Train state machine
 Notes

 Author
 Andrew Sack
****************************************************************************/
SPILeaderSMState_t QuerySPILeaderSM(void)
{
    return CurrentState;
}

bool InitializeSPI(void)
{
    bool ReturnVal = true;
    SPIOperate_HasSS1_Risen();
    //Configure SPI connection
    ReturnVal &= SPISetup_BasicConfig(SPI_SPI1);
    ReturnVal &= SPISetup_SetLeader(SPI_SPI1, SPI_SMP_MID);
    ReturnVal &= SPISetup_SetBitTime(SPI_SPI1, 1000); //1,000 ns = 1 MHZ
    ReturnVal &= SPISetup_MapSSOutput(SPI_SPI1, SPI_RPA0); // make A0 SS
    ReturnVal &= SPISetup_MapSDOutput(SPI_SPI1, SPI_RPA1); // make A1 SDO
    
    //ReturnVal &= SPISetup_MapSDInput(SPI_SPI1, SPI_RPB5); // make B5 SDI 
    //NEED TO IMPLEMENT
    PortSetup_ConfigureDigitalInputs(_Port_B,_Pin_5);
    SDI1R = 0b0001;
    
    ReturnVal &= SPISetup_SetClockIdleState(SPI_SPI1, SPI_CLK_HI); // clock is idle high
    ReturnVal &= SPISetup_SetActiveEdge(SPI_SPI1, SPI_SECOND_EDGE); // read on 2nd edge 
    ReturnVal &= SPISetup_SetXferWidth(SPI_SPI1, SPI_16BIT); //16 bit messages
    ReturnVal &= SPISetEnhancedBuffer(SPI_SPI1, false); //disable ENHBUF
    //After everything set up, enable spi
    ReturnVal &= SPISetup_EnableSPI(SPI_SPI1);
    SPIOperate_HasSS1_Risen();
    return ReturnVal;
}

bool CheckSPIRBF(void)
{
    if (SPI1STATbits.SPIRBF) {
        ES_Event_t ThisEvent;
        ThisEvent.EventType   = SPI_RESPONSE_RECEIVED;
        ThisEvent.EventParam = SPI1BUF;
        PostSPILeaderSM(ThisEvent);
        return true;
    }
}