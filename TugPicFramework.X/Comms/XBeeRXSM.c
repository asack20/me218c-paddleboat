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
#include "../HALs/PIC32PortHAL.h"

/*----------------------------- Module Defines ----------------------------*/

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
  
  //Set up UART for RX
  SetupUART();
  
  //Update last RX Buffer State
  LastRXBufferState = 0;
  
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

/***************************************************************************
 private functions
 ***************************************************************************/

static void SetupUART(void)
{
    // Make TX digital output
    PortSetup_ConfigureDigitalOutputs(_Port_B, _Pin_10);
    // Make RX digital input
    PortSetup_ConfigureDigitalInputs(_Port_A, _Pin_1);
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

static void ParseNewRXMessage(void)
{
    for (uint8_t i=0; i<15; i++) {
        printf("Byte = %x\r\n",RXMessageArray[i]);
    }
    puts("Message Complete\r\n");
    return;
}