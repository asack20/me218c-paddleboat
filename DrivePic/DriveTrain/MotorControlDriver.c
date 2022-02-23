/**************************************************************************** 
 * File:   MotorControlDriver.c
 * Driver to interface with 2 motor CL control
 * 
 * Author: Andrew Sack
 * 
 * Created on February 12, 2022, 3:23 PM
 ***************************************************************************/
/*----------------------------- Include Files -----------------------------*/
#include "terminal.h"
#include "MotorControlDriver.h"
#include "../HALs/PIC32PortHAL.h"
#include "DriveTrain.h"
#include "ES_Configure.h"
#include "ES_Events.h"
#include <xc.h>
#include <sys/attribs.h>
#include <string.h>

/*----------------------------- Module Defines ----------------------------*/
// PID constants
#define pGain 1
#define iGain 2
#define dGain 0.5
#define PositionGain 0.5
// Target RPM at the Goal Position. This is to prevent the robot from coming to a stop early
#define PositionGainOffset 2 //(Try 2 rpm)

#define MAX_RPM 250 // RPM measurements above this value will be ignored to reduce noise

#define TICK_DISTANCE_ERROR 0 // Number of ticks error considered at target 

// PWM configuration
#define PWM_TIMER 3
#define PWM_PERIOD 1999 // Base frequency of 10kHz with prescale of 4
#define DUTY_CYCLE_TO_OCRS  2 // multiplier
#define MAX_DUTY_CYCLE 1000
#define CONTROL_LAW_PERIOD 24999 // Set period to be 5 ms
#define PERIOD_2_RPM 1000000 // conversion factor ((10^9*60)/(200*6*50))
#define ZERO_SPEED_PERIOD 1000000 // Amount of ticks considered not moving (1rpm)

// Left motor ports and pins
#define L_DIRB_PORT _Port_A
#define L_DIRB_PIN _Pin_3
#define L_DIRB_LAT LATAbits.LATA3 // Pin 10
#define L_DIRA_REG RPB4R // Pin 11
#define OC1_PERIPHERAL_CODE 0b0101
#define L_OCRS OC1RS

// Right motor ports and pins
#define R_DIRB_PORT _Port_B
#define R_DIRB_PIN _Pin_9
#define R_DIRB_LAT LATBbits.LATB9 // Pin 18
#define R_DIRA_REG RPB8R // Pin 17
#define OC2_PERIPHERAL_CODE 0b0101
#define R_OCRS OC2RS

// Left encoder ports and pins
#define L_ENCODER_CHA PORTAbits.RA4 // Pin 12
#define L_ENCODER_CHB PORTBbits.RB12 // Pin 23

// RIght encoder ports and pins
#define R_ENCODER_CHA PORTBbits.RB10 // Pin 21
#define R_ENCODER_CHB PORTBbits.RB13 // Pin 24

/*----------------------------- Module Types ------------------------------*/

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/
static void InitPWMTimer(void);
static void InitLeftMotor(void);
static void InitRightMotor(void);
static void InitInputCapture(void);
static void InitLeftEncoder(void);
static void InitRightEncoder(void);
static void InitControlLaw(void);

void __ISR(_TIMER_2_VECTOR, IPL6SOFT) Timer2Handler(void);
void __ISR(_TIMER_4_VECTOR, IPL4SOFT) ControlLawHandler(void);
void __ISR(_INPUT_CAPTURE_1_VECTOR, IPL7SOFT) LeftEncoderHandler(void);
void __ISR(_INPUT_CAPTURE_2_VECTOR, IPL7SOFT) RightEncoderHandler(void);
void UpdateControlLaw(ControlState_t *ThisControl, Encoder_t *ThisEncoder);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
static bool MotorsActive; // true if motors are moving in any way. False if stopped
static RolloverTimer_t ICTimerRollover;
static Encoder_t LeftEncoder;
static Encoder_t RightEncoder;
static ControlState_t LeftControl;
static ControlState_t RightControl;

static bool LeftDriveGoalActive;
static bool RightDriveGoalActive;
static bool LeftDriveGoalReached;
static bool RightDriveGoalReached;
/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
    InitMotorControlDriver

 Parameters
    void
 Returns
    bool, false if error in initialization, true otherwise

 Description
    Initializes ports and registers for hbridges and encoders
 Notes

 Author
    Andrew Sack
