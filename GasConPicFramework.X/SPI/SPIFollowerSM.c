/****************************************************************************
 Module
   SPIFollowerSM.c

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
#include "SPIFollowerSM.h"
#include "../HALs/PIC32PortHAL.h"
#include "../HALs/PIC32_SPI_HAL.h"
#include "../ProjectHeaders/GasconService.h"
#include "ES_Events.h" 
#include <xc.h>
#include <sys/attribs.h>
#include <proc/p32mx170f256b.h>

/*----------------------------- Module Defines ----------------------------*/
#define SPI_DEBUG
#define DEFAULTMESSAGE 0x00 //Was originally 0xFF
#define REFUELDONEMESSAGE 0xFF //Was originally 0xAA
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
static SPIFollowerSMState_t CurrentState;

static uint8_t ReceiveData;
static uint8_t SendData;

// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;

static bool RefuelInProgress;
static bool RefuelDone;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitSPIFollowerSM

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
bool InitSPIFollowerSM(uint8_t Priority)
{
    puts("Initializing SPIFollowerSM...\r");
    ES_Event_t ThisEvent;

    MyPriority = Priority;
    // put us into the Initial PseudoState
    CurrentState = SPIFollowerInitState;
    SendData = DEFAULTMESSAGE;
    puts("...Done Initializing SPIFollowerSM\r\n");
 
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
     PostSPIFollowerSM

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
bool PostSPIFollowerSM(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunSPIFollowerSM

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
ES_Event_t RunSPIFollowerSM(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
    ES_Event_t PostEvent;
    
    switch (CurrentState)
    {
        case SPIFollowerInitState:        // If current state is initial  State
        {
          if (ThisEvent.EventType == ES_INIT)    // only respond to ES_Init
          {
            if (true == InitializeSPI()) //Initialize SPI for display
            {
                //If successful switch state
                CurrentState = SPIFollowerReceiveState;
                puts("...Done Initializing SPI1\r\n");
            }           
            else // SPI failed to initialize, throw error
            {
                puts("ERROR: Failed to Initialize SPI1\r\n");
                ReturnEvent.EventType = ES_ERROR;
            }  
          }
        }break;
        case SPIFollowerReceiveState:
        {
            if (ThisEvent.EventType == SPI_COMMAND_RECEIVED){
                ReceiveData = ThisEvent.EventParam;
                printf("Fuel Level:  %x\r\n",ReceiveData);
                PostEvent.EventType = GASCON_FUEL; //create event to start scroll
                PostEvent.EventParam = ReceiveData;
                PostGasconService(PostEvent);
            }
            else if (ThisEvent.EventType == GASCON_REFUELED){
                SendData = REFUELDONEMESSAGE; //Message back when refueled
                RefuelDone = true;
            }
        }break;
        default:
          ;
    }                                   // end switch on Current State
    
    return ReturnEvent;
}


/****************************************************************************
 Function
     QuerySPIFollowerSM

 Parameters
     None

 Returns
     SPIFollowerSMState_t The current state of the SPIFollowerSM state machine

 Description
     returns the current state of the Drive Train state machine
 Notes

 Author
 Andrew Sack
****************************************************************************/
SPIFollowerSMState_t QuerySPIFollowerSM(void)
{
    return CurrentState;
}

/****************************************************************************
 Function
 InitializeSPI

 Parameters
     None

 Returns
 True if successful, else false

 Description
 Initializes SPI module as follower
 Notes

 Author
 Andrew Sack
****************************************************************************/
bool InitializeSPI(void)
{
    bool ReturnVal = true;
    SPIOperate_HasSS1_Risen();
    //Configure SPI connection
    ReturnVal &= SPISetup_BasicConfig(SPI_SPI1);
    ReturnVal &= SPISetup_SetFollower(SPI_SPI1);
    ReturnVal &= SPISetup_SetBitTime(SPI_SPI1, 1000); //1,000 ns = 1 MHZ
    SPI1CONbits.SSEN = 0;
    SPI1STATbits.SPIROV = 0;
    //ReturnVal &= SPISetup_MapSSInput(SPI_SPI1, SPI_RPA0); // make A0 SS
    PortSetup_ConfigureDigitalInputs(_Port_A,_Pin_0);
    SS1R = 0b0000;
    ReturnVal &= SPISetup_MapSDOutput(SPI_SPI1, SPI_RPB8); // make B8 SDO
    //ReturnVal &= SPISetup_MapSDInput(SPI_SPI1, SPI_RPB5); // make B5 SDI 
    PortSetup_ConfigureDigitalInputs(_Port_B,_Pin_5);
    SDI1R = 0b0001;
    PortSetup_ConfigureDigitalInputs(_Port_B,_Pin_14);
    ReturnVal &= SPISetup_SetClockIdleState(SPI_SPI1, SPI_CLK_HI); // clock is idle high
    ReturnVal &= SPISetup_SetActiveEdge(SPI_SPI1, SPI_SECOND_EDGE); // read on 2nd edge 
    ReturnVal &= SPISetup_SetXferWidth(SPI_SPI1, SPI_8BIT); //16 bit messages
    ReturnVal &= SPISetEnhancedBuffer(SPI_SPI1, false); //disable ENHBUF
    //After everything set up, enable spi
    ReturnVal &= SPISetup_EnableSPI(SPI_SPI1);
    SPIOperate_HasSS1_Risen();
    return ReturnVal;
}

/****************************************************************************
 Function
 CheckSPIRBF

 Parameters
     None

 Returns
 True if something read from buffer

 Description
 Check if something is in SPI buffer and if so, post an event
 Notes

 Author
 Andrew Sack
****************************************************************************/
bool CheckSPIRBF(void)
{
    if (SPI1STATbits.SPIRBF) {
        //printf("%x\r\n",SendData);
        SPIOperate_SPI1_Send8(SendData);
        //SendData = DEFAULTMESSAGE; //Standard message back
        ES_Event_t ThisEvent;
        ThisEvent.EventType   = SPI_COMMAND_RECEIVED;
        ThisEvent.EventParam = SPI1BUF;
        PostSPIFollowerSM(ThisEvent);
        return true;
    }
}

bool QueryRefuelInProgress(void)
{
    return RefuelInProgress;
}
        
void ClearRefuelInProgress(void)
{
    RefuelInProgress = false;
    SendData = DEFAULTMESSAGE;
    RefuelDone = false;
    return;
}

void SetRefuelInProgress(void)
{
    RefuelInProgress = true;
    return;
}

bool QueryRefuelDone(void)
{
    return RefuelDone;
}
        
void ClearRefuelDone(void)
{
    RefuelDone = false;
    return;
}