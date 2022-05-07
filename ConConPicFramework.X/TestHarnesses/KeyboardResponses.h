/****************************************************************************

  Header file for KeyboardResponses service
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef KeyboardResponses_H
#define KeyboardResponses_H

#include "ES_Types.h"

// Public Function Prototypes

bool InitKeyboardResponses(uint8_t Priority);
bool PostKeyboardResponses(ES_Event_t ThisEvent);
ES_Event_t RunKeyboardResponses(ES_Event_t ThisEvent);

#endif /* KeyboardResponses_H */

