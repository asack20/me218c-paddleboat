/**************************************************************************** 
 * File:   TugComm.h
 * FSM to handle high level pairing and communication for the TUG
 * 
 * Author: Andrew Sack
 * 
 * Created on May 13, 2022, 10:26 AM
 ***************************************************************************/

#ifndef TugComm_H
#define TugComm_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */
#include "ES_Framework.h"

// typedefs for the states
// State definitions for use with the query function
typedef enum
{
    WaitingForPairRequestState, WaitingForControlPacketState, PairedState
}TugCommState_t;

// Public Function Prototypes

bool InitTugComm(uint8_t Priority);
bool PostTugComm(ES_Event_t ThisEvent);
ES_Event_t RunTugComm(ES_Event_t ThisEvent);
TugCommState_t QueryTugComm(void);


#endif /* TugComm_H */

