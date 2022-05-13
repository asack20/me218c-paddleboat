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
#include "../HALs/PIC32PortHAL.h"
#include "../HALs/PIC32_AD_Lib.h"

/*----------------------------- Module Defines ----------------------------*/

#define FIVE_SEC 5000
#define ONE_SEC 1000
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
void ConfigureUARTforXBee(void);
void ConfigureSPI(void);
void ConfigureIOPins(void);
void LatchAddress(void);
void RequestToPair(void);
void SendControl(void);
void StartCommsTimer(void);
void StartInactivityTimer(void);
void StopInactivityTimer(void);
void ResetInactivityTimer(void);
bool UpdatePairButtonState(void);
bool UpdateMode3ButtonState(void);
void TurnOnTryingToPairLED(void);
void TurnOffTryingToPairLED(void);
void TurnOnPairedLED(void);
void TurnOffPairedLED(void);
void ToggleCommsLED(void);
void UpdateThrustVals(void);

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
****************************************************************************/
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
                Mode3ToBeActiveOnNextTransmission = false; //NOT SURE IF THIS IS WHAT WE WANT TO DO
                //in current state, this would only send 1 message with mode 3
                //information for each time the button is pressed.  This is
                //event driven.  Instead, we could forego the mode 3 button event
                //checker entirely and just send the state of the mode 3 button
                //in the message to the tug.
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
        ES_Timer_InitTimer(MODE3BUTTONDEBOUNCETIMER, ONE_TENTH_SEC);
        
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

uint32_t QueryLeftThrustVal(void)
{
    return LeftThrustVal;
}

uint32_t QueryRightThrustVal(void)
{
    return RightThrustVal;
}

/***************************************************************************
 private functions
 ***************************************************************************/

void ConfigureUARTforXBee(void)
{
    return;
}

void ConfigureSPI(void)
{
    return;
}

void ConfigureIOPins(void)
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
    
    return;
}

void LatchAddress(void)
{
    LatchAddressMSB = PORTBbits.RB4;
    LatchAddressMidBit = PORTAbits.RA3;
    LatchAddressLSB = PORTAbits.RA2;
    return;
}

void RequestToPair(void)
{
    return;
}

void SendControl(void)
{
    return;
}

void StartCommsTimer(void)
{
    ES_Timer_InitTimer(COMMSTIMER, ONE_FIFTH_SEC);
    return;
}

void StartInactivityTimer(void)
{
    ES_Timer_InitTimer(INACTIVITYTIMER, FIVE_SEC);
    return;
}

void StopInactivityTimer(void)
{
    ES_Timer_StopTimer(INACTIVITYTIMER);
    return;
}

void ResetInactivityTimer(void)
{
    ES_Timer_InitTimer(INACTIVITYTIMER, FIVE_SEC);
    return;
}

bool UpdatePairButtonState(void)
{
    return PAIRBUTTONBIT;
}

bool UpdateMode3ButtonState(void)
{
    return MODE3BUTTONBIT;
}

void TurnOnTryingToPairLED(void)
{
    LATBbits.LATB11 = true;
    return;
}

void TurnOffTryingToPairLED(void)
{
    LATBbits.LATB11 = false;
    return;
}

void TurnOnPairedLED(void)
{
    LATBbits.LATB15 = true;
    return;
}

void TurnOffPairedLED(void)
{
    LATBbits.LATB15 = false;
    return;
}

void ToggleCommsLED(void)
{
    bool NewState;
    NewState = !CommsLEDStatus;
    LATBbits.LATB2 = NewState;
    CommsLEDStatus = NewState;
    return;
}

void UpdateThrustVals(void)
{
    ADC_MultiRead(ThrustADCValues);
    LeftThrustVal = ThrustADCValues[1];
    RightThrustVal = ThrustADCValues[0];
    return;
}