/****************************************************************************
 Module
   BumperService.c

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
#include "../Sensors/BumperService.h"
#include "../SPI/SPIFollowerSM.h"
#include "../HALs/PIC32PortHAL.h"
#include "../DriveTrain/DriveTrain.h"
#include "terminal.h"
#include "dbprintf.h"

/*----------------------------- Module Defines ----------------------------*/
#define BUMPER_PORT _Port_B //Physical pin 26
#define BUMPER_PIN _Pin_15
#define BUMPER_VAL PORTBbits.RB15
#define BUMPER_TIMEOUT 4000 // 4 sec

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file
static BumperState_t CurrentState;

// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;

bool EventCheckerActive;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitBumperService

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, sets up the initial transition and does any
     other required initialization for this state machine
 Notes

 Author
 Andrew Sack
****************************************************************************/
bool InitBumperService(uint8_t Priority)
{
    MyPriority = Priority;
    // put us into the Initial PseudoState
    CurrentState = BumperIdleState;
    // Disable Event Checker
    EventCheckerActive = false;
    
    // Configure Pin as digital input with pullup
    PortSetup_ConfigureDigitalInputs(BUMPER_PORT, BUMPER_PIN);
    PortSetup_ConfigurePullUps(BUMPER_PORT, BUMPER_PIN);

    return true;
}

/****************************************************************************
 Function
     PostBumperService

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
bool PostBumperService(ES_Event_t ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunBumperService

 Parameters
   ES_Event_t : the event to process

 Returns
   ES_Event_t, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here
 Notes
   uses nested switch/case to implement the machine.
 Author
 Andrew Sack
****************************************************************************/
ES_Event_t RunBumperService(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
    ES_Event_t PostEvent;
    switch (CurrentState)
    {
        case BumperIdleState:
        {
            if (DRIVE_UNTIL_BUMP == ThisEvent.EventType)
            {
                printf("Bumper: Starting DRIVE_UNTIL_BUMP\n");
                CurrentState = BumperActiveState;
                EventCheckerActive = true;
                // start timer for timeout
                ES_Timer_InitTimer(BUMPER_TIMER, BUMPER_TIMEOUT);
            }
        }
        break;

        case BumperActiveState:
        {
            // repost event to other services and return to idle
            if (BUMP_FOUND == ThisEvent.EventType)
            {
                printf("Bumper: BUMP_FOUND\n");
                PostEvent.EventType = BUMP_FOUND;
                PostDriveTrain(PostEvent);
                PostSPIList(PostEvent);
                CurrentState = BumperIdleState;
                EventCheckerActive = false;
                ES_Timer_StopTimer(BUMPER_TIMER); // cancel active timer
            }
            // go to idle state
            else if (DRIVE_STOP == ThisEvent.EventType)
            {
                printf("Bumper: DRIVE_STOP received\n");
                CurrentState = BumperIdleState;
                EventCheckerActive = false;
                ES_Timer_StopTimer(BUMPER_TIMER); // cancel active timer
            }
            // Timeout exceeded
            else if (ES_TIMEOUT == ThisEvent.EventType)
            {
                printf("Bumper: BUMPER Timeout Reached\n");
                PostEvent.EventType = BUMP_FOUND;
                PostDriveTrain(PostEvent);
                PostSPIList(PostEvent);
                CurrentState = BumperIdleState;
                EventCheckerActive = false;
            }
        }
        break;
        default:
          ;
    }                                   // end switch on Current State

    return ReturnEvent;
}

/****************************************************************************
 Function
     QueryBumperService
 Parameters
     None

 Returns
     BumperState_t The current state of the Bumper state machine

 Description
     returns the current state of the Bumper state machine
 Notes

 Author
 Andrew Sack
****************************************************************************/
BumperState_t QueryBumperService(void)
{
    return CurrentState;
}


//Include the event checking function for detecting bumper press
bool Check4Bump(void) {
    if (EventCheckerActive) {
        // pressed is low
        if (BUMPER_VAL == 0) {
            printf("Bumper: Bump triggered\n");
            ES_Event_t PostEvent;
            PostEvent.EventType = BUMP_FOUND;
            PostBumperService(PostEvent);
            EventCheckerActive = false;
            return true;
        }
    }
    
    return false;
}
