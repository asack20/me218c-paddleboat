/****************************************************************************
 Module
   BraidService.c

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
#include "BraidService.h"
#include "AnalogChecker.h"
#include "../SPI/SPIFollowerSM.h"
#include "../HALs/PIC32PortHAL.h"
#include "../FrameworkHeaders/bitdefs.h"
#include <stdlib.h>

/*----------------------------- Module Defines ----------------------------*/

#define BLUELED                LATAbits.LATA2
#define GREENLED               LATAbits.LATA3
#define REDLED                 LATAbits.LATA4

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/
/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file
static BraidState_t CurrentState;

// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;
static FullBraidState Config;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitBraidService

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
bool InitBraidService(uint8_t Priority)
{
  ES_Event_t ThisEvent;
  puts("Initializing BraidService\n\r");
  MyPriority = Priority;
  // put us into the Initial PseudoState
  CurrentState = WaitState;
  PortSetup_ConfigureDigitalOutputs(_Port_A, _Pin_2);//Low LED
  PortSetup_ConfigureDigitalOutputs(_Port_A, _Pin_3);//Mid LED
  PortSetup_ConfigureDigitalOutputs(_Port_A, _Pin_4);//High LED
  Config.FullState = 0;
  InitBraidStatus();
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
     PostBraidService

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
bool PostBraidService(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunBraidService

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
ES_Event_t RunBraidService(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ES_Event_t PostEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  
  //DB_printf( "RunBraidService: State %d Event %d \n\r", CurrentState, ThisEvent.EventType);
  switch (CurrentState)
  {
    case WaitState:        // If current state is initial Psedudo State
    {
      if (ThisEvent.EventType == BRAID_START)    // only respond to ES_Init
      {
          //Pick Config
          uint8_t RandNum = (rand() % 3) + 1; //Make this random
          BraidState Random = RandNum;
          Config.Braid1 = Random;
          Config.Braid2 = Random;
          //Config.Braid3 = Random;
          CurrentState = RefuelState;
          if (Random == BraidWhite) {
              REDLED = 1;
              GREENLED = 1;
              BLUELED = 1;
          }
          else if (Random == BraidGreen) {
              REDLED = 0;
              GREENLED = 1;
              BLUELED = 0;
          }
          else if (Random == BraidBlue) {
              REDLED = 0;
              GREENLED = 0;
              BLUELED = 1;
          }
          else if (Random == BraidRed) {
              REDLED = 1;
              GREENLED = 0;
              BLUELED = 0;
          }
      }
    }
    break;

    case RefuelState: 
    {
      if (ThisEvent.EventType == BRAID_UPDATE)    // only respond to ES_Init
      {
          if(ThisEvent.EventParam == Config.FullState){
              PostEvent.EventType = GASCON_REFUELED;
              PostSPIFollowerSM(PostEvent);
              CurrentState = WaitState;
              REDLED = 0;
              GREENLED = 0;
              BLUELED = 0;
          }
      }
      if (ThisEvent.EventType == RESET_BRAID){
            CurrentState = WaitState;
            REDLED = 0;
            GREENLED = 0;
            BLUELED = 0;
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
     BraidState_t The current state of the Display state machine

 Description
     returns the current state of the Display state machine
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:21
****************************************************************************/
BraidState_t QueryBraidService(void)
{
  return CurrentState;
}

/***************************************************************************
 private functions
 ***************************************************************************/

