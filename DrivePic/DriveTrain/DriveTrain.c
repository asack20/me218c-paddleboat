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
#include "../HALs/PIC32PortHAL.h"
#include <xc.h>
#include <sys/attribs.h>

/*----------------------------- Module Defines ----------------------------*/
// PWM configuration
#define PWM_TIMER 3
#define PWM_PERIOD 1999 // Base frequency of 10kHz with prescale of 4
#define DUTY_CYCLE_TO_OCRS 20// multiplier

// Command Tuning
#define FULL_DUTY_CYCLE 100 // Duty cycle for 100% speed
#define HALF_DUTY_CYCLE 75 // Duty cycle for 50% speed
#define ROT_DUTY_CYCLE 100 // Duty cycle used for rotating
#define ROT_90_TIME 1500 // time in ms required to rotate 90 degrees
#define ROT_45_TIME 750 // time in ms required to rotate 45 degrees

// Left motor ports and pins
#define L_DIRB_PORT _Port_A
#define L_DIRB_PIN _Pin_3
#define L_ENABLE_PORT _Port_A
#define L_ENABLE_PIN _Pin_2
#define L_DIRB_LAT LATAbits.LATA3 // Pin 10
#define L_ENABLE_LAT LATAbits.LATA2 // Pin 9
#define L_DIRA_REG RPB4R // Pin 11
#define OC1_PERIPHERAL_CODE 0b0101
#define L_OCRS OC1RS

