/****************************************************************************

  Header file for template Flat Sate Machine
  based on the Gen2 Events and Services Framework

 ****************************************************************************/

#ifndef FSMPilot_H
#define FSMPilot_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// typedefs for the states
// State definitions for use with the query function
typedef enum
{
    AttemptingToPair, Paired
}PilotState_t;

// Public Function Prototypes

bool InitPilotFSM(uint8_t Priority);
bool PostPilotFSM(ES_Event_t ThisEvent);
ES_Event_t RunPilotFSM(ES_Event_t ThisEvent);
PilotState_t QueryPilotFSM(void);

//Event Checkers
bool PairButtonEventChecker(void);
bool Mode3ButtonEventChecker(void);

//Query private variables
uint8_t QueryPairingSelectorAddress(void);
int32_t QueryLeftThrustVal(void);
int32_t QueryRightThrustVal(void);
bool QueryMode3State(void);

#endif /* FSMPilot_H */

