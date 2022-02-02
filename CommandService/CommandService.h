/**************************************************************************** 
 * File:   CommandService.h
 * FSM to interface with 2-motor PWM drive train
 * 
 * Author: Afshan Chandani
 * 
 * Created on February 2, 2022, 11:00 PM
 ***************************************************************************/

#ifndef CommandService_H
#define CommandService_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// typedefs for the states
// State definitions for use with the query function
typedef enum
{
    CommandInitState, CommandQueryState, CommandReceiveState
}CommandServiceState_t;

// Public Function Prototypes

bool InitCommandService(uint8_t Priority);
bool PostCommandService(ES_Event_t ThisEvent);
ES_Event_t RunCommandService(ES_Event_t ThisEvent);
CommandServiceState_t QueryCommandService(void);

bool CheckSPIRBF(void);
#endif /* CommandService_H */

