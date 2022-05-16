/****************************************************************************

  Header file for Display Flat Sate Machine
  based on the Gen2 Events and Services Framework

 ****************************************************************************/

#ifndef BRAIDSERVICE_H
#define BRAIDSERVICE_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */
#include "terminal.h"
#include "dbprintf.h"

// typedefs for the states
// State definitions for use with the query function
typedef enum
{
  WaitState, RefuelState
}BraidState_t;

// Public Function Prototypes

bool InitBraidService(uint8_t Priority);
bool PostBraidService(ES_Event_t ThisEvent);
ES_Event_t RunBraidService(ES_Event_t ThisEvent);
BraidState_t QueryBraidService(void);

#endif /* BRAIDSERVICE_H */

