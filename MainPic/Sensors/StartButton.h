/**************************************************************************** 
 * File:   StartButton.h
 * FSM to handle start button
 * 
 * Author: Afshan Chandani
 * 
 * Created on February 23, 2022, 11:00 PM
 ***************************************************************************/

#ifndef StartButton_H
#define StartButton_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// typedefs for the states
// State definitions for use with the query function
typedef enum
{
    StartButtonLow, StartButtonHigh
}StartButtonState_t;


// Public Function Prototypes

bool InitStartButton(uint8_t Priority);
bool PostStartButton(ES_Event_t ThisEvent);
ES_Event_t RunStartButton(ES_Event_t ThisEvent);
StartButtonState_t QueryStartButton(void);
bool CheckStartButtonEvents(void);
#endif /* StartButton_H */

