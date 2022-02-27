/****************************************************************************
 Module
   RobotTopHSM.c

 Revision
   2.0.1

 Description
   This is a template for the top level Hierarchical state machine

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 02/20/17 14:30 jec      updated to remove sample of consuming an event. We 
                         always want to return ES_NO_EVENT at the top level 
                         unless there is a non-recoverable error at the 
                         framework level
 02/03/16 15:27 jec      updated comments to reflect small changes made in '14 & '15
                         converted unsigned char to bool where appropriate
                         spelling changes on true (was True) to match standard
                         removed local var used for debugger visibility in 'C32
                         removed Microwave specific code and replaced with generic
 02/08/12 01:39 jec      converted from MW_MasterMachine.c
 02/06/12 22:02 jec      converted to Gen 2 Events and Services Framework
 02/13/10 11:54 jec      converted During functions to return Event_t
                         so that they match the template
 02/21/07 17:04 jec      converted to pass Event_t to Start...()
 02/20/07 21:37 jec      converted to use enumerated type for events
 02/21/05 15:03 jec      Began Coding
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "RobotTopHSM.h"
#include "GameHSM.h"
#include "StartupHSM.h"
#include "../Sensors/Find_Beacon.h"
#include "../Launch/LaunchService.h"
#include "../SPI/SPILeaderSM.h"
#include "../HALs/PIC32PortHAL.h"
#include "ES_Port.h"
#include "terminal.h"
#include "dbprintf.h"
#include <string.h>

/*----------------------------- Module Defines ----------------------------*/

#define ONE_SEC 1000
#define ONE_MIN (ONE_SEC * 60)
#define EIGHTEEN_SEC (ONE_SEC * 18)
#define DELAYTIME ONE_SEC
#define REDPIN _Pin_11
#define BLUEPIN _Pin_15
#define TURN_RED_ON LATBbits.LATB11 = 1
#define TURN_RED_OFF LATBbits.LATB11 = 0
#define TURN_BLUE_ON LATBbits.LATB15 = 1
#define TURN_BLUE_OFF LATBbits.LATB15 = 0
/*---------------------------- Module Functions ---------------------------*/
static ES_Event_t DuringRobotInitState( ES_Event_t Event);
static ES_Event_t DuringRobotInactiveState( ES_Event_t Event);
static ES_Event_t DuringRobotActiveState( ES_Event_t Event);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, though if the top level state machine
// is just a single state container for orthogonal regions, you could get
// away without it
static RobotTopHSMState_t CurrentState;
// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;
static uint8_t MinuteCounter=0;
static const bool EnableGameTimer = 1;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitRobotTopHSM

 Parameters
     uint8_t : the priorty of this service

 Returns
     boolean, False if error in initialization, True otherwise

 Description
     Saves away the priority,  and starts
     the top level state machine
 Notes

 Author
     J. Edward Carryer, 02/06/12, 22:06
****************************************************************************/
bool InitRobotTopHSM ( uint8_t Priority )
{
  ES_Event_t ThisEvent;

  MyPriority = Priority;  // save our priority
  
  //Configure digital outputs for team color indication
  PortSetup_ConfigureDigitalOutputs(_Port_B,REDPIN | BLUEPIN);
  
  

  ThisEvent.EventType = ES_ENTRY;
  // Start the Master State machine

  StartRobotTopHSM( ThisEvent );

  return true;
}

