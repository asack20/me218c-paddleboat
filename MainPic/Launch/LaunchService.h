/****************************************************************************

  Header file for Servo LaunchService
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef LaunchService_H
#define LaunchService_H

#include <stdint.h>
#include <stdbool.h>

#include "ES_Events.h"
#include "ES_Port.h"                // needed for definition of REENTRANT
// Public Function Prototypes

bool InitLaunchService(uint8_t Priority);
bool PostLaunchService(ES_Event_t ThisEvent);
ES_Event_t RunLaunchService(ES_Event_t ThisEvent);

#endif /* LaunchService_H */

