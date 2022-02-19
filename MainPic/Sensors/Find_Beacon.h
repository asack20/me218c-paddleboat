/****************************************************************************

  Header file for template Flat Sate Machine
  based on the Gen2 Events and Services Framework

 ****************************************************************************/

#ifndef FSMFind_Beacon
#define FSMFind_Beacon

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// typedefs for the states
// State definitions for use with the query function
typedef enum
{
    Waiting1, Searching1
}Find_BeaconState_t;

typedef enum
{
    DetermineTeam, FindKnownFrequency
}SearchType_t;

typedef enum
{
    Unknown, Red, Blue
}TeamIdentity_t;

// Public Function Prototypes

bool InitFind_Beacon(uint8_t Priority);
bool PostFind_Beacon(ES_Event_t ThisEvent);
ES_Event_t RunFind_Beacon(ES_Event_t ThisEvent);
Find_BeaconState_t QueryFind_Beacon(void);
TeamIdentity_t QueryTeamIdentity(void);

#endif /* Find_Beacon_H */

