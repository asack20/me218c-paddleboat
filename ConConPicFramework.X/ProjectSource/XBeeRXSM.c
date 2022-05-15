/****************************************************************************
 Module
   XBeeRXSM.c

 Revision
   1.0.1

 Description
 This module implements the state machine for the XBee RX communications

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
#include "XBeeRXSM.h"
#include "XBeeTXSM.h"
#include "PilotFSM.h"
#include "../HALs/PIC32PortHAL.h"
#include "terminal.h"
#include "dbprintf.h"
#include <string.h>

/*----------------------------- Module Defines ----------------------------*/

#define RXMSGFRAME_STARTDELIMITER 1
#define RXMSGFRAME_LENGTHMSB 2
#define RXMSGFRAME_LENGTHLSB 3
#define RXMSGFRAME_APIIDENTIFIER 4
#define RXMSGFRAME_SOURCEADDRESSMSB 5
#define RXMSGFRAME_SOURCEADDRESSLSB 6
#define RXMSGFRAME_RSSI 7
#define RXMSGFRAME_OPTIONS 8
#define RXMSGFRAME_MESSAGEID 9
#define RXMSGFRAME_TUGADDRMSB 10
#define RXMSGFRAME_X 10
#define RXMSGFRAME_FUELLEVEL 10
#define RXMSGFRAME_TUGADDRLSB 11
#define RXMSGFRAME_Y 11
#define RXMSGFRAME_PILOTADDRMSB 12
#define RXMSGFRAME_YAW 12
#define RXMSGFRAME_PILOTADDRLSB 13
#define RXMSGFRAME_REFUEL 13
#define RXMSGFRAME_ACK 14
#define RXMSGFRAME_MODE3 14
#define RXMSGFRAME_CHECKSUM 15

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/

static void SetupUART(void);
static void ParseNewRXMessage(void);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file
static XBeeRXState_t CurrentState;

// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;

static uint8_t RXMessageArray[15];
static uint8_t ByteIndex;

static bool LastRXBufferState;

static uint16_t messageLength;

static uint8_t FuelLevel;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitXBeeRXSM

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
bool InitXBeeRXSM(uint8_t Priority)
{

  MyPriority = Priority;
  // put us into the Initial PseudoState
  CurrentState = XBeeRXIdleState;
  // post the initial transition event
  
  //Start pointing to index 0
  ByteIndex = 0;
  
  //Update last RX Buffer State
  LastRXBufferState = 0;
  
  //Initialize with max fuel to avoid refuel upon powerup
  FuelLevel = 0xFF;
  
  //puts("Yooooooooo\r\n");
  
  return true;
}

/****************************************************************************
 Function
     PostXBeeRXSM

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
bool PostXBeeRXSM(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunXBeeRXSM

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
ES_Event_t RunXBeeRXSM(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

  switch (CurrentState)
  {
    case XBeeRXIdleState:      
    {
      switch (ThisEvent.EventType)
      {
        case UART_BYTE_RECEIVED: 
        {  
            //Reset the index to 0
            ByteIndex = 0;
            
            uint8_t tempVal;
            
            //Get the byte from the UART2 RX Buffer
            tempVal = U2RXREG;
            
            //If the byte is valid as a Start Delimiter (0x7E) then proceed.  Otherwise ignore it.
            if (tempVal == 0x7E) {
                //Save in the array
                RXMessageArray[ByteIndex]=tempVal;
                //Increment the index
                ByteIndex++;
                //Move into the next state so we process the whole message
                CurrentState = XBeeRXPrologueState;
            }
        }
        break;

        default:
          ;
      } 
    }
    break;

    case XBeeRXPrologueState:      
    {
      switch (ThisEvent.EventType)
      {
        case UART_BYTE_RECEIVED: 
        {  
            uint8_t tempVal2;
            tempVal2 = U2RXREG;
            //Save in the array
            RXMessageArray[ByteIndex]=tempVal2;
            //Increment the index
            ByteIndex++;
            
            //if the ByteIndex is now 3, then the last thing we received was the length of the message
            if (ByteIndex == 3){
                messageLength = (RXMessageArray[ByteIndex-2]<<8) + (RXMessageArray[ByteIndex-1]);
                //printf("Message Length is %x",messageLength);
                //We need to start counting through the message length now
                ByteIndex = 0;
                //Go to the next state
                CurrentState = XBeeRXFrameDataState;
            }
                
        }
        break;

        
        default:
          ;
      } 
    }
    break;
    
    case XBeeRXFrameDataState:      
    {
      switch (ThisEvent.EventType)
      {
        case UART_BYTE_RECEIVED: 
        {  
            //Save in the array
            RXMessageArray[ByteIndex+3]=U2RXREG;
            //Increment the index
            ByteIndex++;
            
            //If ByteIndex is one more than the message length (we want to count the checksum), move on
            if (ByteIndex > messageLength) {
                //Go back to being idle
                CurrentState = XBeeRXIdleState;
                
                //Call function to handle new message
                ParseNewRXMessage();
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
     QueryXBeeRXSM

 Parameters
     None

 Returns
     XBeeRXState_t The current state of the XBeeRX state machine

 Description
     returns the current state of the XBeeRX state machine
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:21
****************************************************************************/
XBeeRXState_t QueryXBeeRXSM(void)
{
  return CurrentState;
}

