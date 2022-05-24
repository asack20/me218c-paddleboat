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
#include "TugComm.h"
#include "../Propulsion/Propulsion.h"
#include <stdbool.h>
#include <proc/p32mx170f256b.h>

/*----------------------------- Module Defines ----------------------------*/
// MACRO to easily enable/disable print statements
//#define DEBUG_PRINT // define to enable debug message printing
#ifdef DEBUG_PRINT
#define printdebug printf
#else
#define printdebug(fmt, ...) (0)
#endif

#define API_ID_RX16 0x81 // API Identifier for RX 16bit packet 

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/

static void SetupUART(void);
static void ParseNewRXMessage(void);

static void InitializeMode3LEDPins(void);
static void UpdateLEDStatus(uint8_t CurrentIndex);

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
static uint16_t PILOTAddress;

static uint16_t Mode3State;
static bool AutoRefuelInMode3;
static uint8_t Mode3Index;

static const uint8_t RedLEDStateList[4] = {0,1,0,1}; //None, Red, Blue, Purple
static const uint8_t BlueLEDStateList[4] = {0,0,1,1}; //None, Red, Blue, Purple

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
  
  PILOTAddress = 0x2183; // team 3 by default
  
  //Initialize the I/O pins used for controlling Mode3 LEDs
  InitializeMode3LEDPins();
  
  //Initialize Mode3 State as 0 (inactive)
  Mode3State = true;
  AutoRefuelInMode3 = false;
  Mode3Index = 0;
  
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
    //printdebug("Event Checker\r\n");
    if (NewRXBufferState == 1) {
        //In this case new data is available; post an event
        ES_Event_t NewEvent;
        NewEvent.EventType = UART_BYTE_RECEIVED;
        PostXBeeRXSM(NewEvent);
        returnVal = true;
        //printdebug("New Byte Present\r\n");
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
    ES_Event_t PostEvent;
    
    /*
    for (uint8_t i=0; i<15; i++) {
        printdebug("Byte = %x\r\n",RXMessageArray[i]);
    }
    printdebug("Message Complete\r\n");
    */
    
    // Only Accept RX Packet 16bit API identifier (0x81)
    if (RXMessageArray[MSGFRAME_APIIDENTIFIER-1] != API_ID_RX16)
    {
        //printdebug("ParseRX: wrong API ID\r\n");
        return;
    }
    
    TugCommState_t TugCommState = QueryTugComm();
    
    // Validate message ID based on TugCommState
    if (TugCommState == WaitingForPairRequestState)
    {
        // Only accept Request to Pair Messages
        if (RXMessageArray[MSGFRAME_MESSAGEID-1] != XBee_RequestToPair)
        {
            printdebug("ParseRX: Ignoring. MessageID should be RequesttoPair 0x03, but is: %x \r\n", RXMessageArray[MSGFRAME_MESSAGEID-1]);
            return;
        }
        // Set Pilot Address
        PILOTAddress = (RXMessageArray[MSGFRAME_SOURCEADDRESSMSB-1] << 8) + RXMessageArray[MSGFRAME_SOURCEADDRESSLSB-1];
        
        printdebug("ParseRX: Acting on Request to Pair from %x\r\n", PILOTAddress);
        
        // Post message to TUG Comm
        PostEvent.EventType = XBEE_MESSAGE_RECEIVED;
        PostEvent.EventParam = XBee_RequestToPair;
        PostTugComm(PostEvent);
        return;
        
    }
    else
    {
        // Only accept Control Messages
        if (RXMessageArray[MSGFRAME_MESSAGEID-1] != XBee_Control)
        {
            printdebug("ParseRX: Ignoring. MessageID should be Control 0x01, but is: %x \r\n", RXMessageArray[MSGFRAME_MESSAGEID-1]);
            return;
        }
        
        // Ensure Source is Paired Pilot
        uint16_t RxPilot = (RXMessageArray[MSGFRAME_SOURCEADDRESSMSB-1] << 8) + RXMessageArray[MSGFRAME_SOURCEADDRESSLSB-1];
        if (RxPilot != PILOTAddress)
        {
            printdebug("ParseRX: Ignoring Message from unpaired PILOT: %x\r\n", RxPilot);
            return;
        }
        
        printdebug("ParseRX: Acting on Control Message %x\r\n");
        
        // Control Message Validated. Now Act on it.
        // Post message to TUG Comm
        PostEvent.EventType = XBEE_MESSAGE_RECEIVED;
        PostEvent.EventParam = XBee_Control;
        PostTugComm(PostEvent);
        
        // Refuel (Important this is before set thrust)
        // Refuel sent
        if ((RXMessageArray[MSGFRAME_REFUEL-1] != 0) || (AutoRefuelInMode3))
        {
            PostEvent.EventType = PROPULSION_REFUEL;
            PostEvent.EventParam = 0;
            PostPropulsion(PostEvent);
        }
        
        // Set Thrust
        ArcadeControl_t controls;
        controls.X = (int8_t) RXMessageArray[MSGFRAME_X-1];
        controls.Yaw = (int8_t) RXMessageArray[MSGFRAME_YAW-1];
        PostEvent.EventType = PROPULSION_SET_THRUST;
        PostEvent.EventParam = controls.Total;
        PostPropulsion(PostEvent);
        
        // Mode 3
        uint8_t Mode3 = RXMessageArray[MSGFRAME_MODE3-1];
        if (Mode3>0) {
            if (Mode3State == false) {
                //In this case we have that the button was pressed and is now released
                //Toggle lights
                Mode3Index = (Mode3Index+1) % 4; //Cycle back to 0 after 3
                UpdateLEDStatus(Mode3Index); //Update LEDs
            }
            // In this case at least one Mode3 bit is high; Mode3State should be true
            Mode3State = true;
        }
        else {
            //Else Mode3 == 0 so Mode3State should be false
            Mode3State = false;
        }
        
        //Decide what to do based on Mode3Index
        if (Mode3Index>0) {
            //If we got here, Mode3 is active, so give infinite fuel
            AutoRefuelInMode3 = true;
        }
        else {
            //Otherwise, don't autorefuel
            AutoRefuelInMode3 = false;
        }
        
                
    }
    
    
    return;
}

uint16_t GetPILOTAddress(void)
{
    return PILOTAddress;
}
    
static void InitializeMode3LEDPins(void){
    LATBbits.LATB13 = false;
    LATBbits.LATB14 = false;
    PortSetup_ConfigureDigitalOutputs(_Port_B, _Pin_13 | _Pin_14); //RB13 is Red and RB14 is Blue
    return;
}

static void UpdateLEDStatus(uint8_t CurrentIndex){
    LATBbits.LATB13 = RedLEDStateList[CurrentIndex];
    LATBbits.LATB14 = BlueLEDStateList[CurrentIndex];
    return;
}