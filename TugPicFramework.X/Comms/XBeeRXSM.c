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

/*----------------------------- Module Defines ----------------------------*/

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/

static void SetupUART(void);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file
static XBeeRXState_t CurrentState;

// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;

static uint8_t RXMessageArray[15];
static uint8_t ByteIndex;

static bool LastRXBufferState;

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
  LastRXBufferState = U2STAbits.URXDA;
  
  //Enable RX interrupts
  IEC1bits.U2RXIE = 1; //Enable
  
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
    
    if ((LastRXBufferState == 0) && (NewRXBufferState == 1)) {
        //In this case new data is available; post an event
        ES_Event_t NewEvent;
        NewEvent.EventType = UART_BYTE_RECEIVED;
        PostXBeeRXSM(NewEvent);
        returnVal = true;
    }
    LastRXBufferState = NewRXBufferState;
    
    return returnVal;
}

/***************************************************************************
 private functions
 ***************************************************************************/

static void SetupUART(void)
{
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
    U2STAbits.URXISEL = 0b00; //Generate interrupt when receive buffer is nonzero
    
    //Make interrupt priority high for UART
    IPC9bits.U2IP = 0b111;
    
    //Enable interrupts in general
    __builtin_enable_interrupts();
    
    //for debugging
    //U2MODEbits.LPBACK = 1;
    
    U2MODEbits.ON = 1;

    return;
}