/****************************************************************************
 Function
     PostRobotTopHSM

 Parameters
     ES_Event_t ThisEvent , the event to post to the queue

 Returns
     boolean False if the post operation failed, True otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:25
****************************************************************************/
bool PostRobotTopHSM( ES_Event_t ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunRobotTopHSM

 Parameters
   ES_Event: the event to process

 Returns
   ES_Event: an event to return

 Description
   the run function for the top level state machine 
 Notes
   uses nested switch/case to implement the machine.
 Author
   J. Edward Carryer, 02/06/12, 22:09
****************************************************************************/
ES_Event_t RunRobotTopHSM( ES_Event_t CurrentEvent )
{
   bool MakeTransition = false;/* are we making a state transition? */
   RobotTopHSMState_t NextState = CurrentState;
   ES_Event_t EntryEventKind = { ES_ENTRY, 0 };// default to normal entry to new state
   ES_Event_t ReturnEvent = { ES_NO_EVENT, 0 }; // assume no error

    switch ( CurrentState )
   {
       case ROBOT_INIT_STATE :
       {  // If current state is state one
         // Execute During function for state one. ES_ENTRY & ES_EXIT are
         // processed here allow the lowere level state machines to re-map
         // or consume the event
         CurrentEvent = DuringRobotInitState(CurrentEvent);
         //process any events
         if ( CurrentEvent.EventType != ES_NO_EVENT ) //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
               case ES_TIMEOUT:
                   
                   if (CurrentEvent.EventParam == StartupDelayTimer)
                   {
                        //Entry Action:  Post ROBOT_INIT_COMPLETE
                        ES_Event_t NewEvent = {ROBOT_INIT_COMPLETE,0};
                        PostRobotTopHSM(NewEvent);
                       
                        NewEvent.EventType = LATCH_ENGAGE;
                        PostLaunchService(NewEvent);
                        
                        puts("Initialization Complete\r\n");
                   }
                   
                   break;
                
               case ROBOT_INIT_COMPLETE : //If event is event one
                  // Execute action function for state one : event one
                  NextState = ROBOT_INACTIVE_STATE;//Decide what the next state will be
                  // for internal transitions, skip changing MakeTransition
                  MakeTransition = true; //mark that we are taking a transition
                  // if transitioning to a state with history change kind of entry
                  EntryEventKind.EventType = ES_ENTRY;
                  break;
                // repeat cases as required for relevant events
            }
         }
       }
         break;
         
       case ROBOT_INACTIVE_STATE :
       {  // If current state is state one
         // Execute During function for state one. ES_ENTRY & ES_EXIT are
         // processed here allow the lowere level state machines to re-map
         // or consume the event
         CurrentEvent = DuringRobotInactiveState(CurrentEvent);
         //process any events
         if ( CurrentEvent.EventType != ES_NO_EVENT ) //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
               case START_BUTTON_PRESSED : //If event is event one
                  // Execute action function for state one : event one
                  
                  ES_Timer_InitTimer(StartButtonDelayTimer, ONE_SEC);
                  // for internal transitions, skip changing MakeTransition
                  MakeTransition = false; //mark that we are taking a transition
                  // if transitioning to a state with history change kind of entry
                  EntryEventKind.EventType = ES_ENTRY;
                  break;
                  
                case ES_TIMEOUT : //If event is event one
                  // Execute action function for state one : event one
                    if (CurrentEvent.EventParam == StartButtonDelayTimer) {
                        NextState = ROBOT_ACTIVE_STATE;//Decide what the next state will be

                        //Reset MinuteCounter
                        MinuteCounter = 0;
                        //Set Timer for one minute
                        if (EnableGameTimer) {
                          ES_Timer_InitTimer(GameTimer, ONE_MIN);
                        }
                        // for internal transitions, skip changing MakeTransition
                        MakeTransition = true; //mark that we are taking a transition
                        // if transitioning to a state with history change kind of entry
                        EntryEventKind.EventType = ES_ENTRY;
                    }
                  break;
                // repeat cases as required for relevant events
            }
         }
       }
         break;  
         
       case ROBOT_ACTIVE_STATE :
       {  // If current state is state one
         // Execute During function for state one. ES_ENTRY & ES_EXIT are
         // processed here allow the lowere level state machines to re-map
         // or consume the event
         CurrentEvent = DuringRobotActiveState(CurrentEvent);
         //process any events
         if ( CurrentEvent.EventType != ES_NO_EVENT ) //If an event is active
         {
            if ((CurrentEvent.EventType == GAME_TIMEOUT))
            {
                  //If event is event one
                  // Execute action function for state one : event one
                  puts("-------------------------------------\r\n");
                  puts("The game is over.  2:18 has passed.\r\n");
                  puts("-------------------------------------\r\n");
                
                  NextState = ROBOT_INIT_STATE;//Decide what the next state will be
                  // for internal transitions, skip changing MakeTransition
                  MakeTransition = true; //mark that we are taking a transition
                  // if transitioning to a state with history change kind of entry
                  EntryEventKind.EventType = ES_ENTRY;
                  break;
                // repeat cases as required for relevant events
            }
            if ((CurrentEvent.EventType == ES_TIMEOUT) && (CurrentEvent.EventParam == GameTimer) && EnableGameTimer ) {
                if (MinuteCounter == 2) {
                    MinuteCounter = 0;
                    ES_Event_t NewEvent;
                    NewEvent.EventType = GAME_TIMEOUT;
                    PostRobotTopHSM(NewEvent);
                }
                else {
                    if (MinuteCounter == 1) {
                        //Set Timer for eighteen seconds
                        ES_Timer_InitTimer(GameTimer, EIGHTEEN_SEC);
                    }
                    else{
                        //Set Timer for one minute
                        ES_Timer_InitTimer(GameTimer, ONE_MIN);
                    }
                    MinuteCounter++;
                }
            }
         }
       }
         break;  
         
        default:
            break;
      // repeat state pattern as required for other states
    }
    //   If we are making a state transition
    if (MakeTransition == true)
    {
       //   Execute exit function for current state
       CurrentEvent.EventType = ES_EXIT;
       RunRobotTopHSM(CurrentEvent);

       CurrentState = NextState; //Modify state variable

       // Execute entry function for new state
       // this defaults to ES_ENTRY
       RunRobotTopHSM(EntryEventKind);
     }
   // in the absence of an error the top level state machine should
   // always return ES_NO_EVENT, which we initialized at the top of func
   return(ReturnEvent);
}
/****************************************************************************
 Function
     StartRobotTopHSM

 Parameters
     ES_Event CurrentEvent

 Returns
     nothing

 Description
     Does any required initialization for this state machine
 Notes

 Author
     J. Edward Carryer, 02/06/12, 22:15
****************************************************************************/
void StartRobotTopHSM ( ES_Event_t CurrentEvent )
{
  // if there is more than 1 state to the top level machine you will need 
  // to initialize the state variable
  CurrentState = ROBOT_INIT_STATE;
  // now we need to let the Run function init the lower level state machines
  // use LocalEvent to keep the compiler from complaining about unused var
  RunRobotTopHSM(CurrentEvent);
  return;
}

