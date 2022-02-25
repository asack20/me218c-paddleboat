/****************************************************************************

  Header file for template Flat Sate Machine
  based on the Gen2 Events and Services Framework

 ****************************************************************************/

#ifndef BUMPER_SERVICE_H
#define BUMPER_SERVICE_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// typedefs for the states
// State definitions for use with the query function
typedef enum
{
    BumperIdleState, BumperActiveState
}BumperState_t;

// Public Function Prototypes

bool InitBumperService(uint8_t Priority);
bool PostBumperService(ES_Event_t ThisEvent);
ES_Event_t RunBumperService(ES_Event_t ThisEvent);
BumperState_t QueryBumperService(void);

//This is the event checker for detecting tape
bool Check4Bump(void);

#endif /* BUMPER_SERVICE_H */

