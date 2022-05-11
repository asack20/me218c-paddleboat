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
#include "../Propulsion/Propulsion.h"
#include "../Propulsion/MotorControlDriver.h"
#include "../FrameworkHeaders/ES_Timers.h"


/*----------------------------- Module Defines ----------------------------*/
#define PROPULSION_INCREMENT 1 // was 5
#define FULL_THRUST 127
#define HALF_THRUST 64

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/
void PrintInstructions(void);
/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;

static ArcadeControl_t input;


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
    
    input.Total = 0;

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

                case '1':
                {
                    printf("KeyboardService: posting PROPULSION_REFUEL to Propulsion\n\r");
                    PostEvent.EventType = PROPULSION_REFUEL;
                    PostPropulsion(PostEvent);
                } break;
                case '2':
                {
                    printf("KeyboardService: posting PAIRING_COMPLETE to Propulsion\n\r");
                    PostEvent.EventType = PAIRING_COMPLETE;
                    PostPropulsion(PostEvent);
                } break;
                case '3':
                {
                    printf("KeyboardService: posting WAIT_TO_PAIR to Propulsion\n\r");
                    PostEvent.EventType = WAIT_TO_PAIR;
                    PostPropulsion(PostEvent);
                } break;
                
                case '4':
                {
                    printf("KeyboardService: Setting Thrust to 0\n\r");
                    input.X = 0;
                    input.Yaw = 0;
                    PostEvent.EventType = PROPULSION_SET_THRUST;
                    PostEvent.EventParam = input.Total;
                    PostPropulsion(PostEvent);
                } break;
                case '5':
                {
                    printf("KeyboardService: Setting Thrust to Full Forward\n\r");
                    input.X = FULL_THRUST;
                    input.Yaw = 0;
                    PostEvent.EventType = PROPULSION_SET_THRUST;
                    PostEvent.EventParam = input.Total;
                    PostPropulsion(PostEvent);
                } break;
                case '6':
                {
                    printf("KeyboardService: Setting Thrust to Half Forward\n\r");
                    input.X = HALF_THRUST;
                    input.Yaw = 0;
                    PostEvent.EventType = PROPULSION_SET_THRUST;
                    PostEvent.EventParam = input.Total;
                    PostPropulsion(PostEvent);
                } break;
                case '7':
                {
                    printf("KeyboardService: Setting Thrust to Full Backward\n\r");
                    input.X = -FULL_THRUST;
                    input.Yaw = 0;
                    PostEvent.EventType = PROPULSION_SET_THRUST;
                    PostEvent.EventParam = input.Total;
                    PostPropulsion(PostEvent);
                } break;
                case '8':
                {
                    printf("KeyboardService: Setting Thrust to Half Backward\n\r");
                    input.X = -HALF_THRUST;
                    input.Yaw = 0;
                    PostEvent.EventType = PROPULSION_SET_THRUST;
                    PostEvent.EventParam = input.Total;
                    PostPropulsion(PostEvent);
                } break;
                case '9':
                {
                    printf("KeyboardService: Setting Thrust to Full CW Turn\n\r");
                    input.X = 0;
                    input.Yaw = FULL_THRUST;
                    PostEvent.EventType = PROPULSION_SET_THRUST;
                    PostEvent.EventParam = input.Total;
                    PostPropulsion(PostEvent);
                } break;
                case '0':
                {
                    printf("KeyboardService: Setting Thrust to Half CW Turn\n\r");
                    input.X = 0;
                    input.Yaw = HALF_THRUST;
                    PostEvent.EventType = PROPULSION_SET_THRUST;
                    PostEvent.EventParam = input.Total;
                    PostPropulsion(PostEvent);
                } break;
                case '-':
                {
                    printf("KeyboardService: Setting Thrust to Full CCW Turn\n\r");
                    input.X = 0;
                    input.Yaw = -FULL_THRUST;
                    PostEvent.EventType = PROPULSION_SET_THRUST;
                    PostEvent.EventParam = input.Total;
                    PostPropulsion(PostEvent);
                } break;
                case '=':
                {
                    printf("KeyboardService: Setting Thrust to Half CCW Turn\n\r");
                    input.X = 0;
                    input.Yaw = -HALF_THRUST;
                    PostEvent.EventType = PROPULSION_SET_THRUST;
                    PostEvent.EventParam = input.Total;
                    PostPropulsion(PostEvent);
                } break;
                
                case 'q':
                {
                    input.X += PROPULSION_INCREMENT;
                    printf("KeyboardService: Increasing X by %d. X: %d, Yaw: %d\n\r", 
                            PROPULSION_INCREMENT, input.X, input.Yaw );
                    PostEvent.EventType = PROPULSION_SET_THRUST;
                    PostEvent.EventParam = input.Total;
                    PostPropulsion(PostEvent);
                } break;
                case 'a':
                {
                    input.X -= PROPULSION_INCREMENT;
                    printf("KeyboardService: Decreasing X by %d. X: %d, Yaw: %d\n\r", 
                            PROPULSION_INCREMENT, input.X, input.Yaw );
                    PostEvent.EventType = PROPULSION_SET_THRUST;
                    PostEvent.EventParam = input.Total;
                    PostPropulsion(PostEvent);
                } break;
                case 'w':
                {
                    input.Yaw += PROPULSION_INCREMENT;
                    printf("KeyboardService: Increasing Yaw by %d. X: %d, Yaw: %d\n\r", 
                            PROPULSION_INCREMENT, input.X, input.Yaw );
                    PostEvent.EventType = PROPULSION_SET_THRUST;
                    PostEvent.EventParam = input.Total;
                    PostPropulsion(PostEvent);
                } break;
                case 's':
                {
                    input.Yaw -= PROPULSION_INCREMENT;
                    printf("KeyboardService: Decreasing Yaw by %d. X: %d, Yaw: %d\n\r", 
                            PROPULSION_INCREMENT, input.X, input.Yaw );
                    PostEvent.EventType = PROPULSION_SET_THRUST;
                    PostEvent.EventParam = input.Total;
                    PostPropulsion(PostEvent);
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
    
    printf( "\n\n------------Propulsion State Control--------------\r\n");
    printf( "Press '1' to Refuel (PROPULSION_REFUEL)\n\r");
    printf( "Press '2' to post PAIRING_COMPLETE to Propulsion\n\r");
    printf( "Press '3' to post WAIT_TO_PAIR to Propulsion\n\r");
    
    printf( "\n\n------------PROPULSION_SET_THRUST--------------\r\n");
    printf( "Press '4' to Set Thrust to 0 \n\r");
    printf( "Press '5' to Set Thrust to Full Forward\n\r");
    printf( "Press '6' to Set Thrust to Half Forward\n\r");
    printf( "Press '7' to Set Thrust to Full Backward\n\r");
    printf( "Press '8' to Set Thrust to Half Backward\n\r");
    printf( "Press '9' to Set Thrust to Full CW Turn\n\r");
    printf( "Press '0' to Set Thrust to Half CW Turn\n\r");
    printf( "Press '-' to Set Thrust to Full CCW Turn\n\r");
    printf( "Press '=' to Set Thrust to Half CCW Turn\n\r\r");
    
    printf( "Press 'q' to Increase X by %d\n\r", PROPULSION_INCREMENT);
    printf( "Press 'a' to Decrease X by %d\n\r", PROPULSION_INCREMENT);
    printf( "Press 'w' to Increase Yaw by %d\n\r", PROPULSION_INCREMENT);
    printf( "Press 's' to Decrease Yaw by %d\n\r", PROPULSION_INCREMENT);
    
}


/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

