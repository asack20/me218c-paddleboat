/**************************************************************************** 
 * File:   MotorControlDriver.h
 * Driver to interface with 2 motor CL control
 * 
 * Author: Andrew Sack
 * 
 * Created on February 12, 2022, 3:23 PM
 ***************************************************************************/

#ifndef MOTORCONTROLDRIVER_H
#define	MOTORCONTROLDRIVER_H

#include "ES_Types.h"     /* gets bool type for returns */

// Drive Train (In header to allow use in other modules)
#define TICKS_PER_CM 7.639 // Encoder ticks per cm of drive train distance
#define TICKS_PER_DEGREE 1.85 // ticks per degree of drive train rotation (was 1.8)(was 1.763)

typedef enum
{
  _Left_Motor = 0,
  _Right_Motor = 1,
}MotorControl_Motor_t;

typedef enum
{
  _Forward_Dir = 0,
  _Backward_Dir = 1,
}MotorControl_Direction_t;

typedef enum
{
  _Clockwise_Turn = 0,
  _CounterClockwise_Turn = 1,
}MotorControl_Turn_t;

// encoder tick count type
typedef union {
    struct {
        uint16_t CapturedTime;
        uint16_t Rollover;   
    };
    uint32_t TotalTime;
} RolloverTimer_t;


typedef struct {
    RolloverTimer_t LastTime;   // time of last encoder tick
    RolloverTimer_t CurrentPeriod; // Period of most recent tick
    float CurrentRPM;           // Speed of most recent tick in RPM
    uint32_t TickCount;         // Number of ticks since last reset
    MotorControl_Direction_t Direction; // Direction of last tick
}Encoder_t ;

typedef struct {
    uint32_t TargetTickCount;
    float TargetRPM;            // Set by user
    float ActualTargetRPM;      // Actual target used by control law. Changed based on distance when TickGoalSet
    float RequestedDutyCycle;
    float IntegralTerm;
    float RPMError;
    float LastError;
    float SumError;
    MotorControl_Direction_t TargetDirection;
}ControlState_t;

// Public Function Prototypes

// Initialization function for Driver. Configures all pins, timers, interrupts
bool InitMotorControlDriver(void);

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
 *      Only works with CLosed Loop Control Disabled
****************************************************************************/
void MotorControl_SetMotorDutyCycle(MotorControl_Motor_t WhichMotor, MotorControl_Direction_t WhichDirection, uint16_t DutyCycle);

/****************************************************************************
 * Function
 *      MotorControl_StopMotors
 *      
 * Parameters
 *      void
 * Return
 *      void
 * Description
 *      Stop both motors, and cancel any tick goal
****************************************************************************/
void MotorControl_StopMotors(void);

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
void MotorControl_EnableClosedLoop(void);

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
void MotorControl_DisableClosedLoop(void);

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
void MotorControl_SetMotorSpeed(MotorControl_Motor_t WhichMotor, MotorControl_Direction_t WhichDirection, uint16_t Speed);

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
void MotorControl_ResetTickCount(MotorControl_Motor_t WhichMotor);

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
void MotorControl_SetTickGoal(MotorControl_Motor_t WhichMotor, uint32_t NumTicks);


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
Encoder_t MotorControl_GetEncoder(MotorControl_Motor_t WhichMotor);

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
ControlState_t MotorControl_GetControlState(MotorControl_Motor_t WhichMotor);

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
void MotorControl_DriveStraight(MotorControl_Direction_t WhichDirection, uint16_t Speed, uint16_t DistanceCM);

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
void MotorControl_DriveTurn(MotorControl_Turn_t WhichTurn, uint16_t Speed, uint16_t AngleDeg);



#endif	/* MOTORCONTROLDRIVER_H */