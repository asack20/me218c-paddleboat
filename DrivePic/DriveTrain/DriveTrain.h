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

// typedefs for the states
// State definitions for use with the query function
typedef enum
{
    DriveInitState, DriveReadyState, DriveBusyState
}DriveTrainState_t;

// Public Function Prototypes

bool InitDriveTrain(uint8_t Priority);
bool PostDriveTrain(ES_Event_t ThisEvent);
ES_Event_t RunDriveTrain(ES_Event_t ThisEvent);
DriveTrainState_t QueryDriveTrain(void);

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

