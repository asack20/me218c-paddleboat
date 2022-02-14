/****************************************************************************
 Module
   KeyboardService.c

 Revision
   1.0.1

 Description
 Service to read keypresses from connected terminal and trigger events

 Notes


****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "KeyboardService.h"
#include "../DriveTrain/DriveTrain.h"
#include "../SensorInterfacing/Find_Beacon.h"
#include "../SensorInterfacing/Find_Tape.h"
#include "../CommandService/CommandService.h"

/*----------------------------- Module Defines ----------------------------*/

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/
void PrintInstructions(void);
/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitKeyboardService

 Parameters
     uint8_t : the priority of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, and does any
     other required initialization for this service
 Notes

 Author
 Andrew Sack
****************************************************************************/
bool InitKeyboardService(uint8_t Priority)
{
  printf("\n\n---------------------------------------------------------\n\r");
  printf("Initializing KeyboardService\r");
  PrintInstructions(); // print meaning of each key
  MyPriority = Priority;
  return true;
}

/****************************************************************************
 Function
     PostKeyboardService

 Parameters
     EF_Event_t ThisEvent ,the event to post to the queue

 Returns
     bool false if the Enqueue operation failed, true otherwise

 Description
     Posts an event to this state machine's queue
 Notes
****************************************************************************/
bool PostKeyboardService(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunKeyboardService

 Parameters
   ES_Event_t : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
 Check Which Keypress is received and Post corresponding event
 Notes

****************************************************************************/
ES_Event_t RunKeyboardService(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
    ES_Event_t PostEvent;
  
    switch (ThisEvent.EventType)
    {
        case ES_NEW_KEY:
        {
            switch (ThisEvent.EventParam)
            {
                case '?':
                {
                    PrintInstructions();
                } break;
                case 'q':
                {
                    printf("KeyboardService: posting SPI_COMMAND_RECEIVED to SPIFollowerSM\n\r");
                    PostEvent.EventType = SPI_COMMAND_RECEIVED;
                    PostSPIFollowerSM(PostEvent);
                } break;
                case 'w':
                {
                    printf("KeyboardService: posting SPI_TASK_COMPLETE to SPIFollowerSM\n\r");
                    PostEvent.EventType = SPI_TASK_COMPLETE;
                    PostSPIFollowerSM(PostEvent);
                } break;
                case 'e':
                {
                    printf("KeyboardService: posting SPI_TASK_FAILED to SPIFollowerSM\n\r");
                    PostEvent.EventType = SPI_TASK_FAILED;
                    PostSPIFollowerSM(PostEvent);
                } break;
                default:
                {
                    printf("KeyboardService: No Event bound to %c. Press '?' to see list of valid keys.\r\n", (char) ThisEvent.EventParam);
                } break;
            }
        }
        break;       
        default:
        ;
    }
    return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/
/****************************************************************************
 * Function
 *      PrintInstructions
 *      
 * Parameters
 *      void
 * Return
 *      void
 * Description
 *      Prints all the instructions of what each keypress does
****************************************************************************/
void PrintInstructions(void)
{
    printf( "\n\n---------------------------------------------------------\r\n");
    printf( "Press '?' to print Key Press meanings again\n\r");
    printf( "Press 'q' to post SPI_COMMAND_RECEIVED \n\r");
    printf( "Press 'w' to post SPI_TASK_COMPLETE \n\r");
    printf( "Press 'e' to post SPI_TASK_FAILED \n\r");
    printf( "---------------------------------------------------------\r\n\n");
}

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

