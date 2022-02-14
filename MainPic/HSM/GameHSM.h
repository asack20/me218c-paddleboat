/****************************************************************************
 Template header file for Hierarchical Sate Machines AKA StateCharts
 02/08/12 adjsutments for use with the Events and Services Framework Gen2
 3/17/09  Fixed prototpyes to use Event_t
 ****************************************************************************/

#ifndef GameHSM_H
#define GameHSM_H


// typedefs for the states
// State definitions for use with the query function
typedef enum { GAME_STARTUP_STATE, GAME_CYCLE_STATE, GAME_REFILL_STATE } GameHSMState_t ;


// Public Function Prototypes

ES_Event_t RunGameHSM( ES_Event_t CurrentEvent );
void StartGameHSM( ES_Event_t CurrentEvent );
GameHSMState_t QueryGameHSM ( void );

#endif /*GameHSM_H */

