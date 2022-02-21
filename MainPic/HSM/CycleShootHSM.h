/****************************************************************************
 Template header file for Hierarchical Sate Machines AKA StateCharts
 02/08/12 adjsutments for use with the Events and Services Framework Gen2
 3/17/09  Fixed prototpyes to use Event_t
 ****************************************************************************/

#ifndef CycleShootHSM_H
#define CycleShootHSM_H


// typedefs for the states
// State definitions for use with the query function
typedef enum { CycleShootEntry, CycleShootTension, CycleShootFire, CycleShootRelease, CycleShootSecure } CycleShootHSMState_t ;


// Public Function Prototypes

ES_Event_t RunCycleShootHSM( ES_Event_t CurrentEvent );
void StartCycleShootHSM ( ES_Event_t CurrentEvent );
CycleShootHSMState_t QueryCycleShootHSM ( void );

#endif /*CycleShootHSM_H */

