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
#include <xc.h>
#include <sys/attribs.h>

/*----------------------------- Module Defines ----------------------------*/
// PWM configuration
#define PWM_TIMER 3
#define PWM_PERIOD 1999 // Base frequency of 10kHz with prescale of 4
#define DUTY_CYCLE_TO_OCRS 2// multiplier
#define MAX_DUTY_CYCLE 1000

// Command Tuning
#define FULL_DUTY_CYCLE 100 // Duty cycle for 100% speed
#define HALF_DUTY_CYCLE 75 // Duty cycle for 50% speed
#define ROT_DUTY_CYCLE 100 // Duty cycle used for rotating
#define ROT_90_TIME 1500 // time in ms required to rotate 90 degrees
#define ROT_45_TIME 750 // time in ms required to rotate 45 degrees

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
/*----------------------------- Module Types ------------------------------*/
// encoder tick count type
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
static bool MotorsActive; // true if motors are moving in any way. False if stopped

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
bool InitMotorControl(void)
{
    puts("Initializing MotorControlDriver...\r");
    // Call sub-init functions
    InitPWMTimer();
    InitLeftMotor();
    InitRightMotor();
    
    // Enable SFRS
    OC1CONbits.ON = 1;
    OC2CONbits.ON = 1;
    T3CONbits.ON = 1;
    
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
 *      uint8_t DutyCycle - (valid range 0-1000 inclusive) PWM duty cycle to set 
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


