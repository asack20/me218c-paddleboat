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

// Public Function Prototypes

bool InitMotorControlDriver(void);

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

#endif	/* MOTORCONTROLDRIVER_H */