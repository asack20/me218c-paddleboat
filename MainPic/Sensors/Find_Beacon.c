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
#include <sys/attribs.h>
#include "../HSM/RobotTopHSM.h"
#include "Find_Beacon.h"
#include "../SPI/SPILeaderSM.h"
#include "../HALs/PIC32_AD_Lib.h"
#include "../HALs/PIC32PortHAL.h"
#include "terminal.h"
#include "dbprintf.h"
#include <stdbool.h>
#include "ES_Port.h"
#include <string.h>

/*----------------------------- Module Defines ----------------------------*/

#define REDTEAM_PULSEMAX 1666 // 5 million divided by desired min frequency (3.000 kHz in this case)
#define REDTEAM_PULSEMIN 1364 // 5 million divided by desired min frequency (3.666 kHz in this case)
#define BLUETEAM_PULSEMAX 6250 // 5 million divided by desired min frequency (800 Hz in this case)
#define BLUETEAM_PULSEMIN 5000 // 5 million divided by desired min frequency (1000 Hz in this case)

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/

static bool SetupTimer2(void);
static bool SetupIC4(void);
static void EnableIC4Interrupts(void);
static void DisableIC4Interrupts(void);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file
static Find_BeaconState_t CurrentState;

// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;

static volatile uint32_t LastRiseTime;
//	This records the last rising edge time

static volatile uint32_t RolloverCounter = 0;
//	This is the rollover counter for keeping track of time after timer 2 overflows

static volatile bool FirstMeasurementFlag = 1;
//	This flag is high until a first measurement has been taken

static volatile bool Found = 0;

static volatile uint32_t PulseMin;
static volatile uint32_t PulseMax;

static volatile SearchType_t SearchMode = DetermineTeam;
static volatile TeamIdentity_t TeamIdentity = Unknown;

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
  CurrentState = Waiting1;
  
  //Do lots of setup:
  //Setup the timer 2 for input capture 1
  SetupTimer2();
  //Setup the input capture 1 module
  SetupIC4();
  
  //Set up and enable interrupts using the following steps:
//	Ensure multi-vector mode is enabled (INTCONbits.MVEC = 1)
  INTCONbits.MVEC = 1;
