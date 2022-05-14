/****************************************************************************
 Module
   XBeeTXSM.c

 Revision
   1.0.1

 Description
 This state machine handles the transmission of UART messages to the XBee module

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 01/15/12 11:12 jec      revisions for Gen2 framework
 11/07/11 11:26 jec      made the queue static
 10/30/11 17:59 jec      fixed references to CurrentEvent in RunTemplateSM()
 10/23/11 18:20 jec      began conversion from SMTemplate.c (02/20/07 rev)
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "XBeeTXSM.h"
#include "PilotFSM.h"
#include "FuelSM.h"
#include "dbprintf.h"
#include "terminal.h"
#include <string.h>

/*----------------------------- Module Defines ----------------------------*/

#define THISXBEE 3

#define MSGFRAME_STARTDELIMITER 1
#define MSGFRAME_LENGTHMSB 2
#define MSGFRAME_LENGTHLSB 3
#define MSGFRAME_APIIDENTIFIER 4
#define MSGFRAME_FRAMEID 5
#define MSGFRAME_DESTINATIONADDRESSMSB 6
#define MSGFRAME_DESTINATIONADDRESSLSB 7
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

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/
static void ConstructNewTXMessage(uint8_t * TXMessage);
static int8_t CalculateX(uint32_t, uint32_t);
static int8_t CalculateY(uint32_t, uint32_t);
static int8_t CalculateYaw(uint32_t, uint32_t);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file
static XBeeTXState_t CurrentState;

// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;

static uint8_t TUGAddress;
static uint32_t LeftThrustVal;
static uint32_t RightThrustVal;
static bool Mode3ToBeActiveOnNextTransmission;
static bool RefuelBitForComms;

static const uint16_t TUGAddresses[8] = {0x2115, 0x2017, 0x2184, 0x2188, 0x2119, 0x2185, 0x2115, 0x2017}; // Last two are just repeating the first two
static const uint16_t PILOTAddresses[8] = {0x2117, 0x2118, 0x2186, 0x2183, 0x2080, 0x2087, 0x2117, 0x2118}; // Last two are just repeating the first two

static uint16_t ThisPILOTAddress;

static uint8_t NewTXMessage[15];

static PilotState_t PilotState;

static XBeeTXMessage_t NewMessageID;

static uint8_t ByteCount;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitXBeeTXSM

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, sets up the initial transition and does any
     other required initialization for this state machine
 Notes

 Author
     J. Edward Carryer, 10/23/11, 18:55
****************************************************************************/
bool InitXBeeTXSM(uint8_t Priority)
{
  ES_Event_t ThisEvent;

  MyPriority = Priority;
  // put us into the Initial PseudoState
  CurrentState = XBeeTXIdleState;
  
  ThisPILOTAddress = PILOTAddresses[THISXBEE];
  
  ByteCount = 0;
  
  return true;
}

