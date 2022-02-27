/****************************************************************************
 Template header file for Hierarchical Sate Machines AKA StateCharts
 02/08/12 adjsutments for use with the Events and Services Framework Gen2
 3/17/09  Fixed prototpyes to use Event_t
 ****************************************************************************/

#ifndef StartupHSM_H
#define StartupHSM_H


// typedefs for the states
// State definitions for use with the query function
typedef enum { STARTUP_INIT_STATE, FIND_BEACON_STATE, DETERMINE_TEAM_STATE, ROTATE_TO_SIDE_STATE, DRIVE_TO_WALL_STATE, MOVE_FROM_WALL_STATE, ROTATE_TO_FORWARD_STATE } StartupHSMState_t ;


// Public Function Prototypes

ES_Event_t RunStartupHSM( ES_Event_t CurrentEvent );
void StartStartupHSM ( ES_Event_t CurrentEvent );
StartupHSMState_t QueryStartupHSM ( void );

#endif /*SHMTemplate_H */

