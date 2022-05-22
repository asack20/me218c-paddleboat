/****************************************************************************
 Module
   PilotFSM.c

 Revision
   1.0.1

 Description
 This FSM controls the sending and receiving of messages over XBee in accordance
 * with the classwide communications protocol

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 05/10/22 18:15 rnb      define module for XBee comms
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
#include "PilotFSM.h"
#include "XBeeTXSM.h"
#include "../HALs/PIC32PortHAL.h"
#include "../HALs/PIC32_AD_Lib.h"
#include <stdbool.h>

/*----------------------------- Module Defines ----------------------------*/

#define COMMS_TIMEOUT 3000 //Three Seconds
#define FIVE_SEC 5000
#define ONE_SEC 1000
#define MODE3DEBOUNCETIMERDURATION 2000
#define ONE_TENTH_SEC 100
#define ONE_FIFTH_SEC 200// For 5Hz communications
#define PAIRBUTTONBIT PORTAbits.RA4
#define MODE3BUTTONBIT PORTBbits.RB9

#define LEFTTHRUSTANALOGPIN 1<<12
#define RIGHTTHRUSTANALOGPIN 1<<11

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/
static void ConfigureUARTforXBee(void);
static void ConfigureSPI(void);
static void ConfigureIOPins(void);
static void LatchAddress(void);
static void RequestToPair(void);
static void SendControl(void);
static void StartCommsTimer(void);
static void StartInactivityTimer(void);
static void StopInactivityTimer(void);
static void ResetInactivityTimer(void);
static bool UpdatePairButtonState(void);
static bool UpdateMode3ButtonState(void);
static void TurnOnTryingToPairLED(void);
static void TurnOffTryingToPairLED(void);
static void TurnOnPairedLED(void);
static void TurnOffPairedLED(void);
static void ToggleCommsLED(void);
static void UpdateThrustVals(void);

static int32_t CalibrateLeft(uint32_t);
static int32_t CalibrateRight(uint32_t);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file
static PilotState_t CurrentState;

// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;

static bool PairButtonState;
static bool PairButtonEventCheckerActive;

static bool Mode3ButtonState;
static bool Mode3ButtonEventCheckerActive;

static bool LatchAddressMSB;
static bool LatchAddressMidBit;
static bool LatchAddressLSB;

static bool CommsLEDStatus;

static uint32_t ThrustADCValues[2];
static uint32_t LeftThrustVal;
static uint32_t RightThrustVal;

static bool Mode3ToBeActiveOnNextTransmission;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitPilotFSM

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
bool InitPilotFSM(uint8_t Priority)
{
  ES_Event_t ThisEvent;

  MyPriority = Priority;
  // put us into the Initial PseudoState
  CurrentState = AttemptingToPair;
  
  //Configure IO Pins
  ConfigureIOPins();
  
  //Configure SPI
  ConfigureSPI();
  
  //Configure UART for XBee Communications
  ConfigureUARTforXBee();
  
  //Start Comms Timer
  StartCommsTimer();
  
  //Update PairButtonState variable
  PairButtonState = UpdatePairButtonState();
  //Start out with event checker active
  PairButtonEventCheckerActive = true;
  
  //Update PairButtonState variable
  Mode3ButtonState = UpdateMode3ButtonState();
  //Start out with event checker active
  Mode3ButtonEventCheckerActive = true;
  
  //Turn on Trying To Pair LED
  TurnOnTryingToPairLED();
  //Turn off Paired LED
  TurnOffPairedLED();
  
  //Turn off Comms LED
  CommsLEDStatus = true;
  ToggleCommsLED();
  
  //Set up ADC for autoscanning
  ADC_ConfigAutoScan((LEFTTHRUSTANALOGPIN | RIGHTTHRUSTANALOGPIN), 2);
  
  //Disable Mode3 upon init
  Mode3ToBeActiveOnNextTransmission = false;
  
  //Initialize Selector Bits for TUG Address
    LatchAddressMSB = 0;
    LatchAddressMidBit = 1;
    LatchAddressLSB = 1;
  
  return true;
}

