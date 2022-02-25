/****************************************************************************
 Template header file for Hierarchical Sate Machines AKA StateCharts
 02/08/12 adjsutments for use with the Events and Services Framework Gen2
 3/17/09  Fixed prototpyes to use Event_t
 ****************************************************************************/

#ifndef CycleHSM_H
#define CycleHSM_H


// typedefs for the states
// State definitions for use with the query function
typedef enum { CYCLE_DRIVE_FORWARD_STATE, CYCLE_AIM_STATE, CYCLE_RELOAD_STATE, CYCLE_SHOOT_STATE, CYCLE_UNDO_ROTATION_STATE, CYCLE_DRIVE_BACK_STATE , CYCLE_STOPPING_STATE} CycleHSMState_t ;


// Public Function Prototypes

ES_Event_t RunCycleHSM( ES_Event_t CurrentEvent );
void StartCycleHSM ( ES_Event_t CurrentEvent );
CycleHSMState_t QueryCycleHSM ( void );

#endif /*CycleHSM_H */