/****************************************************************************
 Function
     QueryRobotTopHSM

 Parameters
     None

 Returns
     RobotTopHSMState_t  The current state of the Top Level Template state machine

 Description
     returns the current state of the Template state machine
 Notes

 Author
     J. Edward Carryer, 2/05/22, 10:30AM
****************************************************************************/
RobotTopHSMState_t  QueryRobotTopHSM ( void )
{
   return(CurrentState);
}

/***************************************************************************
 private functions
 ***************************************************************************/

static ES_Event_t DuringRobotInitState( ES_Event_t Event)
{
    ES_Event_t ReturnEvent = Event; // assme no re-mapping or comsumption

    // process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {
        // implement any entry actions required for this state machine
        
        //Entry Action:  Initialize All Hardware
        puts("Entering RobotInitState - Initializing All Hardware (To Be Implemented)\r\n");
        puts("Start 1 second delay\r\n");
        
        ES_Event_t NewEvent;
        NewEvent.EventType = FLAG_DOWN;
        PostLaunchService(NewEvent);
        
        NewEvent.EventType = TENSION_RELEASE;
        PostLaunchService(NewEvent);
        
        NewEvent.EventType = RELOAD_IN;
        PostLaunchService(NewEvent);
        
        ES_Timer_InitTimer(StartupDelayTimer,DELAYTIME);
                
//        //Entry Action:  Post ROBOT_INIT_COMPLETE
//        ES_Event_t NewEvent = {ROBOT_INIT_COMPLETE,0};
//        PostRobotTopHSM(NewEvent);
//        puts("Initialization Complete\r\n");
        
        // after that start any lower level machines that run in this state
        //StartLowerLevelSM( Event );
        // repeat the StartxxxSM() functions for concurrent state machines
        // on the lower level
    }
    else if ( Event.EventType == ES_EXIT )
    {
        // on exit, give the lower levels a chance to clean up first
        //RunLowerLevelSM(Event);
        // repeat for any concurrently running state machines
        // now do any local exit functionality
      
    }else
    // do the 'during' function for this state
    {
        // run any lower level state machine
        // ReturnEvent = RunLowerLevelSM(Event);
      
        // repeat for any concurrent lower level machines
      
        // do any activity that is repeated as long as we are in this state
    }
    // return either Event, if you don't want to allow the lower level machine
    // to remap the current event, or ReturnEvent if you do want to allow it.
    return(ReturnEvent);
}

