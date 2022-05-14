/****************************************************************************

  Header file for template Flat Sate Machine
  based on the Gen2 Events and Services Framework

 ****************************************************************************/

#ifndef FuelSM_H
#define FuelSM_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// typedefs for the states
// State definitions for use with the query function
typedef enum
{
    FuelRemaining, FuelEmpty
}FuelState_t;

// Public Function Prototypes

bool InitFuelSM(uint8_t Priority);
bool PostFuelSM(ES_Event_t ThisEvent);
ES_Event_t RunFuelSM(ES_Event_t ThisEvent);
FuelState_t QueryFuelSM(void);

//Query for the refuel bit
bool QueryRefuelBitForComms(void);

#endif /* FuelSM_H */