// Right motor ports and pins
#define R_DIRB_PORT _Port_B
#define R_DIRB_PIN _Pin_9
#define R_ENABLE_PORT _Port_B
#define R_ENABLE_PIN _Pin_10
#define R_DIRB_LAT LATBbits.LATB9 // Pin 18
#define R_ENABLE_LAT LATBbits.LATB10 // Pin 21
#define R_DIRA_REG RPB8R // Pin 17
#define OC2_PERIPHERAL_CODE 0b0101
#define R_OCRS OC2RS
/*----------------------------- Module Types ------------------------------*/
// typedefs for the states
// State definitions for use with the query function
typedef union {
    struct {
        uint16_t CapturedTime;
        uint16_t Rollover;   
    };
    uint32_t TotalTime;
} RolloverTimer_t;

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/
static void InitPWMTimer(void);
static void InitLeftMotor(void);
static void InitRightMotor(void);
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

    // Call sub-init functions
    InitPWMTimer();
    InitLeftMotor();
    InitRightMotor();
    
    // Enable SFRS
    OC1CONbits.ON = 1;
    OC2CONbits.ON = 1;
    T3CONbits.ON = 1;
    
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
                DriveTrain_StopMotors(); // make sure motors are off
                CurrentState = DriveReadyState;
            }
        } break;
        case DriveReadyState:
        {
            switch (ThisEvent.EventType)
            {
                case DRIVE_STOP_MOTORS:
                {
                    DriveTrain_StopMotors();
                    CurrentState = DriveReadyState;
                } break;
               
                case DRIVE_ROTATE_CW90:
                {
                    DriveTrain_SetMotorDutyCycle(_Left_Motor, _Forward_Dir, ROT_DUTY_CYCLE);
                    DriveTrain_SetMotorDutyCycle(_Right_Motor, _Backward_Dir, ROT_DUTY_CYCLE);
                    DriveTrain_StopAfterDelay(ROT_90_TIME);
                    CurrentState = DriveBusyState;
                } break;
                
                case DRIVE_ROTATE_CW45:
                {
                    DriveTrain_SetMotorDutyCycle(_Left_Motor, _Forward_Dir, ROT_DUTY_CYCLE);
                    DriveTrain_SetMotorDutyCycle(_Right_Motor, _Backward_Dir, ROT_DUTY_CYCLE);
                    DriveTrain_StopAfterDelay(ROT_45_TIME);
                    CurrentState = DriveBusyState;
                } break;

                case DRIVE_ROTATE_CCW90:
                {
                    DriveTrain_SetMotorDutyCycle(_Left_Motor, _Backward_Dir, ROT_DUTY_CYCLE);
                    DriveTrain_SetMotorDutyCycle(_Right_Motor, _Forward_Dir, ROT_DUTY_CYCLE);
                    DriveTrain_StopAfterDelay(ROT_90_TIME);
                    CurrentState = DriveBusyState;
                } break;
                
                case DRIVE_ROTATE_CCW45:
                {
                    DriveTrain_SetMotorDutyCycle(_Left_Motor, _Backward_Dir, ROT_DUTY_CYCLE);
                    DriveTrain_SetMotorDutyCycle(_Right_Motor, _Forward_Dir, ROT_DUTY_CYCLE);
                    DriveTrain_StopAfterDelay(ROT_45_TIME);
                    CurrentState = DriveBusyState;                   
                } break;
                
                case DRIVE_FORWARD_HALF:
                {
                    DriveTrain_SetMotorDutyCycle(_Left_Motor, _Forward_Dir, HALF_DUTY_CYCLE);
                    DriveTrain_SetMotorDutyCycle(_Right_Motor, _Forward_Dir, HALF_DUTY_CYCLE);
                    CurrentState = DriveReadyState;
                } break; 
                case DRIVE_FORWARD_FULL:
                {
                    DriveTrain_SetMotorDutyCycle(_Left_Motor, _Forward_Dir, FULL_DUTY_CYCLE);
                    DriveTrain_SetMotorDutyCycle(_Right_Motor, _Forward_Dir, FULL_DUTY_CYCLE);
                    CurrentState = DriveReadyState;
                } break;
                
                case DRIVE_BACKWARD_HALF:
                {
                    DriveTrain_SetMotorDutyCycle(_Left_Motor, _Backward_Dir, HALF_DUTY_CYCLE);
                    DriveTrain_SetMotorDutyCycle(_Right_Motor, _Backward_Dir, HALF_DUTY_CYCLE);
                    CurrentState = DriveReadyState;
                } break;
                
                case DRIVE_BACKWARD_FULL:
                {
                    DriveTrain_SetMotorDutyCycle(_Left_Motor, _Backward_Dir, FULL_DUTY_CYCLE);
                    DriveTrain_SetMotorDutyCycle(_Right_Motor, _Backward_Dir, FULL_DUTY_CYCLE);
                    CurrentState = DriveReadyState;
                } break;
                
                case DRIVE_ROTATE_CWINF:
                {
                    DriveTrain_SetMotorDutyCycle(_Left_Motor, _Forward_Dir, ROT_DUTY_CYCLE);
                    DriveTrain_SetMotorDutyCycle(_Right_Motor, _Backward_Dir, ROT_DUTY_CYCLE);
                    CurrentState = DriveReadyState;
                } break;
                
                case DRIVE_ROTATE_CCWINF:
                {
                    DriveTrain_SetMotorDutyCycle(_Left_Motor, _Backward_Dir, ROT_DUTY_CYCLE);
                    DriveTrain_SetMotorDutyCycle(_Right_Motor, _Forward_Dir, ROT_DUTY_CYCLE);
                    CurrentState = DriveReadyState;
                } break;
                default:
                {
                    printf("DriveTrain: Received unknown event #%u\r\n", ThisEvent.EventType);
                } break;
            }
        } break;
        case DriveBusyState:
        {
            if (DRIVE_STOP_MOTORS == ThisEvent.EventType)
            {
                DriveTrain_StopMotors();
                CurrentState = DriveReadyState;
            }
            else if (ES_TIMEOUT == ThisEvent.EventType)
            {
                DriveTrain_StopMotors();
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
 *      DriveTrain_SetMotorDutyCycle     
 *      
 * Parameters
 *      DriveTrain_Motor_t WhichMotor - Left or Right Motor
 *      DriveTrain_Direction_t WhichDirection - Direction to move motor in
 *          Forward is relative to robot base (CW for right motor, CCW for left)
 *      uint8_t DutyCycle - (valid range 0-100 inclusive) PWM duty cycle to set 
 * Return
 *      void
 * Description
 *      Set specified motor to move at set PWM duty cycle in specified direction
****************************************************************************/
void DriveTrain_SetMotorDutyCycle(DriveTrain_Motor_t WhichMotor, DriveTrain_Direction_t WhichDirection, uint8_t DutyCycle)
{
    if (_Left_Motor == WhichMotor)
    {  
        //Set WhichMotor's DIRB LAT to WhichDirection to set direction correctly
        L_DIRB_LAT = WhichDirection;
        // invert duty cycle for backwards
        if (_Backward_Dir == WhichDirection)
        {
            DutyCycle = 100 - DutyCycle;
        }
        //Set OCRS register for WhichMotor to DUTY_CYCLE_TO_OCRS * Duty Cycle
        L_OCRS = DUTY_CYCLE_TO_OCRS * DutyCycle;
        //Set WhichMotor's ENABLE LAT to 1 to make sure it's enabled
        L_ENABLE_LAT = 1;
    }
    else if (_Right_Motor == WhichMotor)
    {
        //Set WhichMotor's DIRB LAT to WhichDirection to set direction correctly
        R_DIRB_LAT = WhichDirection;
        // invert duty cycle for backwards
        if (_Backward_Dir == WhichDirection)
        {
            DutyCycle = 100 - DutyCycle;
        }
        //Set OCRS register for WhichMotor to DUTY_CYCLE_TO_OCRS * Duty Cycle
        R_OCRS = DUTY_CYCLE_TO_OCRS * DutyCycle;
        //Set WhichMotor's ENABLE LAT to 1 to make sure it's enabled
        R_ENABLE_LAT = 1;
    }
	//Motors are now moving
    MotorsActive = true;
}
	
/****************************************************************************
 * Function
 *      DriveTrain_StopMotors   
 *      
 * Parameters
 *      void
 * Return
 *      void
 * Description
 *      Stop both motors
****************************************************************************/
void DriveTrain_StopMotors(void)
{
    // Set both motors to duty cycle of 0 and disable hbridge for added safety
    //Call SetMotorDutyCycle with duty cycle of 0
    DriveTrain_SetMotorDutyCycle(_Left_Motor, _Forward_Dir, 0);
	DriveTrain_SetMotorDutyCycle(_Right_Motor, _Forward_Dir, 0);	
	L_ENABLE_LAT = 0;
    R_ENABLE_LAT = 0;

	//Update States
    CurrentState = DriveReadyState;
    MotorsActive = false;
	//Clear any active timer
    ES_Timer_StopTimer(DRIVETRAIN_TIMER);
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

/*
 * InitPWMTimer
 * Helper Function for InitDriveTrain
 * Handles configuration of Timer for PWM
 */
static void InitPWMTimer(void)
{
    ////Timer 3 configuration (PWM OC)
    //turn timer off 
    T3CONbits.ON = 0;
    //disable stop in idle 
    T3CONbits.SIDL = 0;
    //Set prescaler to 4
    T3CONbits.TCKPS = 0b010;
    //Set to 16-bit mode (Using T3 does this inherently)
    //Use synchronous internal clock 
    T3CONbits.TCS = 0; 
    T3CONbits.TGATE = 0;
    //Set period to required PWM period 
    PR3 = PWM_PERIOD;
    //Clear timer 
    TMR3 = 0;
}

/*
 * InitLeftMotor
 * Helper Function for InitDriveTrain
 * Handles configuration of ports and OC for Left Motor
 */
static void InitLeftMotor(void)
{
    L_ENABLE_LAT = 0;
    PortSetup_ConfigureDigitalOutputs(L_DIRB_PORT, L_DIRB_PIN);
    PortSetup_ConfigureDigitalOutputs(L_ENABLE_PORT, L_ENABLE_PIN);
    
    // Set DIRB Lo for forward and ENABLE lo to ensure motor isn't running
    L_DIRB_LAT = 0;
    L_ENABLE_LAT = 0;
    
    //Turn OC1 off 
    OC1CONbits.ON = 0;
    //Disable SIDLE 
    OC1CONbits.SIDL = 0;
    //Use 16 bit mode 
    OC1CONbits.OC32 = 0;
    //Use timer 3 
    OC1CONbits.OCTSEL = 1;
    //Set to PWM mode, fault disabled 
    OC1CONbits.OCM = 0b110;
    
    //Write initial duty cycle of 0
    OC1R = 0;
    OC1RS = 0;
    
    // Configure Pins
    L_DIRA_REG = OC1_PERIPHERAL_CODE;
}

/*
 * InitRightMotor
 * Helper Function for InitDriveTrain
 * Handles configuration of ports and OC for Right Motor
 */
static void InitRightMotor(void)
{
    R_ENABLE_LAT = 0;
    
    PortSetup_ConfigureDigitalOutputs(R_DIRB_PORT, R_DIRB_PIN);
    PortSetup_ConfigureDigitalOutputs(R_ENABLE_PORT, R_ENABLE_PIN);
    
    // Set DIRB Lo for forward and ENABLE lo to ensure motor isn't running
    R_DIRB_LAT = 0;
    R_ENABLE_LAT = 0;
    
    //Turn OC2 off 
    OC2CONbits.ON = 0;
    //Disable SIDLE 
    OC2CONbits.SIDL = 0;
    //Use 16 bit mode 
    OC2CONbits.OC32 = 0;
    //Use timer 3 
    OC2CONbits.OCTSEL = 1;
    //Set to PWM mode, fault disabled 
    OC2CONbits.OCM = 0b110;
    
    //Write initial duty cycle of 0
    OC2R = 0;
    OC2RS = 0;
    
    // Configure Pins
    R_DIRA_REG = OC2_PERIPHERAL_CODE;
    
}

