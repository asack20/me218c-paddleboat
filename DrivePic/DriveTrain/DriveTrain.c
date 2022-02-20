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
#include "../SPI/SPIFollowerSM.h"
#include <xc.h>
#include <sys/attribs.h>

/*----------------------------- Module Defines ----------------------------*/
// Command Tuning
#define DRIVE_DEBUG

#define SWEEP_CW_ANGLE 10

/*----------------------------- Module Types ------------------------------*/
/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/
/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file
static DriveTrainState_t CurrentState;

// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;

// Speed (RPM*10)corresponding to Stopped, Low, Medium, High respectively
const uint16_t Speeds[] = {0, 100, 300, 500};

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
    
    // Drive Stop has same behavior in every state
    if (DRIVE_STOP == ThisEvent.EventType)
    {
        MotorControl_StopMotors();
        CurrentState = DriveStoppedState;
        
        PostEvent.EventType = STOP_ACKNOWLEDGED;
        PostSPIList(PostEvent);
    }
    
    else {
    switch (CurrentState)
    {
        case (DriveInitState):
        {
            if (ES_INIT == ThisEvent.EventType)
            {
                MotorControl_StopMotors();
                CurrentState = DriveStoppedState;
            }
        } break;
        
        // Main state where new motions start
        case (DriveStoppedState):
        {
            switch (ThisEvent.EventType)
            {
                case DRIVE_DISTANCE:
                {
                    // Parse SPI Event struct
                    SPI_MOSI_Command_t SPICommand = (SPI_MOSI_Command_t) ThisEvent.EventParam;
                    if (SPICommand.DriveType == Translation)
                    {
                        // Call Motor Control function with correct params
                        MotorControl_DriveStraight(SPICommand.Direction, Speeds[SPICommand.Speed], SPICommand.Data);
#ifdef DRIVE_DEBUG
                        printf("DriveDebug: Starting Drive Straight Distance \r\n");
#endif
                    }
                    else if (SPICommand.DriveType == Rotation)
                    {
                        // Call Motor Control function with correct params
                        MotorControl_DriveTurn(SPICommand.Direction, Speeds[SPICommand.Speed], SPICommand.Data);
#ifdef DRIVE_DEBUG
                        printf("DriveDebug: Starting Drive Turn Distance \r\n");
#endif
                    }
                    else
                    {
                        printf("Error: Invalid Drive Type: %u", SPICommand.DriveType);
                    }
                    
                    CurrentState = DriveDistanceState;
                } break;
                
                case DRIVE_UNTIL_BUMP:
                {
                    // Drive Backwards at slow speed
                    MotorControl_DriveStraight(_Backward_Dir, Speeds[Low], 0);
#ifdef DRIVE_DEBUG
                    printf("DriveDebug: Starting Drive Until Bump\r\n");
#endif
                    CurrentState = DriveUntilBumpState;
                } break; 
                
                case DRIVE_TAPE_ALIGN:
                {
                    // Drive Forwards at slow speed
                    MotorControl_DriveStraight(_Forward_Dir, Speeds[Low], 0);
#ifdef DRIVE_DEBUG
                    printf("DriveDebug: Starting Drive Tape Align\r\n");
#endif
                    CurrentState = DriveUntilFirstTapeDetectState;
                } break;
                
                case DRIVE_BEACON_SWEEP:
                {
                    // Turn CLockwise by SWEEP_CW_ANGLE at slow speed
                    MotorControl_DriveStraight(_Clockwise_Turn, Speeds[Low], SWEEP_CW_ANGLE);
#ifdef DRIVE_DEBUG
                    printf("DriveDebug: Starting Drive Clockwise Sweep\r\n");
#endif
                    CurrentState = DriveClockwiseSweepState;
                } break;
                
            }
        } break;
        
        case (DriveDistanceState):
        {
            if (DRIVE_GOAL_REACHED == ThisEvent.EventType)
            {
                // stop motors
                MotorControl_StopMotors();
                // Post DRIVE_GOAL_REACHED to SPI
                PostEvent.EventType = DRIVE_GOAL_REACHED;
                PostSPIList(PostEvent);
                
#ifdef DRIVE_DEBUG
                printf("DriveDebug: DriveDistanceState DRIVE_GOAL_REACHED\r\n");
#endif
            }
        } break;
        
        case (DriveUntilBumpState):
        {
            if (BUMP_FOUND == ThisEvent.EventType)
            {
                // stop motors
                MotorControl_StopMotors();
                
#ifdef DRIVE_DEBUG
                printf("DriveDebug: DriveUntilBump BUMP_FOUND\r\n");
#endif
            }
        } break;
        
        // Tape Detect
        case (DriveUntilFirstTapeDetectState):
        {
            
        } break;
        
        case (DriveTapeSquareUpState):
        {
            
        } break;
        
        case (DriveClockwiseSweepState):
        {
            
        } break;
        
        case (DriveCounterClockwiseSweepState):
        {
            
        } break;
        
        case (DriveBeaconWaitState):
        {
            
        } break;
        
        case (DriveUndoRotateState):
        {
            
        } break;
        
        default:
          ;
    }                                   // end switch on Current State
    }
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

/***************************************************************************
 private functions
 ***************************************************************************/