****************************************************************************/
bool InitMotorControlDriver(void)
{
    puts("Initializing MotorControlDriver...\r");
    
    //disable global interrupts (built in)
    __builtin_disable_interrupts();
    //Enable multi vector configuration 
    INTCONbits.MVEC = 1;
    
    // Call sub-init functions
    InitPWMTimer();
    InitLeftMotor();
    InitRightMotor();
    InitInputCapture();
    InitLeftEncoder();
    InitRightEncoder();
    InitControlLaw();
    
    //enable global interrupts (built in)
    __builtin_enable_interrupts();
    
    // Enable SFRS
    OC1CONbits.ON = 1;
    OC2CONbits.ON = 1;
    IC1CONbits.ON = 1;
    IC2CONbits.ON = 1;
    T2CONbits.ON = 1;
    T3CONbits.ON = 1;
    T4CONbits.ON = 0; // Control law timer. Not turning on for now
    
    // Init Variables to 0
    MotorsActive = 0;
    memset(&ICTimerRollover, 0, sizeof(ICTimerRollover));
    memset(&LeftEncoder, 0, sizeof(LeftEncoder));
    memset(&RightEncoder, 0, sizeof(RightEncoder));
    memset(&LeftControl, 0, sizeof(LeftControl));
    memset(&RightControl, 0, sizeof(RightControl));
    
    LeftDriveGoalActive = 0;
    RightDriveGoalActive = 0;
    LeftDriveGoalReached = 0;
    RightDriveGoalReached = 0;
    
    puts("...Done Initializing MotorControl\r\n");
 
    return true;
}

/****************************************************************************
 * Function
 *      MotorControl_SetMotorDutyCycle     
 *      
 * Parameters
 *      MotorControl_Motor_t WhichMotor - Left or Right Motor
 *      MotorControl_Direction_t WhichDirection - Direction to move motor in
 *          Forward is relative to robot base (CW for right motor, CCW for left)
 *      uint16_t DutyCycle - (valid range 0-1000 inclusive) PWM duty cycle to set 
 * Return
 *      void
 * Description
 *      Set specified motor to move at set PWM duty cycle in specified direction
****************************************************************************/
void MotorControl_SetMotorDutyCycle(MotorControl_Motor_t WhichMotor, MotorControl_Direction_t WhichDirection, uint16_t DutyCycle)
{
    if (_Left_Motor == WhichMotor)
    {  
        //Set WhichMotor's DIRB LAT to WhichDirection to set direction correctly
        L_DIRB_LAT = WhichDirection;
        // invert duty cycle for backwards
        if (_Backward_Dir == WhichDirection)
        {
            DutyCycle = MAX_DUTY_CYCLE - DutyCycle;
        }
        //Set OCRS register for WhichMotor to DUTY_CYCLE_TO_OCRS * Duty Cycle
        L_OCRS = DUTY_CYCLE_TO_OCRS * DutyCycle;

    }
    else if (_Right_Motor == WhichMotor)
    {
        //Set WhichMotor's DIRB LAT to WhichDirection to set direction correctly
        R_DIRB_LAT = WhichDirection;
        // invert duty cycle for backwards
        if (_Backward_Dir == WhichDirection)
        {
            DutyCycle = MAX_DUTY_CYCLE - DutyCycle;
        }
        //Set OCRS register for WhichMotor to DUTY_CYCLE_TO_OCRS * Duty Cycle
        R_OCRS = DUTY_CYCLE_TO_OCRS * DutyCycle;

    }
	//Motors are now moving
    MotorsActive = true;
}
	
/****************************************************************************
 * Function
 *      MotorControl_StopMotors   
 *      
 * Parameters
 *      void
 * Return
 *      void
 * Description
 *      Stop both motors
****************************************************************************/
void MotorControl_StopMotors(void)
{
    //Call SetMotorDutyCycle with duty cycle of 0
    MotorControl_SetMotorDutyCycle(_Left_Motor, _Forward_Dir, 0);
	MotorControl_SetMotorDutyCycle(_Right_Motor, _Forward_Dir, 0);	
	//Update States
    MotorsActive = false;
    
    // Set Target Speed to 0 for control law
    LeftControl.TargetRPM = 0;
    RightControl.TargetRPM = 0;
    
    // Cancel any tick goal
    LeftControl.TargetTickCount = 0;
    RightControl.TargetTickCount = 0;
    
    // Reset Drive Goals
    LeftDriveGoalActive = 0;
    LeftDriveGoalReached = 0;
    RightDriveGoalActive = 0;
    RightDriveGoalReached = 0;

}

