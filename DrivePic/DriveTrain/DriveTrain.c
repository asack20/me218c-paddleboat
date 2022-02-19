/****************************************************************************
 Module
   DriveTrain.c

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
#include "DriveTrain.h"
#include "MotorControlDriver.h"
#include "../HALs/PIC32PortHAL.h"
#include <xc.h>
#include <sys/attribs.h>

/*----------------------------- Module Defines ----------------------------*/
// Command Tuning
#define FULL_DUTY_CYCLE 1000 // Duty cycle for 100% speed
#define HALF_DUTY_CYCLE 500 // Duty cycle for 50% speed
#define ROT_DUTY_CYCLE 1000 // Duty cycle used for rotating
#define LOW_SPEED 200
#define HIGH_SPEED 1000

#define ROT_90_TIME 1500 // time in ms required to rotate 90 degrees
#define ROT_45_TIME 750 // time in ms required to rotate 45 degrees

/*----------------------------- Module Types ------------------------------*/
/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/
/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file
static DriveTrainState_t CurrentState;
static bool MotorsActive; // true if motors are moving in any way. False if stopped

// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitDriveTrain

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
bool InitDriveTrain(uint8_t Priority)
{
    puts("Initializing DriveTrain...\r");
    ES_Event_t ThisEvent;

    MyPriority = Priority;
    // put us into the Initial PseudoState
    CurrentState = DriveInitState;

    InitMotorControlDriver();
    
    puts("...Done Initializing DriveTrain\r\n");
 
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
     PostDriveTrain

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
bool PostDriveTrain(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunDriveTrain

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
ES_Event_t RunDriveTrain(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
    
    ES_Event_t PostEvent;
    
    switch (CurrentState)
    {
        case DriveInitState:
        {
            if (ThisEvent.EventType == ES_INIT)
            {
                MotorControl_StopMotors(); // make sure motors are off
                CurrentState = DriveReadyState;
            }
        } break;
        case DriveReadyState:
        {
            switch (ThisEvent.EventType)
            {
                case DRIVE_STOP_MOTORS:
                {
                    MotorControl_StopMotors();
                    CurrentState = DriveReadyState;
                } break;
               
                case DRIVE_ROTATE_CW90:
                {
                    MotorControl_SetMotorDutyCycle(_Left_Motor, _Forward_Dir, ROT_DUTY_CYCLE);
                    MotorControl_SetMotorDutyCycle(_Right_Motor, _Backward_Dir, ROT_DUTY_CYCLE);
                    DriveTrain_StopAfterDelay(ROT_90_TIME);
                    CurrentState = DriveBusyState;
                } break;
                
                case DRIVE_ROTATE_CW45:
                {
                    MotorControl_SetMotorDutyCycle(_Left_Motor, _Forward_Dir, ROT_DUTY_CYCLE);
                    MotorControl_SetMotorDutyCycle(_Right_Motor, _Backward_Dir, ROT_DUTY_CYCLE);
                    DriveTrain_StopAfterDelay(ROT_45_TIME);
                    CurrentState = DriveBusyState;
                } break;

                case DRIVE_ROTATE_CCW90:
                {
                    MotorControl_SetMotorDutyCycle(_Left_Motor, _Backward_Dir, ROT_DUTY_CYCLE);
                    MotorControl_SetMotorDutyCycle(_Right_Motor, _Forward_Dir, ROT_DUTY_CYCLE);
                    DriveTrain_StopAfterDelay(ROT_90_TIME);
                    CurrentState = DriveBusyState;
                } break;
                
                case DRIVE_ROTATE_CCW45:
                {
                    MotorControl_SetMotorDutyCycle(_Left_Motor, _Backward_Dir, ROT_DUTY_CYCLE);
                    MotorControl_SetMotorDutyCycle(_Right_Motor, _Forward_Dir, ROT_DUTY_CYCLE);
                    DriveTrain_StopAfterDelay(ROT_45_TIME);
                    CurrentState = DriveBusyState;                   
                } break;
                
                case DRIVE_FORWARD_HALF:
                {
                    MotorControl_DriveStraight(_Forward_Dir, LOW_SPEED, 0);
                    CurrentState = DriveReadyState;
                } break; 
                case DRIVE_FORWARD_FULL:
                {
                    MotorControl_DriveStraight(_Forward_Dir, HIGH_SPEED, 0);
                    CurrentState = DriveReadyState;
                } break;
                
                case DRIVE_BACKWARD_HALF:
                {
                    MotorControl_DriveStraight(_Backward_Dir, LOW_SPEED, 0);
                    CurrentState = DriveReadyState;
                } break;
                
                case DRIVE_BACKWARD_FULL:
                {
                    MotorControl_DriveStraight(_Backward_Dir, HIGH_SPEED, 0);
                    CurrentState = DriveReadyState;
                } break;
                
                case DRIVE_ROTATE_CWINF:
                {
                    MotorControl_DriveTurn(_Clockwise_Turn, LOW_SPEED, 0);
                    CurrentState = DriveReadyState;
                } break;
                
                case DRIVE_ROTATE_CCWINF:
                {
                    MotorControl_DriveTurn(_CounterClockwise_Turn, LOW_SPEED, 0);
                    CurrentState = DriveReadyState;
                } break;
                default:
                {
                } break;
            }
        } break;
        case DriveBusyState:
        {
            if (DRIVE_STOP_MOTORS == ThisEvent.EventType)
            {
                MotorControl_StopMotors();
                CurrentState = DriveReadyState;
            }
            else if (ES_TIMEOUT == ThisEvent.EventType)
            {
                MotorControl_StopMotors();
                CurrentState = DriveReadyState;
                // Post DRIVE_COMMAND_COMPLETE to distrolist
            }
            else
            {
                // add event to deferral queue
                printf("DriveTrain: Currently busy. Unable to accept new command \r\n");
            }
        } break;
        default:
          ;
    }                                   // end switch on Current State
    
    return ReturnEvent;
}


/****************************************************************************
 Function
     QueryDriveTrain

 Parameters
     None

 Returns
     DriveTrainState_t The current state of the DriveTrain state machine

 Description
     returns the current state of the Drive Train state machine
 Notes

 Author
 Andrew Sack
****************************************************************************/
DriveTrainState_t QueryDriveTrain(void)
{
    return CurrentState;
}

/****************************************************************************
 * Function
 *      DriveTrain_StopAfterDelay   
 *      
 * Parameters
 *      uint16_t DelayMS - Time in ms to delay for
 * Return
 *      void
 * Description
 *      Sets timer for correct amount of time which generates timout to stop
 *		motors after it expires
****************************************************************************/
void DriveTrain_StopAfterDelay(uint16_t DelayMS)
{
    // Set Timer for DelayMS time
    ES_Timer_InitTimer(DRIVETRAIN_TIMER, DelayMS);
	// Update Current state    
    CurrentState = DriveBusyState;
}
	
/***************************************************************************
 private functions
 ***************************************************************************/
