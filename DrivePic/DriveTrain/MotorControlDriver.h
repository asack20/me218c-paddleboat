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
    float TargetRPM;
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
 *      Stop both motors
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
 *                       Max speed is approx 40-50 RPM
 * Return
 *      void
 * Description
 *      Set specified motor to move at set PWM duty cycle in specified direction
 *      Only works when CloseLoop is enabled
****************************************************************************/
void MotorControl_SetMotorSpeed(MotorControl_Motor_t WhichMotor, MotorControl_Direction_t WhichDirection, uint16_t Speed);

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

#endif	/* MOTORCONTROLDRIVER_H */