/****************************************************************************
 * Function
 *      MotorControl_EnableClosedLoop 
 *      
 * Parameters
 *      void
 * Return
 *      void
 * Description
 *      Enables Control Law timer interrupt, causing motors to follow CL control
 *      SetMotorDutyCycle will not work properly when enabled
****************************************************************************/
void MotorControl_EnableClosedLoop(void)
{
    // Turn on timer to start control law running
    T4CONbits.ON = 1;
}

/****************************************************************************
 * Function
 *      MotorControl_DisableClosedLoop 
 *      
 * Parameters
 *      void
 * Return
 *      void
 * Description
 *      Disables Control Law timer interrupt. Motors use direct Duty Cycle
 *      SetMotorDutyCycle will not work properly when enabled
****************************************************************************/
void MotorControl_DisableClosedLoop(void)
{
    // Turn off timer to stop control law running
    T4CONbits.ON = 0;
    
    // Reset all error terms
    LeftControl.IntegralTerm = 0;
    LeftControl.RPMError = 0;
    LeftControl.LastError = 0;
    LeftControl.SumError = 0;

    RightControl.IntegralTerm = 0;
    RightControl.RPMError = 0;
    RightControl.LastError = 0;
    RightControl.SumError = 0;
}

/****************************************************************************
 * Function
 *      MotorControl_SetMotorSpeed
 *      
 * Parameters
 *      MotorControl_Motor_t WhichMotor - Left or Right Motor
 *      MotorControl_Direction_t WhichDirection - Direction to move motor in
 *          Forward is relative to robot base (CW for right motor, CCW for left)
 *      uint16_t Speed - target speed to move at in units of 0.1 RPM (5 RPM = 50)
 *                       Max speed is approx 170 RPM
 * Return
 *      void
 * Description
 *      Set specified motor to move at set PWM duty cycle in specified direction
 *      Only works when CloseLoop is enabled
****************************************************************************/
void MotorControl_SetMotorSpeed(MotorControl_Motor_t WhichMotor, MotorControl_Direction_t WhichDirection, uint16_t Speed)
{
    MotorsActive = true;
    // Turn on closed loop control whenever a speed command is sent
    MotorControl_EnableClosedLoop();
    
    if (_Left_Motor == WhichMotor)
    {
        LeftControl.TargetDirection = WhichDirection;
        // input is speed in units of 0.1 rpm so need to convert to rpm
        LeftControl.TargetRPM = (float) Speed / 10;
    }
    
    else if (_Right_Motor == WhichMotor)
    {
        RightControl.TargetDirection = WhichDirection;
        // input is speed in units of 0.1 rpm so need to convert to rpm
        RightControl.TargetRPM = (float) Speed / 10;        
    }   
}
/****************************************************************************
 * Function
 *      MotorControl_ResetTickCount
 *      
 * Parameters
 *      MotorControl_Motor_t WhichMotor - Left or Right Motor
 * Return
 *      void
 * Description
 *      Sets current tick count for specified motor to zero
****************************************************************************/
void MotorControl_ResetTickCount(MotorControl_Motor_t WhichMotor)
{
    if (_Left_Motor == WhichMotor)
    {
        LeftEncoder.TickCount = 0;
    }
    else if (_Right_Motor == WhichMotor)
    {
        RightEncoder.TickCount = 0;       
    }   
}

/****************************************************************************
 * Function
 *      MotorControl_SetTickGoal
 *      
 * Parameters
 *      MotorControl_Motor_t WhichMotor - Left or Right Motor
 *      uint32_t NumTicks - Amount of ticks encoder will count to before stopping
 * Return
 *      void
 * Description
 *      Sets tick goal for specified motor to NumTicks
****************************************************************************/
void MotorControl_SetTickGoal(MotorControl_Motor_t WhichMotor, uint32_t NumTicks)
{
    if (_Left_Motor == WhichMotor)
    {
        LeftControl.TargetTickCount = NumTicks; 
        
        // reset DriveReached
        LeftDriveGoalReached = false;
        // if NumTicks nonzero set to active
        LeftDriveGoalActive = (NumTicks != 0);
    }
    else if (_Right_Motor == WhichMotor)
    {
        RightControl.TargetTickCount = NumTicks;
        
        // reset DriveReached
        RightDriveGoalReached = false;
        // if NumTicks nonzero set to active
        RightDriveGoalActive = (NumTicks != 0);
    }   
}

