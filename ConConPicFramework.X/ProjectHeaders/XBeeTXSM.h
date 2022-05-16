/****************************************************************************

  Header file for template Flat Sate Machine
  based on the Gen2 Events and Services Framework

 ****************************************************************************/

#ifndef XBeeTXSM_H
#define XBeeTXSM_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// typedefs for the states
// State definitions for use with the query function
typedef enum
{
    XBeeTXIdleState, XBeeTXActiveState
}XBeeTXState_t;

typedef enum
{
    XBee_Control=1, XBee_Status=2, XBee_RequestToPair=3, XBee_PairingAcknowledged=4
}XBeeTXMessage_t;
// Public Function Prototypes

bool InitXBeeTXSM(uint8_t Priority);
bool PostXBeeTXSM(ES_Event_t ThisEvent);
ES_Event_t RunXBeeTXSM(ES_Event_t ThisEvent);
XBeeTXState_t QueryXBeeTXSM(void);

#endif /* XBeeTXSM_H */

