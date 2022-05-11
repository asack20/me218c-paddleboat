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
#define DEBUG_PRINT // define to enable debug message printing

#define LEFT_MIN_DUTY_CYCLE 100 // Duty cycle at which the left motor starts moving
#define RIGHT_MIN_DUTY_CYCLE 100 // Duty cycle at which the right motor starts moving
#define MAX_DUTY_CYCLE 1000 // Max duty cycle input
#define MAX_INPUT 127 // max val of x and yaw

#define FULL_FUEL 255 
#define FUEL_BURN_TIME 200 // msec = 5 Hz
#define ONE_SEC 1000
#define FUEL_BURN_PER_SEC 25.5 // Fuel burn at 100% thrust per second

#define abs(a) ((a) >= 0 ?  (a) : -1*(a))

/*----------------------------- Module Types ------------------------------*/
/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/
void SetThrust(ArcadeControl_t input);
uint16_t Propulsion_SetMotorDutyCycle(MotorControl_Motor_t WhichMotor, float Thrust);

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
            switch (ThisEvent.EventType)
            {
                case (PROPULSION_REFUEL):
                {
                    printf("Propulsion: Fuel Empty PROPULSION_REFUEL\r\n");
                    FuelLevel = FULL_FUEL;
                    CurrentState = FuelFullState;
                    // Start fuel burning timer
                    ES_Timer_InitTimer(FUEL_TIMER, FUEL_BURN_TIME);
                } break;
                case (PAIRING_COMPLETE):
                {
                    printf("Propulsion: Fuel Empty PAIRING_COMPLETE\r\n");
                    FuelLevel = FULL_FUEL;
                    CurrentState = FuelFullState;
                    // Start fuel burning timer
                    ES_Timer_InitTimer(FUEL_TIMER, FUEL_BURN_TIME);
                } break;
                case (WAIT_TO_PAIR):
                {
                    printf("Propulsion: Fuel Empty WAIT_TO_PAIR\r\n");
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
            switch (ThisEvent.EventType)
            {
                case (PROPULSION_SET_THRUST):
                {
                    printf("Propulsion: FuelFull PROPULSION_SET_THRUST\r\n");
                    SetThrust((ArcadeControl_t) ThisEvent.EventParam);
                } break;
                case (ES_TIMEOUT):
                {
                    //printf("Propulsion: FuelFull ES_TIMEOUT\r\n");
                    // Make sure it is correct timer
                    if (ThisEvent.EventParam == FUEL_TIMER)
                    {
                        FuelLevel -= FuelBurnRate; // Decrement Fuel
                        //printf("Burned Fuel. Level: %0.3f \t Rate: %0.5f \r\n", FuelLevel, FuelBurnRate );
                        
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
                    printf("Propulsion: FuelFull WAIT_TO_PAIR\r\n");
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

/****************************************************************************
 Function
 SetThrust

 Parameters
 ArcadeControl_t input

 Returns
 void

 Description
 Converts control input to L&R thrust and sets motor duty cycles
 Calculates thrust percentage and sets FuelBurnRate
 
 Notes

 Author
 Andrew Sack
****************************************************************************/
void SetThrust(ArcadeControl_t input)
{
    printf("Propulsion: Setting thrust %x\r\n", input.Total);
    // Convert To floats betwen -1 and 1 for math
    float X_ratio = (float) input.X / MAX_INPUT;
    float Yaw_ratio = (float) input.Yaw / MAX_INPUT;
    
    //variables to determine the quadrants 
    float maximum = max(abs(X_ratio), abs(Yaw_ratio));
    float total = X_ratio + Yaw_ratio;
    float difference = X_ratio - Yaw_ratio;
    
    float LeftThrust;
    float RightThrust;

    // set thrust according to the quadrant that the values are in
    if (X_ratio >= 0)
    {
        if (Yaw_ratio >= 0) // 1st quadrant
        {
            LeftThrust = maximum;
            RightThrust = difference;
        }
        else // 2nd quadrant
        {
            LeftThrust = total;
            RightThrust = maximum;
        }
    }
    else
    {
        if (Yaw_ratio >= 0) // 4th quadrant
        {
            LeftThrust = total;
            RightThrust = -1 * maximum;
        }
        else // 3rd quadrant
        {
            LeftThrust = -1 * maximum;
            RightThrust = difference;
        }
    }  
    
    // Set motor duty cycles according to thrust
    uint16_t LeftDC = Propulsion_SetMotorDutyCycle(_Left_Motor, LeftThrust);
    uint16_t RightDC = Propulsion_SetMotorDutyCycle(_Right_Motor, RightThrust);
    
    // Calculate FuelBurnRate per timer cycle based on average of 2 thrusts
    FuelBurnRate = (abs(LeftThrust) + abs(RightThrust)) * FUEL_BURN_PER_SEC * FUEL_BURN_TIME / (2*ONE_SEC);
    
#ifdef DEBUG_PRINT
    printf("X: %d \t Yaw: %d \t Fuel: %0.3f \t Burn: %0.5f \r\nLeft Thrust: %0.3f \t Right Thrust %0.3f Left DC: %u \t Right DC: %u \r\n\r\n", 
            input.X, input.Yaw, FuelLevel, FuelBurnRate, LeftThrust, RightThrust, LeftDC, RightDC );
#endif
}

/* Propulsion_SetMotorDutyCycle
 * Helper to scale thrust to PWM value and set motor duty cycle accordingly
 * 
 * Returns DutyCycle that was set
 */
uint16_t Propulsion_SetMotorDutyCycle(MotorControl_Motor_t WhichMotor, float Thrust)
{
    
    MotorControl_Direction_t WhichDir;
    
    // Record thrust direction then take absolute value
    if (Thrust >= 0) {
        WhichDir = _Forward_Dir;
    }
    else{
        WhichDir = _Backward_Dir;
    }
    Thrust = abs(Thrust);
    
    uint16_t DutyCycle;

    // Convert to Duty Cycle with Minimum DC offset
    if (Thrust == 0) // turn motor all the way off to get rid of noise
    {
        DutyCycle = 0;
    }
    else if (WhichMotor == _Left_Motor)
    {
        DutyCycle = Thrust * (MAX_DUTY_CYCLE - LEFT_MIN_DUTY_CYCLE) 
                + LEFT_MIN_DUTY_CYCLE;
    }
    else // Right motor
    {
        DutyCycle = Thrust * (MAX_DUTY_CYCLE - RIGHT_MIN_DUTY_CYCLE) 
                + RIGHT_MIN_DUTY_CYCLE;
    }
    
    // Set motor 
    MotorControl_SetMotorDutyCycle(WhichMotor, WhichDir, DutyCycle);
    
    return DutyCycle;
}