/****************************************************************************
 Function
     PostXBeeTXSM

 Parameters
     EF_Event_t ThisEvent , the event to post to the queue

 Returns
     boolean False if the Enqueue operation failed, True otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:25
****************************************************************************/
bool PostXBeeTXSM(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunXBeeTXSM

 Parameters
   ES_Event_t : the event to process

 Returns
   ES_Event_t, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here
 Notes
   uses nested switch/case to implement the machine.
 Author
   J. Edward Carryer, 01/15/12, 15:23
****************************************************************************/
ES_Event_t RunXBeeTXSM(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  
  switch (CurrentState)
  {
    case XBeeTXIdleState:        
    {
      switch (ThisEvent.EventType)
      {
        case XBEE_TRANSMIT_MESSAGE:
        { 
            //Move into the active state
            CurrentState = XBeeTXActiveState;
            
            //Grab all the relevant parameters for the message
            //TUG Address
            TUGAddress = QueryPairingSelectorAddress();
            //Left Thrust Value
            LeftThrustVal = QueryLeftThrustVal();
            //Right Thrust Value
            RightThrustVal = QueryRightThrustVal();
            //Mode 3 Setting
            Mode3ToBeActiveOnNextTransmission = QueryMode3State();
            //Refuel bit
            RefuelBitForComms = QueryRefuelBitForComms();
            
            //MessageID
            PilotState = QueryPilotFSM();
            if (PilotState == AttemptingToPair) {
                NewMessageID = XBee_RequestToPair;
            }
            else if (Paired) {
                NewMessageID = XBee_Control;
            }
            
            ConstructNewTXMessage(NewTXMessage);
            //Now we have a new message to send
            
            //for (uint8_t i=0; i<15; i++) {
            //    DB_printf("Byte = %x\r\n",NewTXMessage[i]);
            //}
            
            ES_Event_t NewEvent;
            NewEvent.EventType = TRANSMIT_BYTE;
            PostXBeeTXSM(NewEvent);
            
            ByteCount = 0;
            //WriteByteToTX(NewTXMessage[ByteCount]);
            ByteCount++;
        }
        break;

        default:
          ;
      }
    }
    break;

    case XBeeTXActiveState:     
    {
      switch (ThisEvent.EventType)
      {
        case TRANSMIT_BYTE:
        { 
            if (ByteCount<15) {
                //WriteByteToTX(NewTXMessage[ByteCount]);
                ByteCount++;
                
                ES_Event_t NewEvent;
                NewEvent.EventType = TRANSMIT_BYTE;
                PostXBeeTXSM(NewEvent);
            }
            else {
                ByteCount = 0;
                CurrentState = XBeeTXIdleState;
            }
        }
        break;

        default:
          ;
      }
    }
    break;
    
    default:
      ;
  } 
  return ReturnEvent;
}

/****************************************************************************
 Function
     QueryXBeeTXSM

 Parameters
     None

 Returns
     XBeeTXState_t The current state of the Template state machine

 Description
     returns the current state of the Template state machine
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:21
****************************************************************************/
XBeeTXState_t QueryXBeeTXSM(void)
{
  return CurrentState;
}

/***************************************************************************
 private functions
 ***************************************************************************/

static void ConstructNewTXMessage(uint8_t * TXMessage)
{
    TXMessage[MSGFRAME_STARTDELIMITER-1]=0x7E;
    TXMessage[MSGFRAME_LENGTHMSB-1]=0x00;
    TXMessage[MSGFRAME_LENGTHLSB-1]=0x0C;
    uint8_t CheckSum = 0;
    CheckSum += (TXMessage[MSGFRAME_APIIDENTIFIER-1]=0x01);
    CheckSum += (TXMessage[MSGFRAME_FRAMEID-1]=0x01);
    CheckSum += (TXMessage[MSGFRAME_DESTINATIONADDRESSMSB-1]=(TUGAddresses[TUGAddress] & 0xFF00) >> 8);
    CheckSum += (TXMessage[MSGFRAME_DESTINATIONADDRESSLSB-1]=(TUGAddresses[TUGAddress] & 0x00FF));
    CheckSum += (TXMessage[MSGFRAME_OPTIONS-1]=0x00);
    CheckSum += (TXMessage[MSGFRAME_MESSAGEID-1]=NewMessageID);
    if (NewMessageID == XBee_RequestToPair) {
        CheckSum += (TXMessage[MSGFRAME_TUGADDRMSB-1]=(TUGAddresses[TUGAddress] & 0xFF00) >> 8);
        CheckSum += (TXMessage[MSGFRAME_TUGADDRLSB-1]=(TUGAddresses[TUGAddress] & 0x00FF));
        CheckSum += (TXMessage[MSGFRAME_PILOTADDRMSB-1]=(ThisPILOTAddress & 0xFF00) >> 8);
        CheckSum += (TXMessage[MSGFRAME_PILOTADDRLSB-1]=(ThisPILOTAddress & 0x00FF));
        CheckSum += (TXMessage[MSGFRAME_ACK-1]=0xAA);
    }
    else if (NewMessageID == XBee_Control) {
        CheckSum += (TXMessage[MSGFRAME_X-1]=CalculateX(LeftThrustVal,RightThrustVal));
        CheckSum += (TXMessage[MSGFRAME_Y-1]=CalculateY(LeftThrustVal,RightThrustVal));
        CheckSum += (TXMessage[MSGFRAME_YAW-1]=CalculateYaw(LeftThrustVal,RightThrustVal));
        CheckSum += (TXMessage[MSGFRAME_REFUEL-1]=RefuelBitForComms);
        CheckSum += (TXMessage[MSGFRAME_MODE3-1]=Mode3ToBeActiveOnNextTransmission);
    }
    else {
        puts("Invalid TX Message is Trying to be Sent\r\n");
    }
    TXMessage[MSGFRAME_CHECKSUM-1]=0xFF-CheckSum;
    
    return;
}

static int8_t CalculateX(uint32_t LTV, uint32_t RTV)
{
    return 0;
}

static int8_t CalculateY(uint32_t LTV, uint32_t RTV)
{
    return 0;
}

static int8_t CalculateYaw(uint32_t LTV, uint32_t RTV)
{
    return 0;
}