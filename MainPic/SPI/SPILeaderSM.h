/**************************************************************************** 
 * File:   SPILeaderSM.h
 * FSM to interface with 2-motor PWM drive train
 * 
 * Author: Afshan Chandani
 * 
 * Created on February 2, 2022, 11:00 PM
 ***************************************************************************/

#ifndef SPILeaderSM_H
#define SPILeaderSM_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// typedefs for the states
// State definitions for use with the query function
typedef enum
{
    SPILeaderInitState, SPILeaderQueryState, SPILeaderReceiveState
}SPILeaderSMState_t;

// Public Function Prototypes

bool InitSPILeaderSM(uint8_t Priority);
bool PostSPILeaderSM(ES_Event_t ThisEvent);
ES_Event_t RunSPILeaderSM(ES_Event_t ThisEvent);
SPILeaderSMState_t QuerySPILeaderSM(void);

bool CheckSPIRBF(void);
#endif /* SPILeaderSM_H */