/****************************************************************************
 Function
     PostPilotFSM

 Parameters
     EF_Event_t ThisEvent , the event to post to the queue

 Returns
     boolean False if the Enqueue operation failed, True otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:25
****    ************************************************************************/
bool PostPilotFSM(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunPilotFSM

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
ES_Event_t RunPilotFSM(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

  switch (CurrentState)
  {
    case AttemptingToPair:        // If current state is initial Psedudo State
    {
      switch (ThisEvent.EventType)
      {
          case PAIR_BUTTON_PRESSED:
          {
              //puts("PilotFSM received PAIR_BUTTON_PRESSED Event in AttemptingToPair State\r\n");
              LatchAddress();
          }
          break;
          
          case ES_TIMEOUT:
          {
              //puts("PilotFSM received ES_TIMEOUT Event in AttemptingToPair State\r\n");
              if (ThisEvent.EventParam == COMMSTIMER) {
                  //puts("ES_TIMEOUT Event is of type CommsTimer\r\n");
                  UpdateThrustVals();
                  StartCommsTimer();
                  RequestToPair();
                  ToggleCommsLED();
              }
              if (ThisEvent.EventParam == PAIRBUTTONDEBOUNCETIMER) {
                  //puts("ES_TIMEOUT Event is of type PairButtonDebounceTimer\r\n");
                  PairButtonEventCheckerActive = true;
              }
              if (ThisEvent.EventParam == MODE3BUTTONDEBOUNCETIMER) {
                  Mode3ButtonEventCheckerActive = true;
              }
          }
          break;
          
          case ACK_RECEIVED:
          {
              //puts("PilotFSM received ACK_RECEIVED Event in AttemptingToPair State\r\n");
              StartInactivityTimer();
              CurrentState = Paired;
              TurnOffTryingToPairLED();
              TurnOnPairedLED();
          }
          break;
          
          default:
              ;
      }
    }
    break;

    case Paired:       
    {
      switch (ThisEvent.EventType)
      {
        case VALID_STATUS_RECEIVED:  
        { 
            //puts("PilotFSM received VALID_STATUS_RECEIVED Event in Paired State\r\n");
            ResetInactivityTimer();
        }
        break;
        
        case ES_TIMEOUT:  
        { 
            //puts("PilotFSM received ES_TIMEOUT Event in Paired State\r\n");
            if (ThisEvent.EventParam == COMMSTIMER) {
                //puts("ES_TIMEOUT Event is of type CommsTimer\r\n");
                UpdateThrustVals();
                StartCommsTimer();
                SendControl();
                ToggleCommsLED();
            }
            if (ThisEvent.EventParam == INACTIVITYTIMER){
                //puts("ES_TIMEOUT Event is of type InactivityTimer\r\n");
                LatchAddress();
                CurrentState = AttemptingToPair;
                StopInactivityTimer();
                TurnOffPairedLED();
                TurnOnTryingToPairLED();
            }
            if (ThisEvent.EventParam == PAIRBUTTONDEBOUNCETIMER) {
                  //puts("ES_TIMEOUT Event is of type PairButtonDebounceTimer\r\n");
                  PairButtonEventCheckerActive = true;
            }
            if (ThisEvent.EventParam == MODE3BUTTONDEBOUNCETIMER) {
                  Mode3ButtonEventCheckerActive = true;
                  Mode3ToBeActiveOnNextTransmission = false; //NOT SURE IF THIS IS WHAT WE WANT TO DO
                //in current state, this would only send 1 message with mode 3
                //information for each time the button is pressed.  This is
                //event driven.  Instead, we could forego the mode 3 button event
                //checker entirely and just send the state of the mode 3 button
                //in the message to the tug.
            }
        }
        break;
        
        case PAIR_BUTTON_PRESSED:  
        { 
            //puts("PilotFSM received PAIR_BUTTON_PRESSED Event in Paired State\r\n");
            LatchAddress();
            CurrentState = AttemptingToPair;
            StopInactivityTimer();
            TurnOffPairedLED();
            TurnOnTryingToPairLED();
        }
        break;
        
        case MODE3_BUTTON_PRESSED:  
        { 
            puts("PilotFSM received MODE3_BUTTON_PRESSED Event in Paired State\r\n");
            Mode3ToBeActiveOnNextTransmission = true;
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
     QueryPilotFSM

 Parameters
     None

 Returns
     PilotState_t The current state of the PilotFSM state machine

 Description
     returns the current state of the PilotFSM state machine
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:21
****************************************************************************/
PilotState_t QueryPilotFSM(void)
{
  return CurrentState;
}

//Event checker for Pairing Button
bool PairButtonEventChecker(void)
{
    bool returnVal;
    bool oldVal;
    bool newVal;
    
    returnVal = false;
    
    oldVal = PairButtonState;
    newVal = UpdatePairButtonState();
    if (((oldVal == false) && (newVal == true)) && PairButtonEventCheckerActive) {
        //Then rising edge - button pressed
        puts("Pair Button Pressed\r\n");
        returnVal = true;
        PairButtonEventCheckerActive = false; // Wait for timeout to check again for software debouncing
        ES_Timer_InitTimer(PAIRBUTTONDEBOUNCETIMER, ONE_TENTH_SEC);
        
        ES_Event_t ThisEvent;
        ThisEvent.EventType   = PAIR_BUTTON_PRESSED;
        ES_PostAll(ThisEvent);
    }
    PairButtonState = newVal;
    return returnVal;
}

bool Mode3ButtonEventChecker(void)
{
    bool returnVal;
    bool oldVal;
    bool newVal;
    
    returnVal = false;
    
    oldVal = Mode3ButtonState;
    newVal = UpdateMode3ButtonState();
    if (((oldVal == false) && (newVal == true)) && Mode3ButtonEventCheckerActive) {
        //Then rising edge - button pressed
        puts("Mode 3 Button Pressed\r\n");
        returnVal = true;
        Mode3ButtonEventCheckerActive = false; // Wait for timeout to check again for software debouncing
        ES_Timer_InitTimer(MODE3BUTTONDEBOUNCETIMER, MODE3DEBOUNCETIMERDURATION);
        
        ES_Event_t ThisEvent;
        ThisEvent.EventType   = MODE3_BUTTON_PRESSED;
        ES_PostAll(ThisEvent);
    }
    Mode3ButtonState = newVal;
    return returnVal;
}

//Allow terminal probing of private variables
uint8_t QueryPairingSelectorAddress(void)
{
    return (LatchAddressMSB << 2) + (LatchAddressMidBit << 1) + (LatchAddressLSB << 0);
}

int32_t QueryLeftThrustVal(void)
{
    
    return CalibrateLeft(LeftThrustVal);
}

int32_t QueryRightThrustVal(void)
{
    return CalibrateRight(RightThrustVal);
}

bool QueryMode3State(void)
{
    return Mode3ToBeActiveOnNextTransmission;
}

/***************************************************************************
 private functions
 ***************************************************************************/

static void ConfigureUARTforXBee(void)
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
    
    //Make interrupt priority high for UART
    IPC9bits.U2IP = 0b111;
    
    //Enable interrupts in general
    __builtin_enable_interrupts();
    
    //for debugging
    //U2MODEbits.LPBACK = 1;
    
    U2MODEbits.ON = 1;
    return;
}

static void ConfigureSPI(void)
{
    return;
}

static void ConfigureIOPins(void)
{
    //Configure Digital Inputs for Port A
    //Pin 1 is RX for UART2
    //Pin 2 is input for Pairing Selector 1/3
    //Pin 3 is input for Pairing Selector 2/3
    //Pin 4 is input for Pair Button
    PortSetup_ConfigureDigitalInputs(_Port_A, _Pin_1 | _Pin_2 | _Pin_3 | _Pin_4);
    
    //Configure Digital Inputs for Port B
    //Pin 4 is input for Pairing Selector 3/3
    //Pin 5 is SDI for SPI1
    //Pin 9 is input for Mode 3
    PortSetup_ConfigureDigitalInputs(_Port_B, _Pin_4 | _Pin_5 | _Pin_9);
    
    //Configure Digital Outputs for Port A
    //Pin 0 is SS output for SPI1
    PortSetup_ConfigureDigitalOutputs(_Port_A, _Pin_0);
    
    //Configure Digital Outputs for Port B
    //Pin 8 is SDO for SPI1
    //Pin 10 is TX for UART2
    //Pin 11 is output for Trying To Pair LED
    //Pin 14 is SCK for SPI1
    //Pin 15 is output for Paired LED
    PortSetup_ConfigureDigitalOutputs(_Port_B, _Pin_2 | _Pin_8 | _Pin_10 | _Pin_11 | _Pin_14 | _Pin_15);
    
    //Configure Analog Inputs for Port B
    //Pin 12 is analog input for left thrust
    //Pin 13 is analog input for right thrust
    PortSetup_ConfigureAnalogInputs(_Port_B, _Pin_12 | _Pin_13);
    
    //Add weak pull ups to the button pins
    PortSetup_ConfigurePullUps(_Port_A, _Pin_4);
    PortSetup_ConfigurePullUps(_Port_B, _Pin_9);
    
    return;
}

static void LatchAddress(void)
{
    LatchAddressMSB = PORTBbits.RB4;
    LatchAddressMidBit = PORTAbits.RA3;
    LatchAddressLSB = PORTAbits.RA2;
    return;
}

static void RequestToPair(void)
{
    ES_Event_t NewEvent;
    NewEvent.EventType = XBEE_TRANSMIT_MESSAGE;
    PostXBeeTXSM(NewEvent);
    return;
}

static void SendControl(void)
{
    ES_Event_t NewEvent;
    NewEvent.EventType = XBEE_TRANSMIT_MESSAGE;
    PostXBeeTXSM(NewEvent);
    return;
}

static void StartCommsTimer(void)
{
    ES_Timer_InitTimer(COMMSTIMER, ONE_FIFTH_SEC);
    return;
}

static void StartInactivityTimer(void)
{
    ES_Timer_InitTimer(INACTIVITYTIMER, COMMS_TIMEOUT);
    return;
}

static void StopInactivityTimer(void)
{
    ES_Timer_StopTimer(INACTIVITYTIMER);
    return;
}

static void ResetInactivityTimer(void)
{
    ES_Timer_StopTimer(INACTIVITYTIMER);
    ES_Timer_InitTimer(INACTIVITYTIMER, COMMS_TIMEOUT);
    return;
}

static bool UpdatePairButtonState(void)
{
    return PAIRBUTTONBIT;
}

static bool UpdateMode3ButtonState(void)
{
    return MODE3BUTTONBIT;
}

static void TurnOnTryingToPairLED(void)
{
    LATBbits.LATB11 = true;
    return;
}

static void TurnOffTryingToPairLED(void)
{
    LATBbits.LATB11 = false;
    return;
}

static void TurnOnPairedLED(void)
{
    LATBbits.LATB15 = true;
    return;
}

static void TurnOffPairedLED(void)
{
    LATBbits.LATB15 = false;
    return;
}

static void ToggleCommsLED(void)
{
    bool NewState;
    NewState = !CommsLEDStatus;
    LATBbits.LATB2 = NewState;
    CommsLEDStatus = NewState;
    return;
}

static void UpdateThrustVals(void)
{
    ADC_MultiRead(ThrustADCValues);
    LeftThrustVal = ThrustADCValues[1];
    RightThrustVal = ThrustADCValues[0];
    return;
}

static int32_t CalibrateLeft(uint32_t RawInput)
{
    int32_t CalibratedValue;
    if (RawInput<381) {
        CalibratedValue = RawInput - 381;
    }
    else if (RawInput<642) {
        CalibratedValue = 0;
    }
    else if (RawInput<1024) {
        CalibratedValue = RawInput - 642;
    }
    else {
        puts("Erroneous value inputted into calibration function\r\n");
        CalibratedValue = 0;
    }
    
    return CalibratedValue;
}

static int32_t CalibrateRight(uint32_t RawInput)
{
    int32_t CalibratedValue;
    if (RawInput<233) {
        CalibratedValue = ((163*RawInput)/100) - 381;
    }
    else if (RawInput<465) {
        CalibratedValue = 0;
    }
    else if (RawInput<1024) {
        CalibratedValue = (68*(RawInput - 465))/100;
    }
    else {
        puts("Erroneous value inputted into calibration function\r\n");
        CalibratedValue = 0;
    }
    
    return CalibratedValue;
}
