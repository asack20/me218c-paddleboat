/****************************************************************************
 Module
   TugComm.c

 Revision
   1.0.1

 Description
   FSM to handle high level pairing and communication for the TUG

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------

****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "TugComm.h"
#include "../Propulsion/Propulsion.h"
#include "../HALs/PIC32PortHAL.h"
#include <xc.h>
#include <sys/attribs.h>

/*----------------------------- Module Defines ----------------------------*/
#define DEBUG_PRINT // define to enable debug message printing

#define ONE_SEC
#define TIMEOUT_TIME (5*ONE_SEC)
#define TRANSMIT_TIME 200 // ms (5 Hz)

#define BUTTON_PORT PORTAbits.RA0

/*----------------------------- Module Types ------------------------------*/
/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match that of enum in header file
static TugCommState_t CurrentState;

// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;

bool LastButtonState;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitTugComm

 Parameters
     uint8_t : the priority of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, sets up the initial transition and does any
     other required initialization for this state machine
 Notes

 Author
 Andrew Sack
****************************************************************************/
bool InitTugComm(uint8_t Priority)
{
    puts("Initializing TugComm...\r");
    ES_Event_t ThisEvent;

    MyPriority = Priority;
    // Initialize into waiting to Pair
    CurrentState = WaitingForPairRequestState;

    // Configure Pairing Button as Digital Input with pull up RA0
    PortSetup_ConfigureDigitalInputs(_Port_A, _Pin_0);
    PortSetup_ConfigurePullUps(_Port_A, _Pin_0);
    LastButtonState = BUTTON_PORT;

    puts("...Done Initializing TugComm\r\n");
 
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
     PostTugComm

 Parameters
     ES_Event_t ThisEvent , the event to post to the queue

 Returns
     boolean False if the Enqueue operation failed, True otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
 Andrew Sack
****************************************************************************/
bool PostTugComm(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunTugComm

 Parameters
   ES_Event_t : the event to process

 Returns
   ES_Event_t, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
  
 Notes
   uses nested switch/case to implement the machine.
 Author
 Andrew Sack
****************************************************************************/
ES_Event_t RunTugComm(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
    
    ES_Event_t PostEvent;
    
    switch (CurrentState)
    {
        case (WaitingForPairRequestState):
        {
            switch (ThisEvent.EventType)
            {
                case (ES_INIT):
                {
                    // Post WAIT_TO_PAIR TO PROPULSION
                    PostEvent.EventType = WAIT_TO_PAIR;
                    PostPropulsion(PostEvent);
                } break;
                case (XBEE_MESSAGE_RECEIVED):
                {
                    printf("TugComm: XBEE_MESSAGE_RECEIVED in PairRequestState\r\n");
                    // TODO
                    
                } break;
                default:
                    ;
            }
        } break;
        
        case (WaitingForControlPacketState):
        {
            switch (ThisEvent.EventType)
            {
                case (PAIRING_BUTTON_PRESSED):
                {
                    printf("TugComm: PAIRING_BUTTON_PRESSED in ControlPacketState\r\n");
                    // Stop timers and return to Waiting for Pair Request
                    ES_Timer_StopTimer(COMM_TIMEOUT_TIMER);
                    ES_Timer_StopTimer(TRANSMISSION_TIMER);
                    // Post WAIT_TO_PAIR TO PROPULSION
                    PostEvent.EventType = WAIT_TO_PAIR;
                    PostPropulsion(PostEvent);
                    // Update State
                    CurrentState = WaitingForPairRequestState;
                } break;
                case (ES_TIMEOUT):
                {
                    // Check which timer it was
                    if (ThisEvent.EventParam == COMM_TIMEOUT_TIMER)
                    {
                        printf("TugComm: COMM_TIMEOUT in ControlPacketState\r\n");
                        // Stop timers and return to Waiting for Pair Request
                        ES_Timer_StopTimer(COMM_TIMEOUT_TIMER);
                        ES_Timer_StopTimer(TRANSMISSION_TIMER);
                        // Post WAIT_TO_PAIR TO PROPULSION
                        PostEvent.EventType = WAIT_TO_PAIR;
                        PostPropulsion(PostEvent);
                        // Update State
                        CurrentState = WaitingForPairRequestState;
                    }
                    else if (ThisEvent.EventParam == TRANSMISSION_TIMER)
                    {
                        // TODO: Transmit Pairing Acknowledged
                        // Reinit timer
                        ES_Timer_InitTimer(TRANSMISSION_TIMER, TRANSMIT_TIME);
                    }
                } break;
                case (XBEE_MESSAGE_RECEIVED):
                {
                    printf("TugComm: XBEE_MESSAGE_RECEIVED in ControlPacketState\r\n");
                    // TODO
                } break;
                default:
                    ;
            }
        } break;
        case (PairedState):
        {
            switch (ThisEvent.EventType)
            {
                case (PAIRING_BUTTON_PRESSED):
                {
                    printf("TugComm: PAIRING_BUTTON_PRESSED in PairedState\r\n");
                    // Stop timers and return to Waiting for Pair Request
                    ES_Timer_StopTimer(COMM_TIMEOUT_TIMER);
                    ES_Timer_StopTimer(TRANSMISSION_TIMER);
                    // Post WAIT_TO_PAIR TO PROPULSION
                    PostEvent.EventType = WAIT_TO_PAIR;
                    PostPropulsion(PostEvent);
                    // Update State
                    CurrentState = WaitingForPairRequestState;
                } break;
                case (ES_TIMEOUT):
                {
                    // Check which timer it was
                    if (ThisEvent.EventParam == COMM_TIMEOUT_TIMER)
                    {
                        printf("TugComm: COMM_TIMEOUT in PairedState\r\n");
                        // Stop timers and return to Waiting for Pair Request
                        ES_Timer_StopTimer(COMM_TIMEOUT_TIMER);
                        ES_Timer_StopTimer(TRANSMISSION_TIMER);
                        // Post WAIT_TO_PAIR TO PROPULSION
                        PostEvent.EventType = WAIT_TO_PAIR;
                        PostPropulsion(PostEvent);
                        // Update State
                        CurrentState = WaitingForPairRequestState;
                    }
                    else if (ThisEvent.EventParam == TRANSMISSION_TIMER)
                    {
                        // TODO: Transmit Status
                        // Reinit timer
                        ES_Timer_InitTimer(TRANSMISSION_TIMER, TRANSMIT_TIME);
                    }
                } break;
                case (XBEE_MESSAGE_RECEIVED):
                {
                    printf("TugComm: XBEE_MESSAGE_RECEIVED in PairedState\r\n");
                    // TODO
                    // Validate Message
                    // Post Propulsion Refuel and Set Thrust to Propulsion
                    // Reinit COMM_Timeout_Timer
                } break;
                default:
                    ;
            }
        } break;
        default:
          ;
    }                                   // end switch on Current State
    return ReturnEvent;
}


/****************************************************************************
 Function
     QueryTugComm

 Parameters
     None

 Returns
     TugCommState_t The current state of the TugComm state machine

 Description
     returns the current state of the TugComm state machine
 Notes

 Author
 Andrew Sack
****************************************************************************/
TugCommState_t QueryTugComm(void)
{
    return CurrentState;
}

/****************************************************************************
 Function
    CheckPairingButton

 Parameters
    None

 Returns
    True if event occurred

 Description
    Checks if Pairing Button went from unpressed to pressed
 Notes

 Author
 Andrew Sack
****************************************************************************/
bool CheckPairingButton(void)
{
    bool CurrentButtonState = BUTTON_PORT;
    bool ReturnVal = false;
    
    // went unpressed to pressed
    if ((CurrentButtonState == 0) && (LastButtonState == 1))
    {
        ES_Event_t PostEvent;
        PostEvent.EventType = PAIRING_BUTTON_PRESSED;
        PostTugComm(PostEvent);
        ReturnVal = true;
    }
    
    // Update last state
    LastButtonState = CurrentButtonState;
    return ReturnVal;
}


/***************************************************************************
 private functions
 ***************************************************************************/

