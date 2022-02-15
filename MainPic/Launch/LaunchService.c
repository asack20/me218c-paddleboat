/****************************************************************************
 Module
   LaunchService.c

 Revision
   1.0.1

 Description
   This is the service to demonstrate the Servo library

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 10/30/21 09:58 jec      started work based on TestHarnessService0
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
// Hardware
#include <xc.h>

// Event & Services Framework
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "ES_Port.h"
#include "../HALs/PIC32PortHAL.h"
#include "terminal.h"
#include "dbprintf.h"

// This module
#include "LaunchService.h"


/*----------------------------- Module Defines ----------------------------*/
// these times assume a 1.000mS/tick timing
#define ONE_SEC 1000
#define HALF_SEC (ONE_SEC / 2)
#define TWO_SEC (ONE_SEC * 2)
#define FIVE_SEC (ONE_SEC * 5)
#define TEN_MS (10)
#define TWENTY_FIVE_MS (25)
#define FIFTY_MS (50)

#define TIME_STEP TWENTY_FIVE_MS

// TICS_PER_MS assumes a 20MHz PBClk /8 = 2.5MHz clock rate
#define TICS_PER_MS 2500

// these are extents of servo motion
#define FULL_CW ((uint16_t)(0.5*TICS_PER_MS))
#define FULL_CCW ((uint16_t)(2.35*TICS_PER_MS))
#define MID_POINT (FULL_CW+((FULL_CCW-FULL_CW)/2))

// these are related to how fast we move. full range of motion in 100 steps
#define TICKS_PER_STEP ((FULL_CCW-FULL_CW)/100)

#define DOOR_CNL 4 // Use pwm channel 4
#define DOOR_PIN PWM_RPB13 //Servo on RB13 (Pin 24)
#define FULL_OPEN 5700 //Open is counter-Clockwise
#define FULL_CLOSE 2900 //close is clockwise

#define DOOR_TMR _Timer3_ // Use PWM timer 3
#define DOOR_FREQ 50 // 50 HZ
#define NUM_PWM 5 // Configure All PWM Channels?
/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitLaunchService

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, and does any
     other required initialization for this service
 Notes

 Author
    
****************************************************************************/
bool InitLaunchService(uint8_t Priority)
{
    puts("Initializing Launch Service\r");
    ES_Event_t ThisEvent;
    bool ReturnVal = true; // assume that everything will be OK
    
    MyPriority = Priority;
    
    // disable interrupts
    __builtin_disable_interrupts();
    // configure pins
    PortSetup_ConfigureDigitalOutputs(_Port_B, _Pin_2);//Flag Servo, OC4
    PortSetup_ConfigureDigitalOutputs(_Port_B, _Pin_3);//Reloader Servo, OC1
    PortSetup_ConfigureDigitalOutputs(_Port_A, _Pin_2);//Latch Servo, OC5
    PortSetup_ConfigureDigitalOutputs(_Port_A, _Pin_3);//Tensioner Servo, OC3
    // configure interrupts
    INTCONSET = _INTCON_MVEC_MASK;
    //Timer 3
    IFS0CLR = _IFS0_T3IF_MASK;
    IEC0SET = _IEC0_T3IE_MASK;
    IPC3bits.T3IP = 5;
    T3CONCLR = _T3CON_ON_MASK;
    T3CONbits.TCKPS = 0b011;
    TMR3 = 0;
    PR3 = 800;
    //OC1
    OC1CONCLR = _OC1CON_ON_MASK;
    RPB3R = 0b0101;
    OC1CONCLR = _OC1CON_OC32_MASK;
    OC1CONSET = _OC1CON_OCTSEL_MASK;
    OC1CONbits.OCM = 0b110;
    OC1R = 0;
    OC1RS = 0;
    //OC3
    OC3CONCLR = _OC3CON_ON_MASK;
    RPA3R = 0b0101;
    OC3CONCLR = _OC3CON_OC32_MASK;
    OC3CONSET = _OC3CON_OCTSEL_MASK;
    OC3CONbits.OCM = 0b110;
    OC3R = 0;
    OC3RS = 0;
    //OC4
    OC4CONCLR = _OC4CON_ON_MASK;
    RPB2R = 0b0101;
    OC4CONCLR = _OC4CON_OC32_MASK;
    OC4CONSET = _OC4CON_OCTSEL_MASK;
    OC4CONbits.OCM = 0b110;
    OC4R = 0;
    OC4RS = 0;
    //OC5
    OC5CONCLR = _OC5CON_ON_MASK;
    RPA2R = 0b0110;
    OC5CONCLR = _OC5CON_OC32_MASK;
    OC5CONSET = _OC5CON_OCTSEL_MASK;
    OC5CONbits.OCM = 0b110;
    OC5R = 0;
    OC5RS = 0;
    __builtin_enable_interrupts();
    T3CONSET = _T3CON_ON_MASK;
    OC1CONSET = _OC1CON_ON_MASK;
    OC3CONSET = _OC3CON_ON_MASK;
    OC4CONSET = _OC4CON_ON_MASK;
    OC5CONSET = _OC5CON_ON_MASK;
    
    /********************************************
     Initialization sequence for timers to do servo drive
     *******************************************/

    // post the initial transition event
    ThisEvent.EventType = ES_INIT;
    if (!ES_PostToService(MyPriority, ThisEvent))
    {
        ReturnVal = false;
    }
    else
    {
      // no else clause
    }
    return ReturnVal;
}

/****************************************************************************
 Function
     PostLaunchService

 Parameters
     ES_Event ThisEvent ,the event to post to the queue

 Returns
     bool false if the Enqueue operation failed, true otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     
****************************************************************************/
bool PostLaunchService(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunLaunchService

 Parameters
   ES_Event : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
 Responds to events to move servo
 Notes

 Author

****************************************************************************/
ES_Event_t RunLaunchService(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  
  switch (ThisEvent.EventType)
  {
    case FLAG_UP:
    {
    }
    break;
    case FLAG_DOWN:
    {
    }
    break;
    case RELOAD_OUT:
    {
    }
    break;
    case RELOAD_IN:
    {
    }
    break;
    case LATCH_ENGAGE:
    {
    }
    break;
    case LATCH_RELEASE:
    {
    }
    break;
    case TENSION_ENGAGE:
    {
    }
    break;
    case TENSION_RELEASE:
    {
    }
    break;
    case SERVO_RESET:
    {
    }
    break;
    default:
    {}
    break;
  }

  return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

