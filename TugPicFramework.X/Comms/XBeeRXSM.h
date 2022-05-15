/****************************************************************************

  Header file for template Flat Sate Machine
  based on the Gen2 Events and Services Framework

 ****************************************************************************/

#ifndef XBeeRXSM_H
#define XBeeRXSM_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */


// Byte Definitions:
// These are in RX because TX already includes RX
#define MSGFRAME_STARTDELIMITER 1
#define MSGFRAME_LENGTHMSB 2
#define MSGFRAME_LENGTHLSB 3
#define MSGFRAME_APIIDENTIFIER 4

// For TX
#define MSGFRAME_FRAMEID 5
#define MSGFRAME_DESTINATIONADDRESSMSB 6
#define MSGFRAME_DESTINATIONADDRESSLSB 7

// For RX
#define MSGFRAME_SOURCEADDRESSMSB 5
#define MSGFRAME_SOURCEADDRESSLSB 6
#define MSGFRAME_RSSI 7

#define MSGFRAME_OPTIONS 8
#define MSGFRAME_MESSAGEID 9
#define MSGFRAME_TUGADDRMSB 10
#define MSGFRAME_X 10
#define MSGFRAME_FUELLEVEL 10
#define MSGFRAME_TUGADDRLSB 11
#define MSGFRAME_Y 11
#define MSGFRAME_PILOTADDRMSB 12
#define MSGFRAME_YAW 12
#define MSGFRAME_PILOTADDRLSB 13
#define MSGFRAME_REFUEL 13
#define MSGFRAME_ACK 14
#define MSGFRAME_MODE3 14
#define MSGFRAME_CHECKSUM 15

typedef enum
{
    XBee_Control=1, XBee_Status=2, XBee_RequestToPair=3, XBee_PairingAcknowledged=4
}XBeeTXMessage_t;

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

//Event checker for RX buffer nonempty in UART2
bool IsRXBufferNonempty(void);
uint16_t GetPILOTAddress(void);

#endif /* XBeeRXSM_H */