/****************************************************************************
 * Function
 *      MotorControl_DriveStraight
 *      
 * Parameters
 *      MotorControl_Direction_t WhichDirection - Direction to drive in 
 *                                      (forward or backward)       
 *      uint16_t Speed - target speed to move at in units of 0.1 RPM (5 RPM = 50)
 *                       Max speed is approx 170 RPM
 *      uint16_t DistanceCM - Ground distance to travel in centimeters
 *                  if set to 0, will drive indefinitely
 * Return
 *      void
 * Description
 *      Drive whole drive train straight forward or backward at set speed for 
 *      specified distance
****************************************************************************/
void MotorControl_DriveStraight(MotorControl_Direction_t WhichDirection, uint16_t Speed, uint16_t DistanceCM)
{
    // Reset Tick Count
    MotorControl_ResetTickCount(_Left_Motor);
    MotorControl_ResetTickCount(_Right_Motor);
    
    // Set Target Tick Count
    // Do math in floating point
    uint16_t NumTicks = (uint16_t) ((float) DistanceCM * TICKS_PER_CM);
    MotorControl_SetTickGoal(_Left_Motor, NumTicks);
    MotorControl_SetTickGoal(_Right_Motor, NumTicks);
    
    // Set Speed
    MotorControl_SetMotorSpeed(_Left_Motor, WhichDirection, Speed);
    MotorControl_SetMotorSpeed(_Right_Motor, WhichDirection, Speed);
}

/****************************************************************************
 * Function
 *      MotorControl_DriveTurn
 *      
 * Parameters
 *      MotorControl_Turn_t WhichTurn - Direction to turn (clock or counterclock)      
 *      uint16_t Speed - target speed to move at in units of 0.1 RPM (5 RPM = 50)
 *                       Max speed is approx 170 RPM
 *      uint16_t AngleDeg - Angle in degrees to rotate base by
 *                  if set to 0, will drive indefinitely
 * Return
 *      void
 * Description
 *      Turn whole drive train on the spot by Angle in specified direction and speed
****************************************************************************/
void MotorControl_DriveTurn(MotorControl_Turn_t WhichTurn, uint16_t Speed, uint16_t AngleDeg)
{
    // Reset Tick Count
    MotorControl_ResetTickCount(_Left_Motor);
    MotorControl_ResetTickCount(_Right_Motor);
    
    // Set Target Tick Count
    // Do math in floating point
    uint16_t NumTicks = (uint16_t) ((float) AngleDeg * TICKS_PER_DEGREE);
    MotorControl_SetTickGoal(_Left_Motor, NumTicks);
    MotorControl_SetTickGoal(_Right_Motor, NumTicks);
    
    // Set Speed and direction
    if (WhichTurn == _Clockwise_Turn)
    {
        MotorControl_SetMotorSpeed(_Left_Motor, _Forward_Dir, Speed);
        MotorControl_SetMotorSpeed(_Right_Motor, _Backward_Dir, Speed);
    }
    else
    {
        MotorControl_SetMotorSpeed(_Left_Motor, _Backward_Dir, Speed);
        MotorControl_SetMotorSpeed(_Right_Motor, _Forward_Dir, Speed);
    }
    
}

/****************************************************************************
 * Function
 *      MotorControl_GetEncoder
 *      
 * Parameters
 *      MotorControl_Motor_t WhichMotor - Left or Right Motor
 * Return
 *      Encoder_t struct for specified motor
 * Description
 *      Get function to return Encoder Struct for specified Motor
****************************************************************************/
Encoder_t MotorControl_GetEncoder(MotorControl_Motor_t WhichMotor)
{
    if (_Left_Motor == WhichMotor)
    {
        return LeftEncoder;
    }
    else
    {
        return RightEncoder;      
    }
}

