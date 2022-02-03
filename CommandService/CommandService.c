/****************************************************************************
 Module
   CommandService.c

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
#include "CommandService.h"
#include "ES_DeferRecall.h"
#include "../HALs/PIC32PortHAL.h"
#include "../HALs/PIC32_SPI_HAL.h" 
#include <xc.h>
#include <sys/attribs.h>
#include "../DriveTrain/DriveTrain.h"

/*----------------------------- Module Defines ----------------------------*/

#define COMMAND_TIME 100

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
static CommandServiceState_t CurrentState;

static uint8_t LastCommand;
static uint8_t ThisCommand;

// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitCommandService

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
bool InitCommandService(uint8_t Priority)
{
    puts("Initializing CommandService...\r");
    ES_Event_t ThisEvent;

    MyPriority = Priority;
    LastCommand = 0xFF;
    ThisCommand = 0xFF;
    // put us into the Initial PseudoState
    CurrentState = CommandInitState;
    
    puts("...Done Initializing CommandService\r\n");
 
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
     PostCommandService

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
bool PostCommandService(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunCommandService

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
ES_Event_t RunCommandService(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
    ES_Event_t PostEvent;
    
    switch (CurrentState)
    {
        case CommandInitState:        // If current state is initial  State
        {
          if (ThisEvent.EventType == ES_INIT)    // only respond to ES_Init
          {
            if (true == InitializeSPI()) //Initialize SPI for display
            {
                //If successful switch state
                CurrentState = CommandQueryState;
                ES_Timer_InitTimer(COMMAND_TIMER,COMMAND_TIME);
                puts("...Done Initializing SPI1\r\n");
            }           
            else // SPI failed to initialize, throw error
            {
                puts("ERROR: Failed to Initialize SPI1\r\n");
                ReturnEvent.EventType = ES_ERROR;
            }  
          }
        }
        case CommandQueryState:
        {
            if (ThisEvent.EventType == ES_QUERY)
            {
                SPIOperate_SPI1_Send8(0xAA);
                CurrentState = CommandReceiveState;
                //printf("Sent \r\n");
            }
            if (ThisEvent.EventType == ES_TIMEOUT){
                ES_Event_t ThisEvent;
                ThisEvent.EventType  = ES_QUERY;
                PostCommandService(ThisEvent);
                ES_Timer_InitTimer(COMMAND_TIMER,COMMAND_TIME);
            }
        }
        case CommandReceiveState:
        {
            if (ThisEvent.EventType == ES_RECEIVED)
            {
                ThisCommand = SPI1BUF;                
                if (ThisCommand == 0xFF)
                {
                    //Do nothing
                }
                else if (LastCommand == 0xFF)
                {
                    //Add give up event here
                    
                    printf("Received: %x\r\n", ThisCommand);
                    if (ThisCommand == 0x00)
                    {
                        PostEvent.EventType = DRIVE_STOP_MOTORS;
                        //PostDriveTrain(PostEvent);
                    }
                    else if (ThisCommand == 0x02)
                    {
                        PostEvent.EventType = DRIVE_ROTATE_CW90;
                        //PostDriveTrain(PostEvent);
                    }
                    else if (ThisCommand == 0x03)
                    {
                        PostEvent.EventType = DRIVE_ROTATE_CW45;
                        //PostDriveTrain(PostEvent);
                    }
                    else if (ThisCommand == 0x04)
                    {
                        PostEvent.EventType = DRIVE_ROTATE_CCW90;
                        //PostDriveTrain(PostEvent);
                    }
                    else if (ThisCommand == 0x05)
                    {
                        PostEvent.EventType = DRIVE_ROTATE_CCW45;
                        //PostDriveTrain(PostEvent);
                    }
                    else if (ThisCommand == 0x08)
                    {
                        PostEvent.EventType = DRIVE_FORWARD_HALF;
                        //PostDriveTrain(PostEvent);
                    }
                    else if (ThisCommand == 0x09)
                    {
                        PostEvent.EventType = DRIVE_FORWARD_FULL;
                        //PostDriveTrain(PostEvent);
                    }
                    else if (ThisCommand == 0x10)
                    {
                        PostEvent.EventType = DRIVE_BACKWARD_HALF;
                        //PostDriveTrain(PostEvent);
                    }
                    else if (ThisCommand == 0x11)
                    {
                        PostEvent.EventType = DRIVE_BACKWARD_FULL ;
                        //PostDriveTrain(PostEvent);
                    }
                    else if (ThisCommand == 0x20)
                    {
                        //PostEvent.EventType = DRIVE_ROTATE_CWINF ;
                        //PostDriveTrain(PostEvent);
                    }
                    else if (ThisCommand == 0x40)
                    {
                        //PostEvent.EventType = DRIVE_ROTATE_CCWINF ;
                        //PostDriveTrain(PostEvent);
                    }
                }
                LastCommand = ThisCommand;
                CurrentState = CommandQueryState;
            }
            if (ThisEvent.EventType == ES_TIMEOUT){
                ES_Timer_InitTimer(COMMAND_TIMER,COMMAND_TIME);
            }
        }
        default:
          ;
    }                                   // end switch on Current State
    
    return ReturnEvent;
}


/****************************************************************************
 Function
     QueryCommandService

 Parameters
     None

 Returns
     CommandServiceState_t The current state of the CommandService state machine

 Description
     returns the current state of the Drive Train state machine
 Notes

 Author
 Andrew Sack
****************************************************************************/
CommandServiceState_t QueryCommandService(void)
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
    ReturnVal &= SPISetup_SetXferWidth(SPI_SPI1, SPI_8BIT); //8 bit messages
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
        ThisEvent.EventType   = ES_RECEIVED;
        PostCommandService(ThisEvent);
        return true;
    }
}