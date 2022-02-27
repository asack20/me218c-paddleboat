/****************************************************************************
 Template header file for Hierarchical Sate Machines AKA StateCharts
 02/08/12 adjsutments for use with the Events and Services Framework Gen2
 3/17/09  Fixed prototpyes to use Event_t
 ****************************************************************************/

#ifndef RehomeSM_H
#define RehomeSM_H


// typedefs for the states
// State definitions for use with the query function
typedef enum { REHOME_INIT_STATE, REHOME_ROTATE_TO_SIDE_STATE, REHOME_DRIVE_TO_WALL_STATE, REHOME_MOVE_FROM_WALL_STATE, REHOME_ROTATE_TO_FORWARD_STATE } RehomeState_t ;


// Public Function Prototypes

ES_Event_t RunRehomeSM( ES_Event_t CurrentEvent );
void StartRehomeSM ( ES_Event_t CurrentEvent );
RehomeState_t QueryRehomeSM ( void );

#endif /*Rehome_H */

