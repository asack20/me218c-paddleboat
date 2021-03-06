/**************************************************************************** 
 * File:   Propulsion.h
 * FSM to interface with 2-motor PWM drive train and control fuel level
 * 
 * Author: Andrew Sack
 * 
 * Created on May 8, 2022, 06:21 PM
 ***************************************************************************/

#ifndef Propulsion_H
#define Propulsion_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */
#include "ES_Framework.h"

// Data structure for arcade drive control scheme
typedef union {
    struct {
        int8_t X; // LSB
        int8_t Yaw;   
    };
    uint16_t Total;
} ArcadeControl_t;

// Data structure for tank drive control scheme
typedef union {
    struct {
        int8_t Left; // LSB
        int8_t Right;   
    };
    uint16_t Total;
} TankControl_t;

// typedefs for the states
// State definitions for use with the query function
typedef enum
{
    FuelEmptyState, FuelFullState 
}PropulsionState_t;

// Public Function Prototypes

bool InitPropulsion(uint8_t Priority);
bool PostPropulsion(ES_Event_t ThisEvent);
ES_Event_t RunPropulsion(ES_Event_t ThisEvent);
PropulsionState_t QueryPropulsion(void);
uint8_t Propulsion_GetFuelLevel(void);

#endif /* Propulsion_H */

