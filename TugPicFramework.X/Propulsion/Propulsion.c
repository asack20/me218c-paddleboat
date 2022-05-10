/****************************************************************************
 Module
   Propulsion.c

 Revision
   1.0.1

 Description
   FSM to interface with 2-motor PWM drive train and control fuel level

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
#include "Propulsion.h"
#include "MotorControlDriver.h"
#include "../HALs/PIC32PortHAL.h"
#include <xc.h>
#include <sys/attribs.h>

/*----------------------------- Module Defines ----------------------------*/
#define LEFT_MIN_DUTY_CYCLE 0 // Duty cycle at which the left motor starts moving
#define RIGHT_MIN_DUTY_CYCLE 0 // Duty cycle at which the right motor starts moving
#define FULL_FUEL 255 

#define FUEL_BURN_TIME 200 // msec = 5 Hz

/*----------------------------- Module Types ------------------------------*/
/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/
void SetThrust(ArcadeControl_t input);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match that of enum in header file
static PropulsionState_t CurrentState;

// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;

static float FuelLevel;
static float FuelBurnRate;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitPropulsion

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
bool InitPropulsion(uint8_t Priority)
{
    puts("Initializing Propulsion...\r");
    ES_Event_t ThisEvent;

    MyPriority = Priority;
    // put us into the Initial State
    CurrentState = FuelEmptyState;

    InitMotorControlDriver();
    MotorControl_StopMotors();
    
    // Start Fuel as full and burn rate as 0
    FuelLevel = FULL_FUEL;
    FuelBurnRate = 0;
    
    puts("...Done Initializing Propulsion\r\n");
 
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
     PostPropulsion

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
bool PostPropulsion(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunPropulsion

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
ES_Event_t RunPropulsion(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
    
    ES_Event_t PostEvent;
    
    switch (CurrentState)
    {
        case (FuelEmptyState):
        {
            switch (ThisEvent.EventParam)
            {
                case (PROPULSION_REFUEL):
                {
                    FuelLevel = FULL_FUEL;
                    CurrentState = FuelFullState;
                    // Start fuel burning timer
                    ES_Timer_InitTimer(FUEL_TIMER, FUEL_BURN_TIME);
                } break;
                case (PAIRING_COMPLETE):
                {
                    FuelLevel = FULL_FUEL;
                    CurrentState = FuelFullState;
                    // Start fuel burning timer
                    ES_Timer_InitTimer(FUEL_TIMER, FUEL_BURN_TIME);
                } break;
                case (WAIT_TO_PAIR):
                {
                    // Disable Motors when pairing
                    MotorControl_StopMotors();
                    FuelBurnRate = 0;
                } break;
                default:
                    ;
            }
        } break;
        
        case (FuelFullState):
        {
            switch (ThisEvent.EventParam)
            {
                case (PROPULSION_SET_THRUST):
                {
                    SetThrust((ArcadeControl_t) ThisEvent.EventParam);
                } break;
                case (ES_TIMEOUT):
                {
                    // Make sure it is correct timer
                    if (ThisEvent.EventParam == FUEL_TIMER)
                    {
                        FuelLevel -= FuelBurnRate; // Decrement Fuel
                        
                        if (FuelLevel > 0)
                        {
                            // Restart fuel burning timer
                            ES_Timer_InitTimer(FUEL_TIMER, FUEL_BURN_TIME);
                        }
                        else
                        {
                            // Stop motors and go to FuelEmpty State
                            MotorControl_StopMotors();
                            FuelBurnRate = 0;
                            CurrentState = FuelEmptyState;
                        }
                    }
                } break;
                case (WAIT_TO_PAIR):
                {
                    // Disable Motors when pairing
                    MotorControl_StopMotors();
                    FuelBurnRate = 0;
                    CurrentState = FuelEmptyState;
                    // Stop fuel burning timer
                    ES_Timer_StopTimer(FUEL_TIMER);
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
     QueryPropulsion

 Parameters
     None

 Returns
     PropulsionState_t The current state of the Propulsion state machine

 Description
     returns the current state of the Propulsion state machine
 Notes

 Author
 Andrew Sack
****************************************************************************/
PropulsionState_t QueryPropulsion(void)
{
    return CurrentState;
}

/****************************************************************************
 * Function
 *      Propulsion_GetFuelLevel
 *      
 * Parameters
 *      void
 * Return
 *      Fuellevel as a uint8
 * Description
 *      Returns the current FuelLevel as a uint8
****************************************************************************/
uint8_t Propulsion_GetFuelLevel(void)
{
    if (FuelLevel <= 0) return 0;
    if (FuelLevel >= 255) return 255;
    return (uint8_t) FuelLevel;
    
}

/***************************************************************************
 private functions
 ***************************************************************************/
void SetThrust(ArcadeControl_t input)
{
    
}