/****************************************************************************

  Header file for template Flat Sate Machine
  based on the Gen2 Events and Services Framework

 ****************************************************************************/

#ifndef FSMFind_Tape
#define FSMFind_Tape

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// typedefs for the states
// State definitions for use with the query function
typedef enum
{
    Waiting2, Searching2
}Find_TapeState_t;

// Public Function Prototypes

bool InitFind_Tape(uint8_t Priority);
bool PostFind_Tape(ES_Event_t ThisEvent);
ES_Event_t RunFind_Tape(ES_Event_t ThisEvent);
Find_TapeState_t QueryFind_Tape(void);

//This is the event checker for detecting tape
bool Check4Tape(void);

#endif /* Find_Tape_H */