static ES_Event_t DuringRobotInactiveState( ES_Event_t Event)
{
    ES_Event_t ReturnEvent = Event; // assme no re-mapping or comsumption

    // process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {
        // implement any entry actions required for this state machine
        //ES_Event_t NewEvent;
        
        ES_Event_t NewEvent;
        NewEvent.EventType = FLAG_DOWN;
        PostLaunchService(NewEvent);
        
        //This is just for checkpoint 3 - simulate start button press
        //NewEvent.EventType = START_BUTTON_PRESSED;
        //PostRobotTopHSM(NewEvent);
        
        // after that start any lower level machines that run in this state
        //StartLowerLevelSM( Event );
        // repeat the StartxxxSM() functions for concurrent state machines
        // on the lower level
    }
    else if ( Event.EventType == ES_EXIT )
    {
        // on exit, give the lower levels a chance to clean up first
        //RunLowerLevelSM(Event);
        // repeat for any concurrently running state machines
        // now do any local exit functionality
      
    }else
    // do the 'during' function for this state
    {
        // run any lower level state machine
        // ReturnEvent = RunLowerLevelSM(Event);
      
        // repeat for any concurrent lower level machines
      
        // do any activity that is repeated as long as we are in this state
    }
    // return either Event, if you don't want to allow the lower level machine
    // to remap the current event, or ReturnEvent if you do want to allow it.
    return(ReturnEvent);
}

static ES_Event_t DuringRobotActiveState( ES_Event_t Event)
{
    ES_Event_t ReturnEvent = Event; // assme no re-mapping or comsumption

    // process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {
        // implement any entry actions required for this state machine
        TURN_RED_OFF;
        TURN_BLUE_OFF;
        
        ES_Event_t NewEvent;
        NewEvent.EventType = FLAG_UP;
        PostLaunchService(NewEvent);
        
        // after that start any lower level machines that run in this state
        StartGameHSM(Event);
        // repeat the StartxxxSM() functions for concurrent state machines
        // on the lower level
    }
    else if ( Event.EventType == ES_EXIT )
    {
        // on exit, give the lower levels a chance to clean up first
        RunGameHSM(Event);
        // repeat for any concurrently running state machines
        // now do any local exit functionality
        ES_Event_t NewEvent;
        NewEvent.EventType = GIVE_UP;
        PostFind_Beacon(NewEvent);
        
        SPI_MOSI_Command_t NewCommand;
        NewCommand.Name = SPI_STOP;
        NewEvent.EventType = SEND_SPI_COMMAND;
        NewEvent.EventParam = NewCommand.FullCommand;
        PostSPILeaderSM(NewEvent);
        NewEvent.EventType = SPI_RESET;
        PostSPILeaderSM(NewEvent);
      
    }else
    // do the 'during' function for this state
    {
        // run any lower level state machine
        ReturnEvent = RunGameHSM(Event);
      
        // repeat for any concurrent lower level machines
      
        // do any activity that is repeated as long as we are in this state
    }
    // return either Event, if you don't want to allow the lower level machine
    // to remap the current event, or ReturnEvent if you do want to allow it.
    return(ReturnEvent);
}