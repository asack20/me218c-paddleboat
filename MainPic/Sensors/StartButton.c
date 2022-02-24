/****************************************************************************
 Module
   StartButton.c

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
#include "terminal.h"
#include "StartButton.h"
#include "../HSM/RobotTopHSM.h"
#include "../HALs/PIC32PortHAL.h"
#include "../HALs/PIC32_SPI_HAL.h"
#include "terminal.h"
#include "dbprintf.h"
#include "ES_Events.h" 
#include <xc.h>

/*----------------------------- Module Defines ----------------------------*/

#define START_BUTTON_PORT _Port_A //Physical pin 12
#define START_BUTTON_PIN _Pin_4
#define START_BUTTON_VAL PORTAbits.RA4
#define DEBOUNCE_TIME 5

/*----------------------------- Module Types ------------------------------*/
// typedefs for the states
// State definitions for use with the query function

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/



/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file
static StartButtonState_t CurrentState;
static bool LastButtonState;
static bool CurrentButtonState;

// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitStartButton

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
bool InitStartButton(uint8_t Priority)
{
    puts("Initializing StartButton...\r");
    ES_Event_t ThisEvent;

    MyPriority = Priority;
    // put us into the Initial PseudoState
    
    //Initialize digital input for start button
    PortSetup_ConfigureDigitalInputs(START_BUTTON_PORT, START_BUTTON_PIN);
    //Set LastButtonState to value read from button pin
    LastButtonState = (bool) START_BUTTON_VAL;
    CurrentButtonState = LastButtonState;
    
    CurrentState = StartButtonHigh;
    
    puts("...Done Initializing StartButton\r\n");
 
    // post the initial transition event
    ThisEvent.EventType = ES_INIT;
    if (ES_PostToService(MyPriority, ThisEvent) == true)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/****************************************************************************
 Function
     PostStartButton

 Parameters
     ES_Event_t ThisEvent , the event to post to the queue

 Returns
     boolean False if the Enqueue operation failed, True otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
 Afshan Chandani
****************************************************************************/
bool PostStartButton(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunStartButton

 Parameters
   ES_Event_t : the event to process

 Returns
   ES_Event_t, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
  
 Notes
   uses nested switch/case to implement the machine.
 Author
 Afshan Chandani
****************************************************************************/
ES_Event_t RunStartButton(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
    ES_Event_t PostEvent;
    
    switch (CurrentState)
    {
        
        case StartButtonHigh:
        {
            if (ThisEvent.EventType == START_BUTTON_CHANGE){
                ES_Timer_InitTimer(StartButtonTimer,DEBOUNCE_TIME);
            }
            if (ThisEvent.EventType == ES_TIMEOUT){
                if(CurrentButtonState == 0){
                    CurrentState = StartButtonLow;
                    ES_Event_t PostEvent;
                    PostEvent.EventType = START_BUTTON_PRESSED;
                    //printf("start\r\n");
                    PostRobotTopHSM(PostEvent);
                    puts("Start Button Pressed (StartButton.c)\r\n");
                }
            }           
        }break;
        case StartButtonLow:
        {
            if (ThisEvent.EventType == START_BUTTON_CHANGE){
                ES_Timer_InitTimer(StartButtonTimer,DEBOUNCE_TIME);
            }
            if (ThisEvent.EventType == ES_TIMEOUT){
                if(CurrentButtonState == 1){
                    CurrentState = StartButtonHigh;
                }
            }
        }break;
        default:
          ;
    }                                   // end switch on Current State
    
    return ReturnEvent;
}


/****************************************************************************
 Function
     QueryStartButton

 Parameters
     None

 Returns
     StartButtonState_t The current state of the StartButton state machine

 Description
     returns the current state of the Drive Train state machine
 Notes

 Author
 Afshan Chandani
****************************************************************************/
StartButtonState_t QueryStartButton(void)
{
    return CurrentState;
}

bool CheckStartButtonEvents(void)
{
    bool ReturnVal = false;
    ES_Event_t PostEvent;
    
    // Set CurrentButtonState to state read from port pin
    CurrentButtonState = (bool) START_BUTTON_VAL;
    // If the CurrentButtonState is different from the LastButtonState
    if (CurrentButtonState != LastButtonState)
    {  
        PostEvent.EventType = START_BUTTON_CHANGE;
        PostStartButton(PostEvent); 
    }
    LastButtonState = CurrentButtonState;
    return ReturnVal;
}
