/****************************************************************************
 Module
   KeyboardResponses.c

 Revision
   1.0.1

 Description
   This is a template file for implementing a simple service under the
   Gen2 Events and Services Framework.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 01/16/12 09:58 jec      began conversion from TemplateFSM.c
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "KeyboardResponses.h"
#include "PilotFSM.h"
#include "terminal.h"
#include "dbprintf.h"
#include <string.h>

/*----------------------------- Module Defines ----------------------------*/

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
     InitTemplateService

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, and does any
     other required initialization for this service
 Notes

 Author
     J. Edward Carryer, 01/16/12, 10:00
****************************************************************************/
bool InitKeyboardResponses(uint8_t Priority)
{
  ES_Event_t ThisEvent;

  MyPriority = Priority;
  /********************************************
   in here you write your initialization code
   *******************************************/
  clrScrn();
  puts("\rStarting Test Harness for \r");
  DB_printf( "the 2nd Generation Events & Services Framework V2.4\r\n");
  DB_printf( "compiled at %s on %s\n", __TIME__, __DATE__);
  DB_printf( "\n\r\n");
  
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
     PostTemplateService

 Parameters
     EF_Event_t ThisEvent ,the event to post to the queue

 Returns
     bool false if the Enqueue operation failed, true otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:25
****************************************************************************/
bool PostKeyboardResponses(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunTemplateService

 Parameters
   ES_Event_t : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here
 Notes

 Author
   J. Edward Carryer, 01/15/12, 15:23
****************************************************************************/
ES_Event_t RunKeyboardResponses(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  /********************************************
   in here you write your service code
   *******************************************/
  switch (ThisEvent.EventType)
  {
      case ES_INIT:
      {
          puts("Initialize Event Received in KeyboardResponses Service\r");
          puts("Press \'?\' for a list of available commands\r\n");
      }
      break;
      
      case ES_NEW_KEY:
      {
          //DB_printf("Key received:  \'%c\'\n",(char)ThisEvent.EventParam);
          ES_Event_t NewEvent;
          switch (ThisEvent.EventParam)
          {
              case '?':
              {
                  puts("------------------------------------------------------\r");
                  puts("Legend of Keyboard Commands\r");
                  puts("Simulate ACK_RECEIVED Event:                       \'A\'\r");
                  puts("Simulate PAIR_BUTTON_PRESSED Event:                \'B\'\r");
                  puts("Simulate CommsTimer ES_TIMEOUT Event:              \'C\'\r");
                  puts("Simulate InactivityTimer ES_TIMEOUT Event:         \'I\'\r");
                  puts("Simulate VALID_STATUS_RECEIVED Event:              \'V\'\r");
                  puts("Query State of PilotFSM:                           \'Q\'\r");
                  puts("Query Address of Target TUG:                       \'T\'\r");
                  puts("Query Left Thrust Value:                           \'L\'\r");
                  puts("Query Right Thrust Value:                          \'R\'\r");
                  puts("------------------------------------------------------\r\n");
              }
              break;
              
              case 'A':
              {
                puts("Posting ACK_RECEIVED Event to PilotFSM\r\n");
                NewEvent.EventType = ACK_RECEIVED;
                PostPilotFSM(NewEvent);
              }
              break;
              
              case 'B':
              {
                puts("Posting PAIR_BUTTON_PRESSED Event to PilotFSM\r\n");
                NewEvent.EventType = PAIR_BUTTON_PRESSED;
                PostPilotFSM(NewEvent);
              }
              break;
              
              case 'C':
              {
                puts("Posting CommsTimer ES_TIMEOUT Event to PilotFSM\r\n");
                NewEvent.EventType = ES_TIMEOUT;
                NewEvent.EventParam = COMMSTIMER;
                PostPilotFSM(NewEvent);
              }
              break;
              
              case 'I':
              {
                puts("Posting InactivityTimer ES_TIMEOUT Event to PilotFSM\r\n");
                NewEvent.EventType = ES_TIMEOUT;
                NewEvent.EventParam = INACTIVITYTIMER;
                PostPilotFSM(NewEvent);
              }
              break;
              
              case 'V':
              {
                puts("Posting VALID_STATUS_RECEIVED Event to PilotFSM\r\n");
                NewEvent.EventType = VALID_STATUS_RECEIVED;
                PostPilotFSM(NewEvent);
              }
              break;
              
              
              case 'Q':
              {
                PilotState_t CurrentPilotFSMState;
                CurrentPilotFSMState = QueryPilotFSM();
                char StateChar[40];
                
                switch (CurrentPilotFSMState)
                {
                    case AttemptingToPair:
                    {
                        strcpy(StateChar,"AttemptingToPair");
                    }
                    break;
                    
                    case Paired:
                    {
                        strcpy(StateChar,"Paired");
                    }
                    break;
                    
                    default:
                    {
                        strcpy(StateChar,"Not in a valid state - ERROR");
                    }
                    break;
                }
                puts("Querying PilotFSM:\r");
                DB_printf("PilotFSM is in state %s\r\n\n",StateChar);
              }
              break;
              
              case 'T':
              {
                  uint8_t FullVal = QueryPairingSelectorAddress();
                  bool MSBVal = (FullVal & (1<<2));
                  bool MidBitVal = (FullVal & (1<<1));
                  bool LSBVal = (FullVal & (1<<0));
                DB_printf("Target TUG Address is: %d%d%d\r\n\n",MSBVal,MidBitVal,LSBVal);
              }
              break;
              
              case 'L':
              {
                DB_printf("Left Thrust Analog Input is: %d\r\n\n",QueryLeftThrustVal());
              }
              break;
              
              case 'R':
              {
                DB_printf("Right Thrust Analog Input is: %d\r\n\n",QueryRightThrustVal());
              }
              break;
              
              default:
                  break;
          }
      }
      break;
      
      default:
          break;
  }
  
  
  return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

