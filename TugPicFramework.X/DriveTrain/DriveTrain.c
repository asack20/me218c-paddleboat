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

#define SWEEP_CW_ANGLE 360 // for CW only // was 30 with ccw rotate
#define SWEEP_OVERROTATE_ANGLE 3 // degrees

/*----------------------------- Module Types ------------------------------*/
/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/
/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match that of enum in header file
static DriveTrainState_t CurrentState;

// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;

// Speed (RPM*10)corresponding to Stopped, Low, Medium, High respectively
const uint16_t Speeds[] = {0, 200, 300, 700}; //medium speed was 300

static int16_t SweepAmount;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitDriveTrain

 Parameters
     uint8_t : the priority of this service

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
    
    SweepAmount = 0;
    
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
#ifdef DRIVE_DEBUG
        printf("DriveDebug: Stopping motors \r\n");
#endif        
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
                    // Drive Backwards at Medium speed
                    MotorControl_DriveStraight(_Backward_Dir, Speeds[Medium], 0);
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
                    // Turn Clockwise by SWEEP_CW_ANGLE at slow speed
                    MotorControl_DriveTurn(_Clockwise_Turn, Speeds[Low], SWEEP_CW_ANGLE);
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
                CurrentState = DriveStoppedState;
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
                CurrentState = DriveStoppedState;
                
#ifdef DRIVE_DEBUG
                printf("DriveDebug: DriveUntilBump BUMP_FOUND\r\n");
#endif
            }
        } break;
        
        // Tape Detect
        case (DriveUntilFirstTapeDetectState):
        {
            CurrentState = DriveStoppedState;
#ifdef DRIVE_DEBUG
            printf("DriveDebug: DriveUntilFirstTapeDetectState NOT IMPLEMENTED. Returning to DriveStoppedState\r\n");
#endif              
        } break;
        
        case (DriveTapeSquareUpState):
        {
            CurrentState = DriveStoppedState;
#ifdef DRIVE_DEBUG
            printf("DriveDebug: DriveTapeSquareUpState NOT IMPLEMENTED. Returning to DriveStoppedState\r\n");
#endif  
        } break;
        
        case (DriveClockwiseSweepState):
        {
            if (BEACON_FOUND == ThisEvent.EventType)
            {     
                // Store Current ticks on Left Encoder
                Encoder_t LeftEncoder = MotorControl_GetEncoder(_Left_Motor);
                SweepAmount = LeftEncoder.TickCount;
                
                // Start overrotate
                MotorControl_DriveTurn(_Clockwise_Turn, Speeds[Low], SWEEP_OVERROTATE_ANGLE);

                CurrentState = DriveOverRotateState;
                
#ifdef DRIVE_DEBUG
                printf("DriveDebug: Beacon Found. Time to shoot\r\n");
#endif                
            }
            // Beacon not found. Stop moving
            else if (DRIVE_GOAL_REACHED == ThisEvent.EventType)
            {
                MotorControl_StopMotors();
                
                // Post to SPI
                PostEvent.EventType = DRIVE_GOAL_REACHED;
                PostSPIList(PostEvent);
                
                CurrentState = DriveStoppedState;
#ifdef DRIVE_DEBUG
                printf("DriveDebug: Beacon not Found. Starting Drive CounterClockwise Sweep\r\n");
#endif
            }
        } break;
        
        case (DriveOverRotateState):
        {
            if (DRIVE_GOAL_REACHED == ThisEvent.EventType) // overrotate complete
            {
                MotorControl_StopMotors();
                
                PostEvent.EventType = BEACON_ACKNOWLEDGED;
                PostSPIList(PostEvent);
                
                CurrentState = DriveBeaconWaitState;
#ifdef DRIVE_DEBUG
                printf("DriveDebug: Over Rotate complete\r\n");
#endif
            }
        } break;
        
        case (DriveCounterClockwiseSweepState):
        {
            if (BEACON_FOUND == ThisEvent.EventType)
            {
                MotorControl_StopMotors();
                
                // Store Current ticks on Left Encoder
                Encoder_t LeftEncoder = MotorControl_GetEncoder(_Left_Motor);
                // Subtract counterclockwise from clockwise amount
                SweepAmount -= LeftEncoder.TickCount;
                
                PostEvent.EventType = BEACON_ACKNOWLEDGED;
                PostSPIList(PostEvent);
                
                CurrentState = DriveBeaconWaitState;
#ifdef DRIVE_DEBUG
                printf("DriveDebug: Beacon Found. Time to shoot\r\n");
#endif                  
            }
            // Beacon not found.
            else if (DRIVE_GOAL_REACHED == ThisEvent.EventType)
            {
                MotorControl_StopMotors();
                
                // Post to SPI
                PostEvent.EventType = DRIVE_GOAL_REACHED;
                PostSPIList(PostEvent);
                
                CurrentState = DriveStoppedState;
                
#ifdef DRIVE_DEBUG
                printf("DriveDebug: Beacon not Found again. Abort cycle\r\n");
#endif                 
            }
        } break;
        
        case (DriveBeaconWaitState):
        {
            if (DRIVE_UNDO_ROTATE == ThisEvent.EventType)
            {
                //Overall rotation was clockwise
                if (SweepAmount >= 0)
                {                    
                    // Need to convert Ticks back to angle 
                    uint16_t Angle = (uint16_t)((float) SweepAmount / TICKS_PER_DEGREE) + SWEEP_OVERROTATE_ANGLE;
#ifdef DRIVE_DEBUG
                    printf("DriveDebug: Undoing Sweep by %d degrees CCW\r\n", Angle);
#endif 
                    // Undo rotation in counterclockwise direction
                    MotorControl_DriveTurn(_CounterClockwise_Turn, Speeds[Medium], Angle);
                }
                else // rotation was CCW
                {
                    // Need to convert Ticks back to angle (SweepAmount is negative)
                    uint16_t Angle = (uint16_t)((float) -1* SweepAmount / TICKS_PER_DEGREE);
#ifdef DRIVE_DEBUG
                    printf("DriveDebug: Undoing Sweep by %d degrees CW\r\n", Angle);
#endif 
                    // Undo rotation in clockwise direction
                    MotorControl_DriveTurn(_Clockwise_Turn, Speeds[Medium], Angle);
                }
                CurrentState = DriveUndoRotateState;
#ifdef DRIVE_DEBUG
                printf("DriveDebug: Shooting complete. Returning to straight orientation\r\n");
#endif                 
            }
        } break;
        
        case (DriveUndoRotateState):
        {
            if (DRIVE_GOAL_REACHED == ThisEvent.EventType)
            {
                MotorControl_StopMotors();
                
                PostEvent.EventType = DRIVE_GOAL_REACHED;
                PostSPIList(PostEvent);
                
                CurrentState = DriveStoppedState;
#ifdef DRIVE_DEBUG
                printf("DriveDebug: Back to original orientation. Ready to return to reload zone\r\n");
#endif                 
            }
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