bool IsRXBufferNonempty(void)
{
    bool returnVal;
    returnVal = false;
    bool NewRXBufferState;
    NewRXBufferState = U2STAbits.URXDA;
    //puts("Event Checker\r\n");
    if (NewRXBufferState == 1) {
        //In this case new data is available; post an event
        ES_Event_t NewEvent;
        NewEvent.EventType = UART_BYTE_RECEIVED;
        PostXBeeRXSM(NewEvent);
        returnVal = true;
        //puts("New Byte Present\r\n");
    }
    LastRXBufferState = NewRXBufferState;
    
    return returnVal;
}

uint8_t QueryFuelLevel(void)
{
    return FuelLevel;
}

/***************************************************************************
 private functions
 ***************************************************************************/

static void ParseNewRXMessage(void)
{
    //for (uint8_t i=0; i<15; i++) {
    //    DB_printf("Byte = %x\r\n",RXMessageArray[i]);
    //}
    //puts("Message Complete\r\n");
    
    //We only care about this message if it's of the type RX Packet:  16-bit Address,
    //indicated by the API Identifier 0x81
    if (RXMessageArray[RXMSGFRAME_APIIDENTIFIER-1] == 0x81) {
        uint8_t ChecksumTest;
        ChecksumTest = 0;

        //We're only expecting two types of messages - ignore all others
        //First type:  Pairing Acknowledgement
        if (RXMessageArray[RXMSGFRAME_MESSAGEID-1] == XBee_PairingAcknowledged) {
            //Only proceed if checksum is correct
            ChecksumTest += RXMessageArray[RXMSGFRAME_APIIDENTIFIER-1];
            ChecksumTest += RXMessageArray[RXMSGFRAME_SOURCEADDRESSMSB-1];
            ChecksumTest += RXMessageArray[RXMSGFRAME_SOURCEADDRESSLSB-1];
            ChecksumTest += RXMessageArray[RXMSGFRAME_RSSI-1];
            ChecksumTest += RXMessageArray[RXMSGFRAME_OPTIONS-1];
            ChecksumTest += RXMessageArray[RXMSGFRAME_MESSAGEID-1];
            ChecksumTest += RXMessageArray[RXMSGFRAME_TUGADDRMSB-1];
            ChecksumTest += RXMessageArray[RXMSGFRAME_TUGADDRLSB-1];
            ChecksumTest += RXMessageArray[RXMSGFRAME_PILOTADDRMSB-1];
            ChecksumTest += RXMessageArray[RXMSGFRAME_PILOTADDRLSB-1];
            ChecksumTest += RXMessageArray[RXMSGFRAME_ACK-1];
            ChecksumTest += RXMessageArray[RXMSGFRAME_CHECKSUM-1];
            if (ChecksumTest == 0xFF) {
                //In this case, we're good -- post that a pairing acknowledgement occurred
                ES_Event_t NewEvent;
                NewEvent.EventType = ACK_RECEIVED;
                PostPilotFSM(NewEvent);
            }
        }
        //Second type:  Status while paired
        else if (RXMessageArray[RXMSGFRAME_MESSAGEID-1] == XBee_Status) {
            //Only proceed if checksum is correct
            ChecksumTest += RXMessageArray[RXMSGFRAME_APIIDENTIFIER-1];
            ChecksumTest += RXMessageArray[RXMSGFRAME_SOURCEADDRESSMSB-1];
            ChecksumTest += RXMessageArray[RXMSGFRAME_SOURCEADDRESSLSB-1];
            ChecksumTest += RXMessageArray[RXMSGFRAME_RSSI-1];
            ChecksumTest += RXMessageArray[RXMSGFRAME_OPTIONS-1];
            ChecksumTest += RXMessageArray[RXMSGFRAME_MESSAGEID-1];
            ChecksumTest += RXMessageArray[RXMSGFRAME_FUELLEVEL-1];
            ChecksumTest += RXMessageArray[11-1];
            ChecksumTest += RXMessageArray[12-1];
            ChecksumTest += RXMessageArray[13-1];
            ChecksumTest += RXMessageArray[14-1];
            ChecksumTest += RXMessageArray[RXMSGFRAME_CHECKSUM-1];
            if (ChecksumTest == 0xFF) {
                //In this case, we're good -- update fuel level
                FuelLevel = RXMessageArray[RXMSGFRAME_FUELLEVEL-1];
                ES_Event_t NewEvent;
                NewEvent.EventType = VALID_STATUS_RECEIVED;
                PostPilotFSM(NewEvent);
                //DB_printf("Fuel Level = %d\r\n",FuelLevel);
            }
        }
    }
    
    return;
}