/**************************************************************************** 
 * File:   ReloadButton.h
 * FSM to handle start button
 * 
 * Author: Afshan Chandani
 * 
 * Created on February 23, 2022, 11:00 PM
 ***************************************************************************/

#ifndef ReloadButton_H
#define ReloadButton_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// typedefs for the states
// State definitions for use with the query function
typedef enum
{
    ReloadButtonLow, ReloadButtonHigh
}ReloadButtonState_t;


// Public Function Prototypes

bool InitReloadButton(uint8_t Priority);
bool PostReloadButton(ES_Event_t ThisEvent);
ES_Event_t RunReloadButton(ES_Event_t ThisEvent);
ReloadButtonState_t QueryReloadButton(void);
bool CheckReloadButtonEvents(void);
#endif /* ReloadButton_H */

