/****************************************************************************

  Header file for template Flat Sate Machine
  based on the Gen2 Events and Services Framework

 ****************************************************************************/

#ifndef XBeeRXSM_H
#define XBeeRXSM_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// typedefs for the states
// State definitions for use with the query function
typedef enum
{
    XBeeRXIdleState, XBeeRXPrologueState, XBeeRXFrameDataState
}XBeeRXState_t;

// Public Function Prototypes

bool InitXBeeRXSM(uint8_t Priority);
bool PostXBeeRXSM(ES_Event_t ThisEvent);
ES_Event_t RunXBeeRXSM(ES_Event_t ThisEvent);
XBeeRXState_t QueryXBeeRXSM(void);

uint8_t QueryFuelLevel(void);

//Event checker for RX buffer nonempty in UART2
bool IsRXBufferNonempty(void);

#endif /* XBeeRXSM_H */

