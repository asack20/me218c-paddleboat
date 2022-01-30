/**************************************************************************** 
 * File:   DriveTrain.h
 * FSM to interface with 2-motor PWM drive train
 * 
 * Author: Andrew Sack
 * 
 * Created on January 28, 2022, 10:07 PM
 ***************************************************************************/

#ifndef DriveTrain_H
#define DriveTrain_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

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

// typedefs for the states
// State definitions for use with the query function
typedef enum
{
    DriveInitState, DriveStoppedState, DriveIdleMovingState, DriveActiveMovingState
}DriveTrainState_t;

// Public Function Prototypes

bool InitDriveTrain(uint8_t Priority);
bool PostDriveTrain(ES_Event_t ThisEvent);
ES_Event_t RunDriveTrain(ES_Event_t ThisEvent);
DriveTrainState_t QueryDriveTrain(void);

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
void DriveTrain_StopMotors(void);

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
void DriveTrain_StopAfterDelay(uint16_t DelayMS);

#endif /* DriveTrain_H */