/****************************************************************************
 * Function
 *      MotorControl_GetControlState
 *      
 * Parameters
 *      MotorControl_Motor_t WhichMotor - Left or Right Motor
 * Return
 *      ControlState_t struct for specified motor
 * Description
 *      Get function to return ControlState Struct for specified Motor
****************************************************************************/
ControlState_t MotorControl_GetControlState(MotorControl_Motor_t WhichMotor)
{
    if (_Left_Motor == WhichMotor)
    {
        return LeftControl;
    }
    else
    {
        return RightControl;      
    }
}

/***************************************************************************
 private functions
 ***************************************************************************/

/*
 * InitPWMTimer
 * Helper Function for InitMotorControl
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
 * Helper Function for InitMotorControl
 * Handles configuration of ports and OC for Left Motor
 */
static void InitLeftMotor(void)
{
    PortSetup_ConfigureDigitalOutputs(L_DIRB_PORT, L_DIRB_PIN);

    // Set DIRB Lo for forward to ensure motor isn't running
    L_DIRB_LAT = 0;
    
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
 * Helper Function for InitMotorControl
 * Handles configuration of ports and OC for Right Motor
 */
static void InitRightMotor(void)
{
    PortSetup_ConfigureDigitalOutputs(R_DIRB_PORT, R_DIRB_PIN);
    
    // Set DIRB Lo for forward to ensure motor isn't running
    R_DIRB_LAT = 0;
    
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

/*
 * InitInputCapture
 * Helper Function for InitMotorControl
 * Handles configuration interrupts, and timers for input capture
 */
static void InitInputCapture(void)
{
    // Clear Interrupt flags
    IFS0CLR = _IFS0_T2IF_MASK;
    // Set priority
    IPC2bits.T2IP = 6;
    // Enable Interrupt
    IEC0SET = _IEC0_T2IE_MASK;
    
    // Timer 2 (Encoder input capture)
    //turn timer off 
    T2CONbits.ON = 0;
    //disable stop in idle 
    T2CONbits.SIDL = 0;
    //Set prescaler to 4 
    T2CONbits.TCKPS = 0b010;
    //Set to 16-bit mode 
    T2CONbits.T32 = 0;
    //Use syncronous internal clock 
    T2CONbits.TCS = 0; 
    T2CONbits.TGATE = 0;
    //Set period to max 
    PR2 = 0xFFFF;
    //Clear timer to 0 
    TMR2 = 0;
}

/*
 * InitLeftEncoder
 * Helper Function for InitMotorControl
 * Handles configuration of IC and ports for left encoder
 */
static void InitLeftEncoder(void)
{
    // Clear Interrupt flags
    IFS0CLR = _IFS0_IC1IF_MASK;
    IFS0CLR = _IFS0_IC1EIF_MASK;
    //Set interrupt priority
    IPC1bits.IC1IP = 7;
    //enable interrupt
    IEC0SET = _IEC0_IC1IE_MASK;
    
    ////Input Capture 1 configuration (Encoder)
    //Turn IC1 Off 
    IC1CONbits.ON = 0;
    //disable SIDL 
    IC1CONbits.SIDL = 0;
    //Configure Mode Edge Detect mode ? every edge (rising and falling)
    IC1CONbits.ICM = 0b001;
    //Use 16 bit timer 
    IC1CONbits.C32 = 0;
    //Use timer 2
    IC1CONbits.ICTMR = 1;
    //Interrupt every event 
    IC1CONbits.ICI = 0b00;
    //Clear IC1 buffer by reading IC1BUF 
    IC1BUF;
    
    //Encoder Port Setup
    PortSetup_ConfigureDigitalInputs(_Port_A, _Pin_4); // CH A
    PortSetup_ConfigureDigitalInputs(_Port_B, _Pin_12); // CH B
    // Map IC1 to RPA4 
    IC1R = 0b0010;
    
}

/*
 * InitRightEncoder
 * Helper Function for InitMotorControl
 * Handles configuration of IC and ports for Right encoder
 */
static void InitRightEncoder(void)
{
    // Clear Interrupt flags
    IFS0CLR = _IFS0_IC2IF_MASK;
    IFS0CLR = _IFS0_IC2EIF_MASK;
    //Set interrupt priority
    IPC2bits.IC2IP = 7;
    //enable interrupt
    IEC0SET = _IEC0_IC2IE_MASK;
    
    ////Input Capture 2 configuration (Encoder)
    //Turn IC2 Off 
    IC2CONbits.ON = 0;
    //disable SIDL 
    IC2CONbits.SIDL = 0;
    //Configure Mode Edge Detect mode ? every edge (rising and falling)
    IC2CONbits.ICM = 0b001;
    //Use 16 bit timer 
    IC2CONbits.C32 = 0;
    //Use timer 2
    IC2CONbits.ICTMR = 1;
    //Interrupt every event 
    IC2CONbits.ICI = 0b00;
    //Clear IC2 buffer by reading IC1BUF 
    IC2BUF;
    
    //Encoder Port Setup
    PortSetup_ConfigureDigitalInputs(_Port_B, _Pin_10); // CH A
    PortSetup_ConfigureDigitalInputs(_Port_B, _Pin_13); // CH B
    // Map IC2 to RPB10 
    IC2R = 0b0011;
}

/*
 * InitControlLaw
 * Helper Function for InitMotorControl
 * Handles configuration of interrupts and timer for control law
 */
static void InitControlLaw(void)
{
    // Clear Interrupt flags
    IFS0CLR = _IFS0_T4IF_MASK;
    //Set priority for Timer 4 to 4 
    IPC4bits.T4IP = 4;
    // enable interrupt
    IEC0SET = _IEC0_T4IE_MASK;
    
    // Timer 4 (Control Law timer)
    //turn timer off 
    T4CONbits.ON = 0;
    //disable stop in idle 
    T4CONbits.SIDL = 0;
    //Set prescaler to 4 
    T4CONbits.TCKPS = 0b010;
    //Set to 16-bit mode 
    T4CONbits.T32 = 0;
    //Use syncronous internal clock 
    T4CONbits.TCS = 0; 
    T4CONbits.TGATE = 0;
    //Set period to 24999 for time of 5 ms
    PR4 = CONTROL_LAW_PERIOD;
    //Clear timer to 0 
    TMR4 = 0;
}
/****************************************************************************
 Function
 Timer2Handler

 Parameters
     None

 Returns
 void
 Description
 Interrupt Handler for Encoder timer rollover
 Notes

 Author
 * Andrew Sack 
****************************************************************************/
void __ISR(_TIMER_2_VECTOR, IPL6SOFT) Timer2Handler(void)
{
    //Disable interrupts globally (creates protected region in case IC happens)
    __builtin_disable_interrupts();
    //If T2IF is pending (there is a small chance that the IC has fired and handled it already)
    if (1 == IFS0bits.T2IF)
    {
        //	Increment the rollover counter
        ICTimerRollover.Rollover++;
        //	Clear the roll?over interrupt (timer interrupt flag)
        IFS0CLR = _IFS0_T2IF_MASK;  
    }
    
    // Check for long time since last tick and set speed to 0 if so
    if ((ICTimerRollover.TotalTime - LeftEncoder.LastTime.TotalTime) > ZERO_SPEED_PERIOD)
    {
        LeftEncoder.CurrentRPM = 0;
    }
        if ((ICTimerRollover.TotalTime - RightEncoder.LastTime.TotalTime) > ZERO_SPEED_PERIOD)
    {
        RightEncoder.CurrentRPM = 0;
    }
    
    
    //Enable interrupts (OK, since we know interrupts were enabled to get here)
    __builtin_enable_interrupts();
}

/****************************************************************************
 Function
 LeftEncoderHandler

 Parameters
     None

 Returns
 void
 Description
 Interrupt Handler for LeftEncoder
 Notes

 Author
 * Andrew Sack 
****************************************************************************/
void __ISR(_INPUT_CAPTURE_1_VECTOR, IPL7SOFT) LeftEncoderHandler(void)
{
    __builtin_disable_interrupts();
    
    //Read ICxBUFinto a static variable
    ICTimerRollover.CapturedTime = IC1BUF;
    //Clear the capture interrupt flag
    IFS0CLR = _IFS0_IC1IF_MASK;
    //If T2IF is pending and CapturedTime is after rollover (<0x8000)
    if (1 == IFS0bits.T2IF && ICTimerRollover.CapturedTime < 0x8000)
    {
        //	Increment the rollover counter
        ICTimerRollover.Rollover++;
        //	Clear the rollover interrupt (timer interrupt flag)
        IFS0CLR = _IFS0_T2IF_MASK;
    }   
    
    //Copy timer value to Encoder struct and calculate speed
    LeftEncoder.CurrentPeriod.TotalTime = ICTimerRollover.TotalTime - LeftEncoder.LastTime.TotalTime;
    LeftEncoder.LastTime.TotalTime = ICTimerRollover.TotalTime;
    LeftEncoder.TickCount++;
    
    // Calculate RPM. Only keep if below max
    float TempRPM = (float) PERIOD_2_RPM / LeftEncoder.CurrentPeriod.TotalTime;
    if (TempRPM < MAX_RPM) LeftEncoder.CurrentRPM = TempRPM;
    
    // Trigger was a rising edge
    if (1 == L_ENCODER_CHA)
    {
        // direction is value on ch B
        LeftEncoder.Direction = L_ENCODER_CHB; // 0 is forward, 1 is backward
    }
    else
    {
        // direction is inverted
        LeftEncoder.Direction = !L_ENCODER_CHB;
    }
    
    // Distance handling
    if (LeftControl.TargetTickCount != 0) // Tick Target is set
    {
        // target reached within margin of error
        if (LeftEncoder.TickCount >= (LeftControl.TargetTickCount - TICK_DISTANCE_ERROR))
        {
            printf("Left Drive Goal Reached \r\n");
            // stop motor
            LeftControl.TargetRPM = 0;
            //Set DriveGoalReached to true
            LeftDriveGoalReached = true;
            // Reset TargetTickCount
            LeftControl.TargetTickCount = 0;
        }
    }    
    __builtin_enable_interrupts();
}

/****************************************************************************
 Function
 RightEncoderHandler

 Parameters
     None

 Returns
 void
 Description
 Interrupt Handler for RightEncoder
 Notes

 Author
 * Andrew Sack 
****************************************************************************/
void __ISR(_INPUT_CAPTURE_2_VECTOR, IPL7SOFT) RightEncoderHandler(void)
{
     __builtin_disable_interrupts();
    
    //Read ICxBUFinto a static variable
    ICTimerRollover.CapturedTime = IC2BUF;
    //Clear the capture interrupt flag
    IFS0CLR = _IFS0_IC2IF_MASK;
    //If T2IF is pending and CapturedTime is after rollover (<0x8000)
    if (1 == IFS0bits.T2IF && ICTimerRollover.CapturedTime < 0x8000)
    {
        //	Increment the rollover counter
        ICTimerRollover.Rollover++;
        //	Clear the rollover interrupt (timer interrupt flag)
        IFS0CLR = _IFS0_T2IF_MASK;
    }   
    
    //Copy timer value to Encoder struct and calculate speed
    RightEncoder.CurrentPeriod.TotalTime = ICTimerRollover.TotalTime - RightEncoder.LastTime.TotalTime;
    RightEncoder.LastTime.TotalTime = ICTimerRollover.TotalTime;
    RightEncoder.TickCount++;
    
    // Calculate RPM. Only keep if below max
    float TempRPM = (float) PERIOD_2_RPM / RightEncoder.CurrentPeriod.TotalTime;
    if (TempRPM < MAX_RPM) RightEncoder.CurrentRPM = TempRPM;
    
    // Trigger was a rising edge
    if (1 == R_ENCODER_CHA)
    {
        // direction is value on ch B
        RightEncoder.Direction = R_ENCODER_CHB; // 0 is forward, 1 is backward
    }
    else
    {
        // direction is inverted
        RightEncoder.Direction = !R_ENCODER_CHB;
    }
    // Distance handling
    if (RightControl.TargetTickCount != 0) // Tick Target is set
    {
        // target reached
        if (RightEncoder.TickCount >= (RightControl.TargetTickCount - TICK_DISTANCE_ERROR))
        {
            printf("Right Drive Goal Reached \r\n");
            // stop motor
            RightControl.TargetRPM = 0;
            //Set DriveGoalReached to true
            RightDriveGoalReached = true;
            // Reset TargetTickCount
            RightControl.TargetTickCount = 0;
        }
    } 
    
    // reenable interrupts
    __builtin_enable_interrupts();
}

/****************************************************************************
 Function
 ControlLawHandler

 Parameters
     None

 Returns
 void
 Description
 Interrupt Handler for Control Law Timer
 Notes

 Author
 * Andrew Sack 
****************************************************************************/
void __ISR(_TIMER_4_VECTOR, IPL4SOFT) ControlLawHandler(void)
{
    //	Clear the timer interrupt flag
    IFS0CLR = _IFS0_T4IF_MASK;  
    
    // Left Motor Control Law
    UpdateControlLaw(&LeftControl, &LeftEncoder);
    MotorControl_SetMotorDutyCycle(_Left_Motor, LeftControl.TargetDirection, (uint16_t) LeftControl.RequestedDutyCycle);
    
    // Right Motor Control Law.
    UpdateControlLaw(&RightControl, &RightEncoder);
    MotorControl_SetMotorDutyCycle(_Right_Motor, RightControl.TargetDirection, (uint16_t)RightControl.RequestedDutyCycle);
    
    // Check Drive Goal status for event posting
    bool DriveGoalReached = false;
    // if either are active
    if (LeftDriveGoalActive || RightDriveGoalActive)
    {
        // If either are active and not reached, its false. Else, true
        DriveGoalReached = !((LeftDriveGoalActive && !LeftDriveGoalReached) || 
                (RightDriveGoalActive && !RightDriveGoalReached));
    }
    // Post event if reached
    if (DriveGoalReached)
    {
        printf("MotorControl: Posting DRIVE_GOAL_REACHED\n\r");
        ES_Event_t PostEvent;
        PostEvent.EventType = DRIVE_GOAL_REACHED;
        PostDriveTrain(PostEvent);
        
        // Clear all flags
        LeftDriveGoalActive = false;
        LeftDriveGoalReached = false;
        RightDriveGoalActive = false;
        RightDriveGoalReached = false;
    }
}

/****************************************************************************
 Function
 UpdateControlLaw

 Parameters
 ControlState_t *ThisControl - Pointer to Control struct for desired motor
 Encoder_t *Encoder         - Pointer to Encoder struct for desired motor

 Returns
 void
 Description
 Helper function for control law
 Notes

 Author
 * Andrew Sack 
****************************************************************************/
void UpdateControlLaw(ControlState_t *ThisControl, Encoder_t *ThisEncoder)
{
    
    // Position goal set
    if (ThisControl->TargetTickCount != 0)
    {
        // Scale target velocity based on distance to goal
        ThisControl->ActualTargetRPM = (PositionGain * 
                (float)(ThisControl->TargetTickCount - ThisEncoder->TickCount))
                + PositionGainOffset; // offset the 0rpm point to beyond the goal
                                      // to prevent issues with very low velocity motion
        
        // Bound by 0 and TargetRPM
        // Take Min of the two
        ThisControl->ActualTargetRPM = 
                (ThisControl->ActualTargetRPM  < ThisControl->TargetRPM) ? 
                    ThisControl->ActualTargetRPM : ThisControl->TargetRPM;
        
        // Take Max of the two
        ThisControl->ActualTargetRPM = 
                (ThisControl->ActualTargetRPM  > 0) ? 
                    ThisControl->ActualTargetRPM : 0;
    }
    else // no position target. Always move at target velocity
    {
        ThisControl->ActualTargetRPM = ThisControl->TargetRPM;
    }
    
    ThisControl->RPMError = ThisControl->ActualTargetRPM - ThisEncoder->CurrentRPM;
    ThisControl->SumError += ThisControl->RPMError;
    ThisControl->RequestedDutyCycle =
    (pGain * ((ThisControl->RPMError)+(iGain * ThisControl->SumError)+
            (dGain* (ThisControl->RPMError-ThisControl->LastError))));
    if (ThisControl->RequestedDutyCycle > MAX_DUTY_CYCLE) {
        ThisControl->RequestedDutyCycle = MAX_DUTY_CYCLE;
        ThisControl->SumError -= ThisControl->RPMError;   /* anti-windup */
    }else if (ThisControl->RequestedDutyCycle < 0){
        ThisControl->RequestedDutyCycle = 0;
        ThisControl->SumError -= ThisControl->RPMError;   /* anti-windup */
    } 
    ThisControl->LastError = ThisControl->RPMError; // update
}