/****************************************************************************
 Module
   KeyboardService.c

 Revision
   1.0.1

 Description
 Service to read keypresses from connected terminal and trigger events

 Notes


****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "KeyboardService.h"
#include "../DriveTrain/DriveTrain.h"
#include "../DriveTrain/MotorControlDriver.h"
#include "../FrameworkHeaders/ES_Timers.h"


/*----------------------------- Module Defines ----------------------------*/
#define MAX_SPEED 1500
#define SPEED_STEP 100
#define TICKS_PER_ROT 300
#define SMALL_TICK_TARGET 50
#define BIG_TICK_TARGET 5*TICKS_PER_ROT
#define LOG_TIME 10 // ms

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/
void PrintInstructions(void);
void PrintMotorDetails(void);
void LogLeftMotor(void);
void LogRightMotor(void);
/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;

static MotorControl_Direction_t LeftDirection;
static MotorControl_Direction_t RightDirection;
static uint16_t LeftSpeed;
static uint16_t RightSpeed;

static bool LeftLog;
static bool RightLog;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitKeyboardService

 Parameters
     uint8_t : the priority of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, and does any
     other required initialization for this service
 Notes

 Author
 Andrew Sack
****************************************************************************/
bool InitKeyboardService(uint8_t Priority)
{
    printf("\n\n---------------------------------------------------------\n\r");
    printf("Initializing KeyboardService\r");
    PrintInstructions(); // print meaning of each key
    MyPriority = Priority;
    
    LeftDirection = _Forward_Dir;
    RightDirection = _Forward_Dir;
    LeftSpeed = 0;
    RightSpeed = 0; 
    
    LeftLog = false;
    RightLog = false;

    return true;
}

