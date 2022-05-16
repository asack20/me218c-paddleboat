/**************************************************************************** 
 * File:   SPIFollowerSM.h
 * FSM to interface with 2-motor PWM drive train
 * 
 * Author: Afshan Chandani
 * 
 * Created on February 2, 2022, 11:00 PM
 ***************************************************************************/

#ifndef SPIFollowerSM_H
#define SPIFollowerSM_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// typedefs for the states
// State definitions for use with the query function
typedef enum
{
    SPIFollowerInitState, SPIFollowerReceiveState
}SPIFollowerSMState_t;

// Public Function Prototypes

bool InitSPIFollowerSM(uint8_t Priority);
bool PostSPIFollowerSM(ES_Event_t ThisEvent);
ES_Event_t RunSPIFollowerSM(ES_Event_t ThisEvent);
SPIFollowerSMState_t QuerySPIFollowerSM(void);

bool QueryRefuelInProgress(void);
void ClearRefuelInProgress(void);
void SetRefuelInProgress(void);
bool QueryRefuelDone(void);
void ClearRefuelDone(void);

bool CheckSPIRBF(void);
#endif /* SPIFollowerSM_H */

