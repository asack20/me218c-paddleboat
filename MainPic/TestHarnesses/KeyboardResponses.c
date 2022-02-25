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
#include "../HSM/RobotTopHSM.h"
#include "../HSM/GameHSM.h"
#include "../HSM/StartupHSM.h"
#include "../HSM/CycleHSM.h"
#include "../Sensors/Find_Beacon.h"
#include "../SPI/SPILeaderSM.h"
#include "../Launch/LaunchService.h"
#include "../HSM/CycleShootHSM.h"
#include "ES_Port.h"
#include "terminal.h"
#include "dbprintf.h"
#include <string.h>

/*----------------------------- Module Defines ----------------------------*/

#define TURN_RED_ON LATBbits.LATB11 = 1
#define TURN_RED_OFF LATBbits.LATB11 = 0
#define TURN_BLUE_ON LATBbits.LATB15 = 1
#define TURN_BLUE_OFF LATBbits.LATB15 = 0

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static uint8_t RedState = 0;
static uint8_t BlueState = 0;

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
                  puts("ROBOT_INIT_COMPLETE:                             \'I\'\r");
                  puts("START_BUTTON_PRESSED:                            \'s\'\r");
                  puts("GAME_STARTUP_COMPLETE:                           \'S\'\r");
                  puts("GAME_CYCLE_COMPLETE:                             \'C\'\r");
                  puts("REFILL_BUTTON_PRESSED:                           \'r\'\r");
                  puts("STARTUP_STEP_COMPLETE:                           \'a\'\r");
                  puts("DRIVE_GOAL_REACHED:                              \'R\'\r");
                  puts("BEACON_FOUND:                                    \'B\'\r");
                  puts("RELOAD_COMPLETE:                                 \'L\'\r");
                  puts("SHOT_COMPLETE:                                   \'H\'\r");
                  puts("DRIVE_STOP_MOTORS:                               \'D\'\r");
                  puts("INITIATE_DRIVE:                                  \'d\'\r");
                  puts("FIND_BEACON:                                     \'b\'\r");
                  puts("GIVE_UP:                                         \'g\'\r");
                  puts("FIND_TAPE:                                       \'t\'\r");
                  puts("TAPE_FOUND:                                      \'T\'\r");
                  puts("SEND_SPI_COMMAND:                                \'p\'\r");
                  puts("SPI_RESPONSE_RECEIVED:                           \'P\'\r");
                  puts("GAME_TIMEOUT:                                    \'o\'\r");
                  puts("FIND_BEACON to determine team (only to BeaconSM) \'k\'\r");
                  puts("FIND_BEACON to align          (only to BeaconSM) \'K\'\r");
                  
                  puts("FLAG_DOWN:                                       \'z\'\r");
                  puts("FLAG_UP:                                         \'Z\'\r");
                  puts("RELOAD_OUT:                                      \'q\'\r");
                  puts("RELOAD_IN:                                       \'Q\'\r");
                  puts("LATCH_ENGAGE:                                    \'w\'\r");
                  puts("LATCH_RELEASE:                                   \'W\'\r");
                  puts("TENSION_ENGAGE:                                  \'e\'\r");
                  puts("TENSION_RELEASE:                                 \'E\'\r");
                  puts("Drive Forward Medium Speed                       \'m\'\r");
                  puts("Rotate Clockwise at Medium Speed                 \'M\'\r");
                  puts("Drive Backward Medium Speed                      \'n\'\r");
                  puts("Rotate Counterclockwise at Medium Speed          \'N\'\r");
                  puts("Toggle Red                                       \'j\'\r");
                  puts("Toggle Blue                                      \'J\'\r");
                          
                  puts("Query State of RobotTopHSM:                      \'1\'\r");
                  puts("Query State of GameHSM:                          \'2\'\r");
                  puts("Query State of StartupHSM:                       \'3\'\r");
                  puts("Query State of CycleHSM:                         \'4\'\r");
                  puts("Query State of DriveTrainHSM:                    \'5\'\r");
                  puts("Query State of BeaconSM:                         \'6\'\r");
                  puts("Query State of CycleShootHSM                     \'7\'\r");
                  puts("Query State of SPILeaderSM:                      \'8\'\r");
                  puts("----------------------------------------------------\r\n");
              }
              break;
              
              case 'I':
              {
                puts("Posting ROBOT_INIT_COMPLETE Event to RobotTopHSM\r\n");
                NewEvent.EventType = ROBOT_INIT_COMPLETE;
                PostRobotTopHSM(NewEvent);
              }
              break;
              
              case 's':
              {
                puts("Posting START_BUTTON_PRESSED Event to RobotTopHSM\r\n");
                NewEvent.EventType = START_BUTTON_PRESSED;
                PostRobotTopHSM(NewEvent);
              }
              break;
              
              case 'S':
              {
                puts("Posting GAME_STARTUP_COMPLETE Event to GameHSM\r\n");
                NewEvent.EventType = GAME_STARTUP_COMPLETE;
                PostRobotTopHSM(NewEvent);
              }
              break;
              
              case 'C':
              {
                puts("Posting GAME_CYCLE_COMPLETE Event to GameHSM\r");
                NewEvent.EventType = GAME_CYCLE_COMPLETE;
                PostRobotTopHSM(NewEvent);
              }
              break;
              
              case 'r':
              {
                puts("Posting REFILL_BUTTON_PRESSED Event to GameHSM\r");
                NewEvent.EventType = REFILL_BUTTON_PRESSED;
                PostRobotTopHSM(NewEvent);
              }
              break;
              
              case 'a':
              {
                puts("Posting STARTUP_STEP_COMPLETE Event to StartupHSM\r\n");
                NewEvent.EventType = STARTUP_STEP_COMPLETE;
                PostRobotTopHSM(NewEvent);
              }
              break;
              
              case 'R':
              {
                puts("Posting DRIVE_GOAL_REACHED Event to CycleHSM\r\n");
                NewEvent.EventType = DRIVE_GOAL_REACHED;
                PostRobotTopHSM(NewEvent);
              }
              break;
              
              case 'B':
              {
                puts("Posting BEACON_FOUND Event to StartupHSM and CycleHSM\r\n");
                NewEvent.EventType = BEACON_FOUND;
                PostRobotTopHSM(NewEvent);
              }
              break;
              
              case 'L':
              {
                puts("Posting RELOAD_COMPLETE Event to CycleHSM\r\n");
                NewEvent.EventType = RELOAD_COMPLETE;
                PostRobotTopHSM(NewEvent);
              }
              break;
              
              case 'H':
              {
                puts("Posting SHOT_COMPLETE Event to CycleHSM\r\n");
                NewEvent.EventType = SHOT_COMPLETE;
                PostRobotTopHSM(NewEvent);
              }
              break;
              
              case 'D':
              {
                puts("Stopping Motors\r\n");
                SPI_MOSI_Command_t NewCommand;
                NewCommand.Name = SPI_STOP;
                NewEvent.EventType = SEND_SPI_COMMAND;
                NewEvent.EventParam = NewCommand.FullCommand;
                PostSPILeaderSM(NewEvent);
              }
              break;
              
              case 'd':
              {
                NewEvent.EventType = INITIATE_DRIVE;
              }
              break;
              
              case 'b':
              {
                NewEvent.EventType = FIND_BEACON;
              }
              break;
              
              case 'g':
              {
                NewEvent.EventType = GIVE_UP;
              }
              break;
              
              case 't':
              {
                NewEvent.EventType = FIND_TAPE;
              }
              break;
              
              case 'T':
              {
                NewEvent.EventType = TAPE_FOUND;
              }
              break;
              
              case 'p':
              {
                puts("Posting SEND_SPI_COMMAND Event to SPILeaderSM with a param of 0xABCD \r\n");
                NewEvent.EventType = SEND_SPI_COMMAND;
                NewEvent.EventParam = 0xABCD;
                PostSPILeaderSM(NewEvent);
              }
              break;
              
              case 'P':
              {
                puts("Posting SPI_RESPONSE_RECEIVED Event to SPILeaderSM with a param of 0x1000 \r\n");
                NewEvent.EventType = SPI_RESPONSE_RECEIVED;
                NewEvent.EventParam = 0x1000;
                PostSPILeaderSM(NewEvent);
              }
              break;
              
              case 'o':
              {
                puts("Posting GAME_TIMEOUT Event to RobotTopHSM\r\n");
                NewEvent.EventType = GAME_TIMEOUT;
                PostRobotTopHSM(NewEvent);
              }
              break;
              
              case 'k':
              {
                puts("Posting FIND_BEACON Event to BeaconSM\r\n");
                NewEvent.EventType = FIND_BEACON;
                NewEvent.EventParam = DetermineTeam;
                PostFind_Beacon(NewEvent);
              }
              break;
              
              case 'K':
              {
                puts("Posting FIND_BEACON Event to BeaconSM\r\n");
                NewEvent.EventType = FIND_BEACON;
                NewEvent.EventParam = FindKnownFrequency;
                PostFind_Beacon(NewEvent);
              }
              break;
              
              case 'z':
              {
                puts("Posting FLAG_DOWN Event to LaunchService\r\n");
                NewEvent.EventType = FLAG_DOWN;
                PostLaunchService(NewEvent);
              }
              break;
              
              case 'Z':
              {
                puts("Posting FLAG_UP Event to LaunchService\r\n");
                NewEvent.EventType = FLAG_UP;
                PostLaunchService(NewEvent);
              }
              break;
              
              case 'q':
              {
                puts("Posting RELOAD_OUT Event to LaunchService\r\n");
                NewEvent.EventType = RELOAD_OUT;
                PostLaunchService(NewEvent);
              }
              break;
              
              case 'Q':
              {
                puts("Posting RELOAD_IN Event to LaunchService\r\n");
                NewEvent.EventType = RELOAD_IN;
                PostLaunchService(NewEvent);
              }
              break;
              
              case 'w':
              {
                puts("Posting LATCH_ENGAGE Event to LaunchService\r\n");
                NewEvent.EventType = LATCH_ENGAGE;
                PostLaunchService(NewEvent);
              }
              break;
              
              case 'W':
              {
                puts("Posting LATCH_RELEASE Event to LaunchService\r\n");
                NewEvent.EventType = LATCH_RELEASE;
                PostLaunchService(NewEvent);
              }
              break;
              
              case 'e':
              {
                puts("Posting TENSION_ENGAGE Event to LaunchService\r\n");
                NewEvent.EventType = TENSION_ENGAGE;
                PostLaunchService(NewEvent);
              }
              break;
              
              case 'E':
              {
                puts("Posting TENSION_RELEASE Event to LaunchService\r\n");
                NewEvent.EventType = TENSION_RELEASE;
                PostLaunchService(NewEvent);
              }
              break;
              
              case 'm':
              {
                puts("Send Drive Forward at Medium Speed Command to Motor PIC\r\n");
                SPI_MOSI_Command_t NewCommand;
                NewCommand.Name = SPI_DRIVE_DISTANCE;
                NewCommand.DriveType = Translation;
                NewCommand.Direction = Forward_CW;
                NewCommand.Speed = Medium;
                NewCommand.Data = 0;
                NewEvent.EventType = SEND_SPI_COMMAND;
                NewEvent.EventParam = NewCommand.FullCommand;
                PostSPILeaderSM(NewEvent);
              }
              break;
              
              case 'M':
              {
                puts("Send Rotate Clockwise at Medium Speed Command to Motor PIC\r\n");
                SPI_MOSI_Command_t NewCommand;
                NewCommand.Name = SPI_DRIVE_DISTANCE;
                NewCommand.DriveType = Rotation;
                NewCommand.Direction = Forward_CW;
                NewCommand.Speed = Medium;
                NewCommand.Data = 0;
                NewEvent.EventType = SEND_SPI_COMMAND;
                NewEvent.EventParam = NewCommand.FullCommand;
                PostSPILeaderSM(NewEvent);
              }
              break;
              
              case 'n':
              {
                puts("Send Drive Backward at Medium Speed Command to Motor PIC\r\n");
                SPI_MOSI_Command_t NewCommand;
                NewCommand.Name = SPI_DRIVE_DISTANCE;
                NewCommand.DriveType = Translation;
                NewCommand.Direction = Backward_CCW;
                NewCommand.Speed = Medium;
                NewCommand.Data = 0;
                NewEvent.EventType = SEND_SPI_COMMAND;
                NewEvent.EventParam = NewCommand.FullCommand;
                PostSPILeaderSM(NewEvent);
              }
              break;
              
              case 'N':
              {
                puts("Send Rotate Counterclockwise at Medium Speed Command to Motor PIC\r\n");
                SPI_MOSI_Command_t NewCommand;
                NewCommand.Name = SPI_DRIVE_DISTANCE;
                NewCommand.DriveType = Rotation;
                NewCommand.Direction = Backward_CCW;
                NewCommand.Speed = Medium;
                NewCommand.Data = 0;
                NewEvent.EventType = SEND_SPI_COMMAND;
                NewEvent.EventParam = NewCommand.FullCommand;
                PostSPILeaderSM(NewEvent);
              }
              break;
              
              case 'j':
              {
                  if (RedState == 0) {
                      RedState = 1;
                      puts("Turning on Red\r\n");
                      TURN_RED_ON;
                  }
                  else {
                      RedState = 0;
                      puts("Turning off Red\r\n");
                      TURN_RED_OFF;
                  }
              }
              break;
              
              case 'J':
              {
                  if (BlueState == 0) {
                      BlueState = 1;
                      puts("Turning on Blue\r\n");
                      TURN_BLUE_ON;
                  }
                  else {
                      BlueState = 0;
                      puts("Turning off Blue\r\n");
                      TURN_BLUE_OFF;
                  }
              }
              break;
              
              case '1':
              {
                RobotTopHSMState_t CurrentRobotTopHSMState;
                CurrentRobotTopHSMState = QueryRobotTopHSM();
                char StateChar[40];
                
                switch (CurrentRobotTopHSMState)
                {
                    case ROBOT_INIT_STATE:
                    {
                        strcpy(StateChar,"RobotInitState");
                    }
                    break;
                    
                    case ROBOT_INACTIVE_STATE:
                    {
                        strcpy(StateChar,"RobotInactiveState");
                    }
                    break;
                    
                    case ROBOT_ACTIVE_STATE:
                    {
                        strcpy(StateChar,"RobotActiveState");
                    }
                    break;
                    
                    default:
                    {
                        strcpy(StateChar,"Not in a valid state - ERROR");
                    }
                    break;
                }
                puts("Querying RobotTopHSM:\r");
                DB_printf("RobotTopHSM is in state %s\r\n\n",StateChar);
              }
              break;
              
              case '2':
              {
                GameHSMState_t CurrentGameHSMState;
                CurrentGameHSMState = QueryGameHSM();
                char StateChar[40];
                
                switch (CurrentGameHSMState)
                {
                    case GAME_STARTUP_STATE:
                    {
                        strcpy(StateChar,"GameStartupState");
                    }
                    break;
                    
                    case GAME_CYCLE_STATE:
                    {
                        strcpy(StateChar,"GameCycleState");
                    }
                    break;
                    
                    case GAME_REFILL_STATE:
                    {
                        strcpy(StateChar,"GameRefillState");
                    }
                    break;
                    
                    default:
                    {
                        strcpy(StateChar,"Not in a valid state - ERROR");
                    }
                    break;
                }
                puts("Querying GameHSM:\r");
                DB_printf("GameHSM is in state %s\r\n\n",StateChar);
              }
              break;

              case '3':
              {
                StartupHSMState_t CurrentStartupHSMState;
                CurrentStartupHSMState = QueryStartupHSM();
                char StateChar[40];
                
                switch (CurrentStartupHSMState)
                {
                    case STARTUP_INIT_STATE:
                    {
                        strcpy(StateChar,"StartupInitState");
                    }
                    break;
                    
                    case FIND_BEACON_STATE:
                    {
                        strcpy(StateChar,"FindBeaconState");
                    }
                    break;
                    
                    case DETERMINE_TEAM_STATE:
                    {
                        strcpy(StateChar,"DetermineTeamState");
                    }
                    break;
                    
                    case ROTATE_TO_SIDE_STATE:
                    {
                        strcpy(StateChar,"RotateToSideState");
                    }
                    break;
                    
                    case DRIVE_TO_WALL_STATE:
                    {
                        strcpy(StateChar,"DriveToWallState");
                    }
                    break;
                    
                    case ROTATE_TO_FORWARD_STATE:
                    {
                        strcpy(StateChar,"RotateToForwardState");
                    }
                    break;
                    
                    default:
                    {
                        strcpy(StateChar,"Not in a valid state - ERROR");
                    }
                    break;
                }
                puts("Querying StartupHSM:\r");
                DB_printf("StartupHSM is in state %s\r\n\n",StateChar);
              }
              break;
              
              case '4':
              {
                CycleHSMState_t CurrentCycleHSMState;
                CurrentCycleHSMState = QueryCycleHSM();
                char StateChar[40];
                
                switch (CurrentCycleHSMState)
                {
                    case CYCLE_DRIVE_FORWARD_STATE:
                    {
                        strcpy(StateChar,"CycleDriveForwardState");
                    }
                    break;
                    
                    case CYCLE_AIM_STATE:
                    {
                        strcpy(StateChar,"CycleAimState");
                    }
                    break;
                    
                    case CYCLE_RELOAD_STATE:
                    {
                        strcpy(StateChar,"CycleReloadState");
                    }
                    break;
                    
                    case CYCLE_SHOOT_STATE:
                    {
                        strcpy(StateChar,"CycleShootState");
                    }
                    break;
                    
                    case CYCLE_UNDO_ROTATION_STATE:
                    {
                        strcpy(StateChar,"CycleUndoRotationState");
                    }
                    
                    case CYCLE_DRIVE_BACK_STATE:
                    {
                        strcpy(StateChar,"CycleDriveBackState");
                    }
                    break;
                    
                    default:
                    {
                        strcpy(StateChar,"Not in a valid state - ERROR");
                    }
                    break;
                }
                puts("Querying CycleHSM:\r");
                DB_printf("CycleHSM is in state %s\r\n\n",StateChar);
              }
              break;
              
              case '6':
              {
                Find_BeaconState_t CurrentFind_BeaconState;
                CurrentFind_BeaconState = QueryFind_Beacon();
                char StateChar[40];
                
                switch (CurrentFind_BeaconState)
                {
                    case Waiting1:
                    {
                        strcpy(StateChar,"Waiting");
                    }
                    break;
                    
                    case Searching1:
                    {
                        strcpy(StateChar,"Searching");
                    }
                    break;
                    
                    default:
                    {
                        strcpy(StateChar,"Not in a valid state - ERROR");
                    }
                    break;
                }
                puts("Querying BeaconSM:\r");
                DB_printf("BeaconSM is in state %s\r\n\n",StateChar);
              }
              break;
              
              case '7':
              {
                CycleShootHSMState_t CurrentCycleShootHSMState;
                CurrentCycleShootHSMState = QueryCycleShootHSM();
                char StateChar[40];
                
                switch (CurrentCycleShootHSMState)
                {
                    case CycleShootEntry:
                    {
                        strcpy(StateChar,"Entry");
                    }
                    break;
                    
                    case CycleShootTension:
                    {
                        strcpy(StateChar,"Tension");
                    }
                    break;
                    
                    case CycleShootFire:
                    {
                        strcpy(StateChar,"Fire");
                    }
                    break;
                    
                    case CycleShootRelease:
                    {
                        strcpy(StateChar,"Release");
                    }
                    break;
                    
                    case CycleShootSecure:
                    {
                        strcpy(StateChar,"Secure");
                    }
                    break;
                    
                    default:
                    {
                        strcpy(StateChar,"Not in a valid state - ERROR");
                    }
                    break;
                }
                puts("Querying CycleShootHSM:\r");
                DB_printf("CycleShootHSM is in state %s\r\n\n",StateChar);
              }
              break;
              
              case '8':
              {
                SPILeaderSMState_t CurrentSPILeaderSMState;
                CurrentSPILeaderSMState = QuerySPILeaderSM();
                char StateChar[40];
                
                switch (CurrentSPILeaderSMState)
                {
                    case SPILeaderInitState:
                    {
                        strcpy(StateChar,"Init");
                    }
                    break;
                    
                    case SPILeaderSendState:
                    {
                        strcpy(StateChar,"SendState");
                    }
                    break;
                    
                    case SPILeaderReceiveState:
                    {
                        strcpy(StateChar,"ReceiveState");
                    }
                    break;
                            
                    default:
                    {
                        strcpy(StateChar,"Not in a valid state - ERROR");
                    }
                    break;
                }
                puts("Querying SPILeaderSM:\r");
                DB_printf("SPILeaderSM is in state %s\r\n\n",StateChar);
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

