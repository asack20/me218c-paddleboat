/****************************************************************************
 Module
   GasconService.c

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
#include "GasconService.h"
#include "ES_DeferRecall.h"
#include "../ProjectHeaders/BraidService.h"
#include "../HALs/PIC32PortHAL.h"
#include "../HALs/PIC32_AD_Lib.h"
#include "../FrameworkHeaders/bitdefs.h"
#include "../SPI/SPIFollowerSM.h"

/*----------------------------- Module Defines ----------------------------*/
#define DEBUG_PRINT 0 // T/F to enable debug prints

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/
bool InitializeSPI2(void);
/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file
static GasconState_t CurrentState;

// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;
// add a deferral queue for up to 3 pending deferrals +1 to allow for overhead
static ES_Event_t DeferralQueue[3 + 1];

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitGasconService

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
bool InitGasconService(uint8_t Priority)
{
  ES_Event_t ThisEvent;
  puts("Initializing GasconService\n\r");
  MyPriority = Priority;
  // initialize deferral queue for testing Deferral function
  ES_InitDeferralQueueWith(DeferralQueue, ARRAY_SIZE(DeferralQueue));
  // put us into the Initial PseudoState
  CurrentState = InitGasconPseudoState;
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
     PostGasconService

 Parameters
     EF_Event_t ThisEvent , the event to post to the queue

 Returns
     boolean False if the Enqueue operation failed, True otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:25
****************************************************************************/
bool PostGasconService(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunGasconService

 Parameters
   ES_Event_t : the event to process

 Returns
   ES_Event_t, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here
 Notes
   uses nested switch/case to implement the machine.
 Author
   J. Edward Carryer, 01/15/12, 15:23
****************************************************************************/
ES_Event_t RunGasconService(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ES_Event_t PostEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  
  //DB_printf( "RunGasconService: State %d Event %d \n\r", CurrentState, ThisEvent.EventType);
  switch (CurrentState)
  {
    case InitGasconPseudoState:        // If current state is initial Psedudo State
    {
      if (ThisEvent.EventType == ES_INIT)    // only respond to ES_Init
      {
        if (true == InitializeSPI2()) //Initialize SPI for display
        {
            //If successful Switch state and write another ES_INIT event
            PostEvent.EventType = ES_INIT;
            PostGasconService(PostEvent);
            CurrentState = InitializeGasconState; 
            //puts("\n\r\n\r Initializing SPI2 \n\r\n\r");
        }           
        else // SPI failed to initialize, throw error
        {
            puts("GasconService: ERROR: Failed to Initialize SPI.\n\r");
            ReturnEvent.EventType = ES_ERROR;
        }  
      }
    }
    break;

    case InitializeGasconState: 
    {
      if (ThisEvent.EventType == ES_INIT)    // only respond to ES_Init
      {
          if (false == DM2_TakeInitDisplayStep()) // Take a display step
          {
            //if display init not complete, post another event
            PostEvent.EventType = ES_INIT;
            PostGasconService(PostEvent);
            //puts("\n\r\n\r Initializing Display2 \n\r\n\r");
          }
          else // display init done
          {
              CurrentState = GasconReadyState; //move to ready state  
          }
      }
    }
    break;
    
    case GasconReadyState:        // If current state is state one
    {
      if (ThisEvent.EventType == GASCON_FUEL) // There is new data to scroll
      {
        uint16_t RawFuelValue = ThisEvent.EventParam;
        //uint16_t ScaledFuelValue = RawFuelValue*RawFuelValue; //Fix this scaling at some point
        uint16_t ScaledFuelValue = (RawFuelValue>>3); //Fix this scaling at some point
        DM2_AddRow2DisplayBuffer(ScaledFuelValue); // add row to buffer
        CurrentState = GasconScrollingState; // move to scrolling state
        PostEvent.EventType = GASCON_UPDATE_DISPLAY; //create event to start scroll
        PostGasconService(PostEvent);
        printf("Refuel In Progress:  %d\r\n", QueryRefuelInProgress());
        printf("Refuel Done:  %d\r\n", QueryRefuelDone());
        if ((RawFuelValue < 1) && (!QueryRefuelInProgress())){
            SetRefuelInProgress();
            ClearRefuelDone();
            ES_Event_t ThisEvent;
            ThisEvent.EventType = BRAID_START;
            PostBraidService(ThisEvent);
        }
        else if ((RawFuelValue >= 1) && QueryRefuelDone()) {
            ClearRefuelInProgress();
        }
      }
    }
    break;
    
    case GasconScrollingState:        // If current state is scrolling
    {
      if (ThisEvent.EventType == GASCON_UPDATE_DISPLAY)
      {
          if (false == DM2_TakeDisplayUpdateStep()) // Take a display step
          {
            //if display update not complete, post another event
            PostEvent.EventType = GASCON_UPDATE_DISPLAY;
            PostGasconService(PostEvent);
          }
          else // display update done 
          {
              //Recall any events in queue
              ES_RecallEvents(MyPriority, DeferralQueue); 
              CurrentState = GasconReadyState; //move to ready state 
          }
      }
      else if (ThisEvent.EventType == GASCON_FUEL)// defer detect events
      {
          ES_DeferEvent(DeferralQueue, ThisEvent);        
      }     
    }
    break;

    // repeat state pattern as required for other states
    default:
      ;
  }                                   // end switch on Current State
  return ReturnEvent;
}

/****************************************************************************
 Function
     QueryDisplaySM

 Parameters
     None

 Returns
     GasconState_t The current state of the Display state machine

 Description
     returns the current state of the Display state machine
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:21
****************************************************************************/
GasconState_t QueryGasconService(void)
{
  return CurrentState;
}

/***************************************************************************
 private functions
 ***************************************************************************/
/****************************************************************************
 Function
 InitializeSPI2

 Parameters
     None

 Returns
     ReturnVal; True if SPI was initialized successfully, Else: false

 Description
 Initializes SPI to parameters required for the display
 * SPI_SMP_MID
 * 100 kHz clock
 * SS on A0
 * SDO on A1
 * Clock Idle Low
 * Read 1st edge
 * 16bit transfers
 
****************************************************************************/
bool InitializeSPI2(void)
{
    bool ReturnVal = true;
    SPIOperate_HasSS2_Risen();
    //Configure SPI connection
    ReturnVal &= SPISetup_BasicConfig(SPI_SPI2);
    ReturnVal &= SPISetup_SetLeader(SPI_SPI2, SPI_SMP_MID);
    ReturnVal &= SPISetup_SetBitTime(SPI_SPI2, 10000); //10,000 ns = 100 kHZ
    SPI2CONbits.SSEN = 0;
    SPI2STATbits.SPIROV = 0;
    PortSetup_ConfigureDigitalOutputs(_Port_B,_Pin_9);
    ReturnVal &= SPISetup_MapSSOutput(SPI_SPI2, SPI_RPB9); // make B9 SS
    PortSetup_ConfigureDigitalOutputs(_Port_B,_Pin_11);
    ReturnVal &= SPISetup_MapSDOutput(SPI_SPI2, SPI_RPB11); // make B11 SDO
    PortSetup_ConfigureDigitalOutputs(_Port_B,_Pin_15);
    ReturnVal &= SPISetup_SetClockIdleState(SPI_SPI2, SPI_CLK_LO); // clock is idle low
    ReturnVal &= SPISetup_SetActiveEdge(SPI_SPI2, SPI_FIRST_EDGE); // read on 1st edge 
    ReturnVal &= SPISetup_SetXferWidth(SPI_SPI2, SPI_16BIT); //16 bit messages
    ReturnVal &= SPISetEnhancedBuffer(SPI_SPI2, true); //enable ENHBUF
    
    PortSetup_ConfigureDigitalInputs(_Port_B,_Pin_13); //SDI if we need it
    
    //After everything set up, enable spi
    ReturnVal &= SPISetup_EnableSPI(SPI_SPI2);
    SPIOperate_HasSS2_Risen();
    return ReturnVal;
}

