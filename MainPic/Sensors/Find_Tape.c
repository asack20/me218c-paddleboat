/****************************************************************************
 Module
   Find_Tape.c

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
#include <sys/attribs.h>
#include "../SensorInterfacing/Find_Tape.h"
#include "../ProjectHeaders/PIC32_AD_Lib.h"
#include "../HALs/PIC32PortHAL.h"
#include "../DriveTrain/DriveTrain.h"
#include "terminal.h"
#include "dbprintf.h"

/*----------------------------- Module Defines ----------------------------*/

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/

static bool SetupADC(void);
static bool TakeNewReading(uint32_t *);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file
static Find_TapeState_t CurrentState;

// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;

static uint32_t LastADCValue[1];
static bool EventCheckerActive = 0;
static bool TapeFound = false;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitFind_Tape

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
bool InitFind_Tape(uint8_t Priority)
{
  ES_Event_t ThisEvent;

  MyPriority = Priority;
  // put us into the Initial PseudoState
  CurrentState = Waiting2;
  
  SetupADC();
  
  // post the initial transition event
//  ThisEvent.EventType = ES_INIT;
//  if (ES_PostToService(MyPriority, ThisEvent) == true)
//  {
//    return true;
//  }
//  else
//  {
//    return false;
//  }
  return true;
}

/****************************************************************************
 Function
     PostFind_Tape

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
bool PostFind_Tape(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunFind_Tape

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
ES_Event_t RunFind_Tape(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  ES_Event_t PostEvent;
  
  switch (CurrentState)
  {
    case Waiting2:
    {
        switch (ThisEvent.EventType)
        {
            case FIND_TAPE:
            {
                //Begin driving forward
                PostEvent.EventType = DRIVE_FORWARD_FULL;
                PostDriveTrain(PostEvent);
                //Activate event checker
                EventCheckerActive = true;
                TapeFound = false;
                
                CurrentState = Searching2;
            }
            break;
            
            default:
                ;
        }
    }
    break;

    case Searching2:
    {
      switch (ThisEvent.EventType) {
          case TAPE_FOUND:
          {
              //Stop moving
              PostEvent.EventType = DRIVE_STOP_MOTORS;
              PostDriveTrain(PostEvent);
              DB_printf("Tape found\n");
              
              CurrentState = Waiting2;
          }
          break;
          
          case GIVE_UP:
          {
              //Stop moving
              PostEvent.EventType = DRIVE_STOP_MOTORS;
              PostDriveTrain(PostEvent);
              DB_printf("Tape was not found\n");
              
              EventCheckerActive = false;
              
              CurrentState = Waiting2;
          }
          
          default:
              ;
      }
    }
    break;
    // repeat state pattern as required for other states
    default:
      ;
  }                                   // end switch on Current State
  
  //Take a new reading if it's time to.
  if ((ThisEvent.EventType == ES_TIMEOUT) && (ThisEvent.EventParam == TAPE_DETECT_ADC_TIMER)) {
      TakeNewReading(LastADCValue);
  }
  
  return ReturnEvent;
}

/****************************************************************************
 Function
     QueryFind_Tape

 Parameters
     None

 Returns
     Find_TapeState_t The current state of the Template state machine

 Description
     returns the current state of the Template state machine
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:21
****************************************************************************/
Find_TapeState_t QueryFind_Tape(void)
{
  return CurrentState;
}


//Include the event checking function for detecting tape
bool Check4Tape(void) {
    if (EventCheckerActive) {
        if (LastADCValue[0]>250) {
            TapeFound = true;
            ES_Event_t NewEvent;
            NewEvent.EventType = TAPE_FOUND;
            PostFind_Tape(NewEvent);
            EventCheckerActive = false;
            return true;
        }
    }
    
    return false;
}

/***************************************************************************
 private functions
 ***************************************************************************/

static bool SetupADC(void) {
    PortSetup_ConfigureAnalogInputs(_Port_B,_Pin_13);
    //	Set up ADC on input AN0 using the command ADC_ConfigAutoScan(1<<11,1)
    ADC_ConfigAutoScan(1<<11,1);
    
    ES_Timer_InitTimer(TAPE_DETECT_ADC_TIMER,100);
    
    ADC_MultiRead(LastADCValue);
    
    return true;
}

static bool TakeNewReading(uint32_t * StoragePointer) {
    ADC_MultiRead(StoragePointer);
    
    ES_Timer_InitTimer(TAPE_DETECT_ADC_TIMER,100);
    
    return true;
}