//	Ensure global interrupts are on (__builtin_enable_interrupts())
  __builtin_enable_interrupts();
  
  //Turn on timer 2 to get the ball rolling (T2CONbits.ON = 1)
  T2CONbits.ON = 1;
  
  //Turn on input capture 4 (IC4CONbits.ON = 1)
  IC4CONbits.ON = 1;
  
  
  
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
  
  ES_Event_t PostEvent;
  
  switch (CurrentState)
  {
    case Waiting1:
    {
        Found = 0;
        switch (ThisEvent.EventType)
        {
            case FIND_BEACON:
            {
                Found = 0;
                //begin rotation
                if (ThisEvent.EventParam == DetermineTeam) {
                    SearchMode = DetermineTeam;
                    puts("Determining team identity\r");
                    TeamIdentity = Unknown;
                }
                else if (ThisEvent.EventParam == FindKnownFrequency) {
                    SearchMode = FindKnownFrequency;
                    char StateChar[40];
                    if (TeamIdentity == Red) {
                        strcpy(StateChar,"A (Red)");
                    }
                    else if (TeamIdentity == Blue) {
                        strcpy(StateChar,"B (Blue)");
                    }
                    else {
                        puts("Cannot search yet - need to determine team identity first\r\n");
                        break;
                    }
                    
                    DB_printf("Searching for Team %s\'s Beacon\r\n",StateChar);
                }
                puts("Finding beacon - start rotation\r\n");
                //PostEvent.EventType = DRIVE_ROTATE_CCWINF;
                //PostDriveTrain(PostEvent);
                
                Found = 0;
                FirstMeasurementFlag = 1;
                //enable IC4 interrupts
                EnableIC4Interrupts();
                
                CurrentState = Searching1;
                
                
            }
            break;
            
            default:
              ;
        }
    }
    break;

    case Searching1:
    {
        switch (ThisEvent.EventType)
        {
            case BEACON_FOUND:
            {
                //stop rotation
                
                //PostEvent.EventType = DRIVE_STOP_MOTORS;
                //PostDriveTrain(PostEvent);
                ES_Event_t NewEvent;
                NewEvent.EventType = BEACON_FOUND;
                NewEvent.EventParam = 0;
                PostRobotTopHSM(NewEvent);
                
                //send success message
                DB_printf("Beacon found\r\n");

                //disable IC4 interrupts
                DisableIC4Interrupts();
                FirstMeasurementFlag = 1;
                Found = 0;
                
                CurrentState = Waiting1;
            }
            break;
            
            case GIVE_UP:
            {
                //stop rotation
                //PostEvent.EventType = DRIVE_STOP_MOTORS;
                //PostDriveTrain(PostEvent);
                
                //send failure message
                DB_printf("Did not find beacon\r\n");
                
                //disable IC4 interrupts
                DisableIC4Interrupts();
                FirstMeasurementFlag = 1;
                Found = 0;
                
                CurrentState = Waiting1;
            }
            break;
            
            default:
                ;
        }
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

TeamIdentity_t QueryTeamIdentity(void) 
{
    return TeamIdentity;
}


void __ISR(_TIMER_2_VECTOR, IPL6SOFT) Timer2IntHandler(void) {
// Disable global interrupts (__builtin_disable_interrupts() or EnterCritical()) 
    EnterCritical();
// If Timer 2?s rollover interrupt is active (IFS0bits.T2IF is 1) 
    if (IFS0bits.T2IF) {
    // Increment RolloverCounter 
        RolloverCounter++;
    // Clear the rollover interrupt (IFS0CLR = _IFS0_T2IF_MASK) 
        IFS0CLR = _IFS0_T2IF_MASK;
    }
// Enable global interrupts again (__builtin_enable_interrupts() or ExitCritical()) 
    ExitCritical();

}

void __ISR(_INPUT_CAPTURE_4_VECTOR, IPL7SOFT) MeasureTimingIntHandler(void) {
//	Read input capture 4 buffer into local variable uint32_t CapturedTime
    uint32_t CapturedTime = IC4BUF;
//	Clear the pending capture interrupt (IFS0CLR = _IFS0_IC4IF_MASK)
    IFS0CLR = _IFS0_IC4IF_MASK;
//	
//	If Timer 2?s rollover interrupt is active (IFS0bits.T2IF is 1) and the CapturedTime is after rollover (<0x8000):
     if ((IFS0bits.T2IF == 1) && (CapturedTime<0x8000)) {
//		Increment RolloverCounter
            RolloverCounter++;
//		Clear the rollover interrupt (IFS0CLR = _IFS0_T2IF_MASK)
            IFS0CLR = _IFS0_T2IF_MASK;
     }
//	Set CapturedTime += (RolloverCounter << 16) 	(aka lower 16 bits are from the buffer and upper 16 bits are 
//  from the rollover buffer)
    CapturedTime += (RolloverCounter << 16);
//
//	If FirstMeasurementFlag is true, this is the first measurement, so collect it, disable the flag, and get out:
    if (FirstMeasurementFlag) {
//		Save CapturedTime into LastRiseTime
        LastRiseTime = CapturedTime;
//		Disable the flag (set FirstMeasurementFlag to false)
        FirstMeasurementFlag = 0;
    }
//	Else, this is not the first measurement, so we can measure a pulse period and report a duration:
    else {
        
        //Calculate the last pulsePeriod as CapturedTime ? LastRiseTime (pulsePeriod is a volatile uint32_t module 
//level variable)
        uint32_t pulsePeriod = CapturedTime - LastRiseTime;
        
        // If the pulse is in an acceptable range
        if (SearchMode == DetermineTeam) {
            if (((pulsePeriod < REDTEAM_PULSEMAX) && (pulsePeriod > REDTEAM_PULSEMIN)) && (!Found)) {

                TeamIdentity = Red;
                PulseMin = REDTEAM_PULSEMIN;
                PulseMax = REDTEAM_PULSEMAX;
                
                ES_Event_t ThisEvent;
                ThisEvent.EventType = BEACON_FOUND;
                //Post EncoderPulse event to service
                PostFind_Beacon(ThisEvent);

                Found = true;
            }
            else if (((pulsePeriod < BLUETEAM_PULSEMAX) && (pulsePeriod > BLUETEAM_PULSEMIN)) && (!Found)) {

                TeamIdentity = Blue;
                PulseMin = BLUETEAM_PULSEMIN;
                PulseMax = BLUETEAM_PULSEMAX;
                
                ES_Event_t ThisEvent;
                ThisEvent.EventType = BEACON_FOUND;
                //Post EncoderPulse event to service
                PostFind_Beacon(ThisEvent);

                Found = true;
            }
        }
        else if (SearchMode == FindKnownFrequency) {
            if (((pulsePeriod < PulseMax) && (pulsePeriod > PulseMin)) && (!Found)) {

                ES_Event_t ThisEvent;
                ThisEvent.EventType = BEACON_FOUND;
                //Post EncoderPulse event to service
                PostFind_Beacon(ThisEvent);
                DB_printf("Pulse Period is %d", pulsePeriod);
                Found = true;
            }
        }
        
//Save CapturedTime into LastRiseTime
        LastRiseTime = CapturedTime;
    }
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
  
  return true;
}

static bool SetupIC4(void) {
     //Setup input capture 4 module for beacon detection
    
//	Configure pin RB4 as a digital input (PortSetup_ConfigureDigitalInputs(_Port_B, _Pin_4)) for input capture
  PortSetup_ConfigureDigitalInputs(_Port_B, _Pin_4);
  
    //	Disable the Input Capture module (IC4CONbits.ON = 0)
  IC4CONbits.ON = 0;
    //  Map RPB4 to the input capture 4 input (IC4R = 0b0010)
  IC4R = 0b0010;
    //	Use a 16-bit timer capture (IC4CONbits.C32 = 0)
  IC4CONbits.C32 = 0;
    //	Select timer 2 for the input capture (IC4CONbits.ICTMR = 1)
  IC4CONbits.ICTMR = 1;
    //	Make interrupts occur on every capture event (IC4CONbits.ICI = 0b00)
  IC4CONbits.ICI = 0b00;
    //	Set up input capture mode to simple capture event mode every rising edge (IC4CONbits.ICM = 0b011)
  IC4CONbits.ICM = 0b011;
    //	Clear the input capture interrupt flag bit (IFS0CLR = _IFS0_IC4IF_MASK)
  IFS0CLR = _IFS0_IC4IF_MASK;
    //	Set the input capture?s interrupt priority level to 7 (IPC4bits.IC4IP = 7)
  IPC4bits.IC4IP = 7;
  
  //IEC0SET = _IEC0_IC1IE_MASK;
  
  DisableIC4Interrupts();
  
  return true;
}

static void EnableIC4Interrupts(void) {
  //	Set the local interrupt enable for the input capture module (IEC0SET = _IEC0_IC4IE_MASK)
  IEC0SET = _IEC0_IC4IE_MASK;
  IFS0CLR = _IFS0_IC4IF_MASK;
  IC4CONbits.ON = 1;
}

static void DisableIC4Interrupts(void) {
  //    Clear the local interrupt enable for the input capture module
  IEC0CLR = _IEC0_IC4IE_MASK;
  IFS0CLR = _IFS0_IC4IF_MASK;
  IC4CONbits.ON = 0;
}