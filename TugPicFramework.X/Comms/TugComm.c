/****************************************************************************
 Module
   TugComm.c

 Revision
   1.0.1

 Description
   FSM to handle high level pairing and communication for the TUG

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------

****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "TugComm.h"
#include "../Propulsion/Propulsion.h"
#include "../HALs/PIC32PortHAL.h"
#include <xc.h>
#include <sys/attribs.h>

/*----------------------------- Module Defines ----------------------------*/
#define DEBUG_PRINT // define to enable debug message printing

/*----------------------------- Module Types ------------------------------*/
/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match that of enum in header file
static TugCommState_t CurrentState;

// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;


/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitTugComm

 Parameters
     uint8_t : the priority of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, sets up the initial transition and does any
     other required initialization for this state machine
 Notes

 Author
 Andrew Sack
****************************************************************************/
bool InitTugComm(uint8_t Priority)
{
    puts("Initializing TugComm...\r");
    ES_Event_t ThisEvent;

    MyPriority = Priority;
    // Initialize into waiting to Pair
    CurrentState = WaitingForPairRequestState;


    puts("...Done Initializing TugComm\r\n");
 
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
     PostTugComm

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
bool PostTugComm(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunTugComm

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
ES_Event_t RunTugComm(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
    
    ES_Event_t PostEvent;
    
    switch (CurrentState)
    {
        case (FuelEmptyState):
        {
            switch (ThisEvent.EventType)
            {
                case (PROPULSION_REFUEL):
                {
                    printf("TugComm: Fuel Empty PROPULSION_REFUEL\r\n");
                } break;
                case (PAIRING_COMPLETE):
                {
                    printf("TugComm: Fuel Empty PAIRING_COMPLETE\r\n");
                } break;
                case (WAIT_TO_PAIR):
                {
                    printf("TugComm: Fuel Empty WAIT_TO_PAIR\r\n");
                } break;
                default:
                    ;
            }
        } break;
        
        case (FuelFullState):
        {
            switch (ThisEvent.EventType)
            {
                case (PROPULSION_SET_THRUST):
                {
                    
                } break;
                case (ES_TIMEOUT):
                {
                    // Make sure it is correct timer
                    if (ThisEvent.EventParam == FUEL_TIMER)
                    {
                    }
                } break;
                case (WAIT_TO_PAIR):
                {
                    printf("TugComm: FuelFull WAIT_TO_PAIR\r\n");
                } break;
                default:
                    ;
            }
        } break;
        
        default:
          ;
    }                                   // end switch on Current State
    return ReturnEvent;
}


/****************************************************************************
 Function
     QueryTugComm

 Parameters
     None

 Returns
     TugCommState_t The current state of the TugComm state machine

 Description
     returns the current state of the TugComm state machine
 Notes

 Author
 Andrew Sack
****************************************************************************/
TugCommState_t QueryTugComm(void)
{
    return CurrentState;
}


/***************************************************************************
 private functions
 ***************************************************************************/

