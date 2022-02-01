/****************************************************************************
 Module
   Find_Beacon.c

 Revision
   1.0.1

 Description
   This is a template file for implementing flat state machines under the
   Gen2 Events and Services Framework.

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
#include "Find_Beacon.h"

/*----------------------------- Module Defines ----------------------------*/

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file
static Find_BeaconState_t CurrentState;

// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitFind_Beacon

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
bool InitFind_Beacon(uint8_t Priority)
{
  ES_Event_t ThisEvent;

  MyPriority = Priority;
  // put us into the Initial PseudoState
  CurrentState = Waiting;
  
  //Do lots of setup:
  //Setup the timer 2 for input capture 1
  SetupTimer2();
  //Setup the input capture 1 module
  SetupIC1();
  
  //Set up and enable interrupts using the following steps:
//	Ensure multi-vector mode is enabled (INTCONbits.MVEC = 1)
  INTCONbits.MVEC = 1;
//	Ensure global interrupts are on (__builtin_enable_interrupts())
  __builtin_enable_interrupts();
  
  //Turn on timer 2 to get the ball rolling (T2CONbits.ON = 1)
  T2CONbits.ON = 1;
  
  //Turn on input capture 1 (IC1CONbits.ON = 1)
  IC1CONbits.ON = 1;
  
  
  
  // post the initial transition event
//  ThisEvent.EventType = ES_INIT;
//  if (ES_PostToService(MyPriority, ThisEvent) == true)
//  {
//    return true;
//  }
//  else
//  {
//    return false;
//  }
  return true;
}

/****************************************************************************
 Function
     PostFind_Beacon

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
bool PostFind_Beacon(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunFind_Beacon

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
ES_Event_t RunFind_Beacon(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

  switch (CurrentState)
  {
    case Waiting:
    {
      
    }
    break;

    case Searching:
    {
      
    }
    break;
    // repeat state pattern as required for other states
    default:
      ;
  }                                   // end switch on Current State
  return ReturnEvent;
}

/****************************************************************************
 Function
     QueryFind_Beacon

 Parameters
     None

 Returns
     Find_BeaconState_t The current state of the Template state machine

 Description
     returns the current state of the Template state machine
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:21
****************************************************************************/
Find_BeaconState_t QueryFind_Beacon(void)
{
  return CurrentState;
}

/***************************************************************************
 private functions
 ***************************************************************************/

static bool SetupTimer2(void) {
    //Setup timer 2 for use in input capture for beacon detection
  
  //Set up timer 2 using the following steps:
//		Disable the timer (T2CONbits.ON = 0)
  T2CONbits.ON = 0;
//		Select the internal PBCLK source (T2CONbits.TCS = 0)
  T2CONbits.TCS = 0;
//		Disable gated time accumulation (T2CONbits.TGATE = 0)
  T2CONbits.TGATE = 0;
//		Set the input clock prescale to 4 (T2CONbits.TCKPS = 0b010)
  T2CONbits.TCKPS = 0b010;
//		Set the period register to the max value for 16 bits (PR2 = 0xFFFF)
  PR2 = 0xFFFF;
//		Clear the timer (TMR2 = 0)
  TMR2 = 0;
//		Clear the T2IF interrupt flag bit (IFS0CLR = _IFS0_T2IF_MASK)
  IFS0CLR = _IFS0_T2IF_MASK;
//		Set the timer's interrupt priority level to 6 (IPC2bits.T2IP = 6)
  IPC2bits.T2IP = 6;
//		Set the local interrupt enable for the timer (IEC0SET = _IEC0_T2IE_MASK)
  IEC0SET = _IEC0_T2IE_MASK;
}

static bool SetupIC1(void) {
     //Setup input capture 1 module for beacon detection
    
//	Configure pin RA4 as a digital input (PortSetup_ConfigureDigitalInputs(_Port_A, _Pin_4)) for input capture
  PortSetup_ConfigureDigitalInputs(_Port_A, _Pin_4);
  
    //	Disable the Input Capture module (IC1CONbits.ON = 0)
  IC1CONbits.ON = 0;
    //  Map RPA4 to the input capture 1 input (IC1R = 0b0010)
  IC1R = 0b0010;
    //	Use a 16-bit timer capture (IC1CONbits.C32 = 0)
  IC1CONbits.C32 = 0;
    //	Select timer 2 for the input capture (IC1CONbits.ICTMR = 1)
  IC1CONbits.ICTMR = 1;
    //	Make interrupts occur on every capture event (IC1CONbits.ICI = 0b00)
  IC1CONbits.ICI = 0b00;
    //	Set up input capture mode to simple capture event mode every rising edge (IC1CONbits.ICM = 0b011)
  IC1CONbits.ICM = 0b011;
    //	Clear the input capture interrupt flag bit (IFS0CLR = _IFS0_IC1IF_MASK)
  IFS0CLR = _IFS0_IC1IF_MASK;
    //	Set the input capture?s interrupt priority level to 7 (IPC1bits.IC1IP = 7)
  IPC1bits.IC1IP = 7;
    //	Set the local interrupt enable for the input capture module (IEC0SET = _IEC0_IC1IE_MASK)
  IEC0SET = _IEC0_IC1IE_MASK;
}