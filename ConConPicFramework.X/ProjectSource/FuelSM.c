/****************************************************************************
 Module
   FuelSM.c

 Revision
   1.0.1

 Description
 This module is responsible for keeping track of refueling status

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
#include "FuelSM.h"

/*----------------------------- Module Defines ----------------------------*/

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file
static FuelState_t CurrentState;

// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;

static bool RefuelBitForComms;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitFuelSM

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
bool InitFuelSM(uint8_t Priority)
{
  ES_Event_t ThisEvent;

  MyPriority = Priority;
  
  CurrentState = FuelRemaining;
  
  RefuelBitForComms = false;
  
  return true;
}

/****************************************************************************
 Function
     PostFuelSM

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
bool PostFuelSM(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunFuelSM

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
ES_Event_t RunFuelSM(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

  switch (CurrentState)
  {
    case FuelRemaining:      
    {
      switch (ThisEvent.EventType)
      {
        case ZERO_FUEL:
        {   
            CurrentState = FuelEmpty;
        }
        break;
        
        default:
          ;
      } 
    }
    break;

    case FuelEmpty:
    {
      switch (ThisEvent.EventType)
      {
        case REFUEL_DONE:
        {   
            RefuelBitForComms = true;
        }
        break;

        case NONZERO_FUEL:
        {   
            RefuelBitForComms = false;
            CurrentState = FuelRemaining;
        }
        break;
        
        default:
          ;
      } 
    }
    break;
    
    default:
      ;
  } 
  return ReturnEvent;
}

/****************************************************************************
 Function
     QueryFuelSM

 Parameters
     None

 Returns
     FuelState_t The current state of the Fuel state machine

 Description
     returns the current state of the Fuel state machine
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:21
****************************************************************************/
FuelState_t QueryFuelSM(void)
{
  return CurrentState;
}

bool QueryRefuelBitForComms(void)
{
    return RefuelBitForComms;
}
/***************************************************************************
 private functions
 ***************************************************************************/