/****************************************************************************
 Function
     PostKeyboardService

 Parameters
     EF_Event_t ThisEvent ,the event to post to the queue

 Returns
     bool false if the Enqueue operation failed, true otherwise

 Description
     Posts an event to this state machine's queue
 Notes
****************************************************************************/
bool PostKeyboardService(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunKeyboardService

 Parameters
   ES_Event_t : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
 Check Which Keypress is received and Post corresponding event
 Notes

****************************************************************************/
ES_Event_t RunKeyboardService(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
    ES_Event_t PostEvent;
  
    switch (ThisEvent.EventType)
    {
        case ES_TIMEOUT:
        {
            if (LeftLog)
            {
                LogLeftMotor();
            }
            if (RightLog)
            {
                LogRightMotor();
            }
            ES_Timer_InitTimer(KEYBOARD_TIMER, LOG_TIME);
        } break;
        case ES_NEW_KEY:
        {
            switch (ThisEvent.EventParam)
            {
                case '?':
                {
                    PrintInstructions();
                } break;
                case 'q':
                {
                    printf("KeyboardService: posting DRIVE_STOP_MOTORS (CG: 0x00) to DriveTrain\n\r");
                    PostEvent.EventType = DRIVE_STOP_MOTORS;
                    PostDriveTrain(PostEvent);
                } break;
                case 'w':
                {
                    printf("KeyboardService: posting DRIVE_ROTATE_CW90 (CG: 0x02) to DriveTrain\n\r");
                    PostEvent.EventType = DRIVE_ROTATE_CW90;
                    PostDriveTrain(PostEvent);
                } break;
                case 'e':
                {
                    printf("KeyboardService: posting DRIVE_ROTATE_CW45 (CG: 0x03) to DriveTrain\n\r");
                    PostEvent.EventType = DRIVE_ROTATE_CW45;
                    PostDriveTrain(PostEvent);
                } break;
                case 'r':
                {
                    printf("KeyboardService: posting DRIVE_ROTATE_CCW90 (CG: 0x04) to DriveTrain\n\r");
                    PostEvent.EventType = DRIVE_ROTATE_CCW90;
                    PostDriveTrain(PostEvent);
                } break;
                case 't':
                {
                    printf("KeyboardService: posting DRIVE_ROTATE_CCW45 (CG: 0x05) to DriveTrain\n\r");
                    PostEvent.EventType = DRIVE_ROTATE_CCW45;
                    PostDriveTrain(PostEvent);
                } break;
                case 'y':
                {
                    printf("KeyboardService: posting DRIVE_FORWARD_HALF (CG: 0x08) to DriveTrain\n\r");
                    PostEvent.EventType = DRIVE_FORWARD_HALF;
                    PostDriveTrain(PostEvent);
                } break;
                case 'u':
                {
                    printf("KeyboardService: posting DRIVE_FORWARD_FULL (CG: 0x09) to DriveTrain\n\r");
                    PostEvent.EventType = DRIVE_FORWARD_FULL;
                    PostDriveTrain(PostEvent);
                } break;
                case 'i':
                {
                    printf("KeyboardService: posting DRIVE_BACKWARD_HALF (CG: 0x10) to DriveTrain\n\r");
                    PostEvent.EventType = DRIVE_BACKWARD_HALF;
                    PostDriveTrain(PostEvent);
                } break;
                case 'o':
                {
                    printf("KeyboardService: posting DRIVE_BACKWARD_FULL (CG: 0x11) to DriveTrain\n\r");
                    PostEvent.EventType = DRIVE_BACKWARD_FULL ;
                    PostDriveTrain(PostEvent);
                } break;
                case 'p':
                {
                    printf("KeyboardService: posting DRIVE_ROTATE_CWINF to DriveTrain\n\r");
                    PostEvent.EventType = DRIVE_ROTATE_CWINF ;
                    PostDriveTrain(PostEvent);
                } break;
                case '[':
                {
                    printf("KeyboardService: posting DRIVE_ROTATE_CCWINF to DriveTrain\n\r");
                    PostEvent.EventType = DRIVE_ROTATE_CCWINF ;
                    PostDriveTrain(PostEvent);
                } break;
                case 'a':
                {
                    if(!LeftLog && !RightLog)
                        printf("KeyboardService: Disabling Control Law\n\r");
                    MotorControl_DisableClosedLoop();
                } break;
                case 's':
                {
                    if(!LeftLog && !RightLog)
                        printf("KeyboardService: Enabling Control Law\n\r");
                    MotorControl_EnableClosedLoop();
                } break;
                
                case 'd':
                {
                    if(!LeftLog && !RightLog)
                        printf("KeyboardService: Setting Left motor direction to FORWARD\n\r");
                    LeftDirection = _Forward_Dir;
                    MotorControl_SetMotorSpeed(_Left_Motor, LeftDirection, LeftSpeed);  
                } break;
                case 'f':
                {
                    if(!LeftLog && !RightLog)
                        printf("KeyboardService: Setting Left motor direction to BACKWARD\n\r");
                    LeftDirection = _Backward_Dir;
                    MotorControl_SetMotorSpeed(_Left_Motor, LeftDirection, LeftSpeed);  
                } break;
                case 'g':
                {
                    LeftSpeed = 0;
                    if(!LeftLog && !RightLog)
                        printf("KeyboardService: Setting Left motor speed to %d RPM\n\r", LeftSpeed/10);
                    MotorControl_SetMotorSpeed(_Left_Motor, LeftDirection, LeftSpeed);  
                } break;
                case 'h':
                {
                    LeftSpeed -= SPEED_STEP;
                    if(!LeftLog && !RightLog)
                        printf("KeyboardService: Setting Left motor speed to %d RPM\n\r", LeftSpeed/10);
                    MotorControl_SetMotorSpeed(_Left_Motor, LeftDirection, LeftSpeed);  
                } break;
                case 'j':
                {
                    LeftSpeed += SPEED_STEP;
                    if(!LeftLog && !RightLog)
                        printf("KeyboardService: Setting Left motor speed to %d RPM\n\r", LeftSpeed/10);
                    MotorControl_SetMotorSpeed(_Left_Motor, LeftDirection, LeftSpeed);  
                } break;
                case 'k':
                {
                    LeftSpeed = MAX_SPEED;
                    if(!LeftLog && !RightLog)
                        printf("KeyboardService: Setting Left motor speed to %d RPM\n\r", LeftSpeed/10);
                    MotorControl_SetMotorSpeed(_Left_Motor, LeftDirection, LeftSpeed);  
                } break;
                
                case 'x':
                {
                    printf("KeyboardService: Setting Right motor direction to FORWARD\n\r");
                    if(!LeftLog && !RightLog)
                        RightDirection = _Forward_Dir;
                    MotorControl_SetMotorSpeed(_Right_Motor, RightDirection, RightSpeed);  
                } break;
                case 'c':
                {
                    printf("KeyboardService: Setting Right motor direction to BACKWARD\n\r");
                    if(!LeftLog && !RightLog)
                        RightDirection = _Backward_Dir;
                    MotorControl_SetMotorSpeed(_Right_Motor, RightDirection, RightSpeed);  
                } break;
                
                case 'v':
                {
                    RightSpeed = 0;
                    if(!LeftLog && !RightLog)
                        printf("KeyboardService: Setting Right motor speed to %d RPM\n\r", RightSpeed/10);
                    MotorControl_SetMotorSpeed(_Right_Motor, RightDirection, RightSpeed);  
                } break;
                case 'b':
                {
                    RightSpeed -= SPEED_STEP;
                    if(!LeftLog && !RightLog)
                        printf("KeyboardService: Setting Right motor speed to %d RPM\n\r", RightSpeed/10);
                    MotorControl_SetMotorSpeed(_Right_Motor, RightDirection, RightSpeed);  
                } break;
                case 'n':
                {
                    RightSpeed += SPEED_STEP;
                    if(!LeftLog && !RightLog)
                        printf("KeyboardService: Setting Right motor speed to %d RPM\n\r", RightSpeed/10);
                    MotorControl_SetMotorSpeed(_Right_Motor, RightDirection, RightSpeed);  
                } break;
                case 'm':
                {
                    RightSpeed = MAX_SPEED;
                    if(!LeftLog && !RightLog)
                        printf("KeyboardService: Setting Right motor speed to %d RPM\n\r", RightSpeed/10);
                    MotorControl_SetMotorSpeed(_Right_Motor, RightDirection, RightSpeed);  
                } break;
                
                case '1':
                {
                    if(!LeftLog && !RightLog)
                        printf("KeyboardService: Moving Left Motor %d ticks\n\r", SMALL_TICK_TARGET);
                    MotorControl_ResetTickCount(_Left_Motor);
                    MotorControl_SetTickGoal(_Left_Motor, SMALL_TICK_TARGET);
                } break;
                case '2':
                {
                    if(!LeftLog && !RightLog)
                        printf("KeyboardService: Moving Left Motor %d ticks\n\r", BIG_TICK_TARGET);
                    MotorControl_ResetTickCount(_Left_Motor);
                    MotorControl_SetTickGoal(_Left_Motor, BIG_TICK_TARGET);
                } break;
                
                case '4':
                {
                    if(!LeftLog && !RightLog)
                        printf("KeyboardService: Moving Right Motor %d ticks\n\r", SMALL_TICK_TARGET);
                    MotorControl_ResetTickCount(_Right_Motor);
                    MotorControl_SetTickGoal(_Right_Motor, SMALL_TICK_TARGET);
                } break;
                case '5':
                {
                    if(!LeftLog && !RightLog)
                        printf("KeyboardService: Moving Right Motor %d ticks\n\r", BIG_TICK_TARGET);
                    MotorControl_ResetTickCount(_Right_Motor);
                    MotorControl_SetTickGoal(_Right_Motor, BIG_TICK_TARGET);
                } break;
                
                case '7':
                {
                    printf("KeyboardService: Starting Left Motor Logging\n\r");
                    printf("CurrentRPM, Dir, TickCount, TargetRPM, ActualTargetRPM, TargetDir, TargetTick, ReqDutyCycle, Integral, RPMError, LastError, SumError\r\n");
                    LeftLog = true;
                    ES_Timer_InitTimer(KEYBOARD_TIMER, LOG_TIME);
                } break;
                case '8':
                {
                    printf("KeyboardService: Starting Right Motor Logging\n\r");
                    printf("CurrentRPM, Dir, TickCount, TargetRPM, ActualTargetRPM, TargetDir, TargetTick, ReqDutyCycle, Integral, RPMError, LastError, SumError\r\n");
                    RightLog = true;
                    ES_Timer_InitTimer(KEYBOARD_TIMER, LOG_TIME);
                } break;
                case '9':
                {
                    printf("KeyboardService: Stopping Motor Logging\n\r");
                    ES_Timer_StopTimer(KEYBOARD_TIMER);
                    LeftLog = false;
                    RightLog = false;
                } break;
                
                case '.':
                {
                    printf("KeyboardService: Current Motor Details\n\r");
                    PrintMotorDetails();
                } break;       
                default:
                {
                    printf("KeyboardService: No Event bound to %c. Press '?' to see list of valid keys.\r\n", (char) ThisEvent.EventParam);
                } break;
            }
        }
        break;       
        default:
        ;
    }
    return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/
/****************************************************************************
 * Function
 *      PrintInstructions
 *      
 * Parameters
 *      void
 * Return
 *      void
 * Description
 *      Prints all the instructions of what each keypress does
****************************************************************************/
void PrintInstructions(void)
{
    printf( "\n\n---------------------------------------------------------\r\n");
    printf( "Press '?' to print Key Press meanings again\n\r");
    printf( "Press 'q' to post DRIVE_STOP_MOTORS (CG: 0x00)\n\r");
    printf( "Press 'w' to post DRIVE_ROTATE_CW90 (CG: 0x02)\n\r");
    printf( "Press 'e' to post DRIVE_ROTATE_CW45 (CG: 0x03)\n\r");
    printf( "Press 'r' to post DRIVE_ROTATE_CCW90 (CG: 0x04)\n\r");
    printf( "Press 't' to post DRIVE_ROTATE_CCW45 (CG: 0x05)\n\r");
    printf( "Press 'y' to post DRIVE_FORWARD_HALF (CG: 0x08)\n\r");
    printf( "Press 'u' to post DRIVE_FORWARD_FULL (CG: 0x09)\n\r");
    printf( "Press 'i' to post DRIVE_BACKWARD_HALF (CG: 0x10)\n\r");
    printf( "Press 'o' to post DRIVE_BACKWARD_FULL (CG: 0x11)\n\r");
    printf( "Press 'p' to post DRIVE_ROTATE_CWINF \n\r");
    printf( "Press '[' to post DRIVE_ROTATE_CCWINF \n\r\n");
    
    printf( "\n\n------------Closed Loop--------------\r\n");
    printf( "Press 'a' to DISABLE Closed Loop Control \n\r");
    printf( "Press 's' to ENABLE Closed Loop Control \n\r\n");
    
    printf( "Press 'd' to set LEFT motor direction to FORWARD \n\r");
    printf( "Press 'f' to set LEFT motor direction to BACKWARD \n\r\n");
    
    printf( "Press 'g' to set LEFT motor speed to 0 RPM \n\r");
    printf( "Press 'h' to decrease LEFT motor speed by 10 RPM \n\r");
    printf( "Press 'j' to increase LEFT motor speed by 10 RPM \n\r");
    printf( "Press 'k' to set LEFT motor speed to MAX RPM \n\r\n");
    
    printf( "Press '1' to move LEFT motor SMAll DISTANCE\n\r");
    printf( "Press '2' to move LEFT motor BIG DISTANCE\n\r\n");
    
    printf( "Press 'x' to set RIGHT motor direction to FORWARD \n\r");
    printf( "Press 'c' to set RIGHT motor direction to BACKWARD \n\r\n");
    
    printf( "Press 'v' to set RIGHT motor speed to 0 RPM \n\r");
    printf( "Press 'b' to decrease RIGHT motor speed by 10 RPM \n\r");
    printf( "Press 'n' to increase RIGHT motor speed by 10 RPM \n\r");
    printf( "Press 'm' to set RIGHT motor speed to MAX RPM \n\r\n");
    
    printf( "Press '4' to move RIGHT motor SMAll DISTANCE\n\r");
    printf( "Press '5' to move RIGHT motor BIG DISTANCE\n\r\n");
    
    printf( "Press '7' to start LEFT motor logging\r\n");
    printf( "Press '8' to start RIGHT motor logging\r\n");
    printf( "Press '9' to stop motor logging\r\n\n");
      
            
    printf( "Press '.' to Print Current Motor Details\n\r");
    printf( "---------------------------------------------------------\r\n\n");
}

/****************************************************************************
 * Function
 *      PrintMotorDetails
 *      
 * Parameters
 *      void
 * Return
 *      void
 * Description
 *      Prints all the struct values for motors
****************************************************************************/
void PrintMotorDetails(void)
{
    Encoder_t LeftEncoder = MotorControl_GetEncoder(_Left_Motor);
    Encoder_t RightEncoder = MotorControl_GetEncoder(_Right_Motor);
    
    ControlState_t LeftControl = MotorControl_GetControlState(_Left_Motor);
    ControlState_t RightControl = MotorControl_GetControlState(_Right_Motor);
    
    printf("Left Encoder:\tCurrentRPM: %0.2f\tDir: %u\tTickCount: %u\r\n", 
            LeftEncoder.CurrentRPM, LeftEncoder.Direction, LeftEncoder.TickCount);
    printf("Left Control:\tTargetRPM: %0.2f\tActualTargetRPM: %0.2f\tDir: %u\tTargetTick: %u\tDutyCycle: %0.2f\r\n",
            LeftControl.TargetRPM, LeftControl.ActualTargetRPM, LeftControl.TargetDirection, LeftControl.TargetTickCount,LeftControl.RequestedDutyCycle);
    printf("Left Error:\tIntegralTerm:%0.2f\tRPMError: %0.2f\tLastError: %0.2f\tSumError: %0.2f\r\n\n",
            LeftControl.IntegralTerm, LeftControl.RPMError, LeftControl.LastError, LeftControl.SumError);
    
    printf("Right Encoder:\tCurrentRPM: %0.2f\tDir: %u\tTickCount: %u\r\n", 
            RightEncoder.CurrentRPM, RightEncoder.Direction, RightEncoder.TickCount);
    printf("Right Control:\tTargetRPM: %0.2f\tActualTargetRPM: %0.2f\tDir: %u\tTargetTick: %u\tDutyCycle: %0.2f\r\n",
            RightControl.TargetRPM, RightControl.ActualTargetRPM, RightControl.TargetDirection, RightControl.TargetTickCount,RightControl.RequestedDutyCycle);
    printf("Right Error:\tIntegralTerm:%0.2f\tRPMError: %0.2f\tLastError: %0.2f\tSumError: %0.2f\r\n\n",
            RightControl.IntegralTerm, RightControl.RPMError, RightControl.LastError, RightControl.SumError);
}
/****************************************************************************
 * Function
 *      LogLeftMotor
 *      
 * Parameters
 *      void
 * Return
 *      void
 * Description
 *      Prints struct values for Left Motor in comma separated form
****************************************************************************/
void LogLeftMotor(void)
{
    Encoder_t LeftEncoder = MotorControl_GetEncoder(_Left_Motor);
    ControlState_t LeftControl = MotorControl_GetControlState(_Left_Motor);
    
    printf("%0.2f, %u, %u, %0.2f, %0.2f, %u, %u, %0.2f, %0.2f, %0.2f, %0.2f, %0.2f\r\n", LeftEncoder.CurrentRPM, 
            LeftEncoder.Direction, LeftEncoder.TickCount, LeftControl.TargetRPM, 
            LeftControl.ActualTargetRPM, LeftControl.TargetDirection, LeftControl.TargetTickCount,
            LeftControl.RequestedDutyCycle,LeftControl.IntegralTerm, LeftControl.RPMError, LeftControl.LastError, LeftControl.SumError);
}
/****************************************************************************
 * Function
 *      LogRightMotor
 *      
 * Parameters
 *      void
 * Return
 *      void
 * Description
 *      Prints struct values for Right Motor in comma separated form
****************************************************************************/
void LogRightMotor(void)
{
    Encoder_t RightEncoder = MotorControl_GetEncoder(_Right_Motor);
    ControlState_t RightControl = MotorControl_GetControlState(_Right_Motor);
    
    printf("%0.2f, %u, %u, %0.2f, %0.2f, %u, %u, %0.2f, %0.2f, %0.2f, %0.2f, %0.2f\r\n", RightEncoder.CurrentRPM, 
            RightEncoder.Direction, RightEncoder.TickCount, RightControl.TargetRPM, 
            RightControl.ActualTargetRPM, RightControl.TargetDirection, RightControl.TargetTickCount,
            RightControl.RequestedDutyCycle,RightControl.IntegralTerm, RightControl.RPMError, RightControl.LastError, RightControl.SumError);
}
/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

