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
#include "../HALs/PIC32PortHAL.h"
#include "../Propulsion/Propulsion.h"
#include "TugComm.h"
#include <string.h>
#include <xc.h>
#include <sys/attribs.h>

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
static void ConfigureUART(void);
static void TurnOnTXInterrupts(void);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match that of enum in header file
static volatile XBeeTXState_t CurrentState;

// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;

static TugCommState_t TugState;
static uint8_t FuelLevel;

static const uint16_t TUGAddresses[8] = {0x2115, 0x2017, 0x2184, 0x2188, 0x2119, 0x2185, 0x2115, 0x2017}; // Last two are just repeating the first two

static uint16_t PILOTAddress;
static uint16_t ThisTUGAddress;


static uint8_t NewTXMessage[15];

static XBeeTXMessage_t NewMessageID;

static volatile uint8_t ByteCount;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitXBeeTXSM

 Parameters
     uint8_t : the priority of this service

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
  
  // Set default addresses
  ThisTUGAddress = TUGAddresses[THISXBEE];
  PILOTAddress = 0x2183; // team 3 by default
  
  ByteCount = 0;
  
  // Configure UART
  ConfigureUART();
  
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
            //TUG Address = ThisTUGAddress();
            //Fuel Level Value
            FuelLevel = Propulsion_GetFuelLevel();
            
            //MessageID
            TugState = QueryTugComm();
            if (TugState == WaitingForControlPacketState) {
                NewMessageID = XBee_PairingAcknowledged;
            }
            else if (TugState == PairedState) {
                NewMessageID = XBee_Status;
            }
            
            ConstructNewTXMessage(NewTXMessage);
            //Now we have a new message to send
            
            //for (uint8_t i=0; i<15; i++) {
            //    printf("Byte = %x\r\n",NewTXMessage[i]);
            //}
            
            ES_Event_t NewEvent;
            NewEvent.EventType = TRANSMIT_BYTE;
            PostXBeeTXSM(NewEvent);
            
            ByteCount = 0;
            TurnOnTXInterrupts();
            //WriteByteToTX(NewTXMessage[ByteCount]);
            
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
            //puts("A new transmission is in progress\r\n");
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

void SetPILOTAddress(uint16_t Address)
{
    PILOTAddress = Address;
}

/***************************************************************************
 private functions
 ***************************************************************************/

static void ConstructNewTXMessage(uint8_t * TXMessage)
{
    TXMessage[MSGFRAME_STARTDELIMITER-1]=0x7E;
    TXMessage[MSGFRAME_LENGTHMSB-1]=0x00;
    TXMessage[MSGFRAME_LENGTHLSB-1]=0x0B;
    uint8_t CheckSum = 0;
    CheckSum += (TXMessage[MSGFRAME_APIIDENTIFIER-1]=0x01);
    CheckSum += (TXMessage[MSGFRAME_FRAMEID-1]=0x01);
    CheckSum += (TXMessage[MSGFRAME_DESTINATIONADDRESSMSB-1]=(PILOTAddress & 0xFF00) >> 8);
    CheckSum += (TXMessage[MSGFRAME_DESTINATIONADDRESSLSB-1]=(PILOTAddress & 0x00FF));
    CheckSum += (TXMessage[MSGFRAME_OPTIONS-1]=0x00);
    CheckSum += (TXMessage[MSGFRAME_MESSAGEID-1]=NewMessageID);
    if (NewMessageID == XBee_PairingAcknowledged) {
        CheckSum += (TXMessage[MSGFRAME_TUGADDRMSB-1]=(ThisTUGAddress & 0xFF00) >> 8);
        CheckSum += (TXMessage[MSGFRAME_TUGADDRLSB-1]=(ThisTUGAddress & 0x00FF));
        CheckSum += (TXMessage[MSGFRAME_PILOTADDRMSB-1]=(PILOTAddress & 0xFF00) >> 8);
        CheckSum += (TXMessage[MSGFRAME_PILOTADDRLSB-1]=(PILOTAddress & 0x00FF));
        CheckSum += (TXMessage[MSGFRAME_ACK-1]=0x55);
    }
    else if (NewMessageID == XBee_Status) {
        CheckSum += (TXMessage[MSGFRAME_FUELLEVEL-1]=FuelLevel);
        CheckSum += (TXMessage[MSGFRAME_Y-1]=0);
        CheckSum += (TXMessage[MSGFRAME_YAW-1]=0);
        CheckSum += (TXMessage[MSGFRAME_REFUEL-1]=0);
        CheckSum += (TXMessage[MSGFRAME_MODE3-1]=0);
    }
    else {
        puts("Invalid TX Message is Trying to be Sent\r\n");
    }
    TXMessage[MSGFRAME_CHECKSUM-1]=0xFF-CheckSum;
    
    return;
}

static void TurnOnTXInterrupts(void)
{
    IEC1bits.U2TXIE = 1; //Enable
    return;
}

void __ISR(_UART_2_VECTOR, IPL7SOFT) TXBufferEmptyInterruptHandler(void)
{
    //Put something in the buffer
    U2TXREG = NewTXMessage[ByteCount];
    ByteCount++;
    if (ByteCount < 15) {
        //Do nothing
    }
    else {
        //Disable interrupt and move back to the inactive state
        IEC1bits.U2TXIE = 0; //Disable
        CurrentState = XBeeTXIdleState;
        ByteCount = 0;
    }
    //Clear interrupt flag
    IFS1CLR = _IFS1_U2TXIF_MASK;
}

static void ConfigureUART(void)
{
    //Configure Digital Inputs for Port A
    //Pin 1 is RX for UART2
    PortSetup_ConfigureDigitalInputs(_Port_A, _Pin_1);
    
 
    //Configure Digital Outputs for Port B
    //Pin 10 is TX for UART2
    PortSetup_ConfigureDigitalOutputs(_Port_B, _Pin_10);
    
    // Turn off UART2
    U2MODE = 0;
    U2STA = 0;
            
    U2MODEbits.ON = 0; 
    // Configure for 9600 baud assuming 20 MHz PBCLK
    U2MODEbits.PDSEL = 0;
    U2MODEbits.STSEL = 0;
    U2MODEbits.BRGH = 0;
    U2BRG = 129;
    
    //Set up pin configurations for UART
    U2RXR = 0b0000;
    RPB10R = 0b0010;
    
    U2STAbits.UTXEN = 1;
    U2STAbits.URXEN = 1;
    
    U2STAbits.UTXISEL = 0b10; //Generate interrupt when transmit buffer is empty
    
    //Make interrupt priority high for UART
    IPC9bits.U2IP = 0b111;
    
    //Enable interrupts in general
    __builtin_enable_interrupts();
    
    //for debugging
    //U2MODEbits.LPBACK = 1;
    
    U2MODEbits.ON = 1;
    return;
}
