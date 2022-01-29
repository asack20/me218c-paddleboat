/**************************************************************************** 
 * File:   DriveTrain.h
 * Module to interface with 2-motor PWM drive train
 * 
 * Author: Andrew Sack
 * 
 * Created on January 28, 2022, 10:07 PM
 ***************************************************************************/

#ifndef DRIVETRAIN_H
#define	DRIVETRAIN_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
  _Left_Motor = 0,
  _Right_Motor = 1,
}DriveTrain_Motor_t;

typedef enum
{
  _Forward_Dir = 0,
  _Backward_Dir = 1,
}DriveTrain_Direction_t;

/****************************************************************************
 * Function
 *      DriveTrain_InitDriveTrain
 *      
 * Parameters
 *      void
 * Return
 *      True if initialization successful. Else, false    
 * Description
 *      Initialization for Ports, OC, Timers, and Interrupts required
 *      Must be called before using any other functions
****************************************************************************/
bool DriveTrain_InitDriveTrain(void);

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
void DriveTrain_SetMotorDutyCycle(DriveTrain_Motor_t WhichMotor, DriveTrain_Direction_t WhichDirection, uint8_t DutyCycle);

void DriveTrain_RotateNDegrees(int16_t RotationDegrees);

// Potential Future Functions
// void DriveTrain_SetMotorSpeed(WhichMotor, float SpeedRPM);
// void DriveTrain_DriveNTicks
// void DriveTrain_DriveNInches
// void DriveTrain_RotateXDegrees (encoder used, also maybe allow speed change)

#endif	/* DRIVETRAIN_H */

