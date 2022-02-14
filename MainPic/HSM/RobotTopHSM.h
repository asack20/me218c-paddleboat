/****************************************************************************
 Template header file for Hierarchical Sate Machines AKA StateCharts

 ****************************************************************************/

#ifndef RobotTopHSM_H
#define RobotTopHSM_H

// State definitions for use with the query function
typedef enum { ROBOT_INIT_STATE, ROBOT_INACTIVE_STATE, ROBOT_ACTIVE_STATE } RobotTopHSMState_t ;

// Public Function Prototypes

ES_Event_t RunRobotTopHSM( ES_Event_t CurrentEvent );
void StartRobotTopHSM ( ES_Event_t CurrentEvent );
bool PostRobotTopHSM( ES_Event_t ThisEvent );
bool InitRobotTopHSM ( uint8_t Priority );
RobotTopHSMState_t  QueryRobotTopHSM ( void );

#endif /*RobotTopHSM_H */

