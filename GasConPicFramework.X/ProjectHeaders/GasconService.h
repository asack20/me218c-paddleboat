/****************************************************************************

  Header file for Display Flat Sate Machine
  based on the Gen2 Events and Services Framework

 ****************************************************************************/

#ifndef GASCONSERVICE_H
#define GASCONSERVICE_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */
#include "../HALs/PIC32_SPI_HAL.h" // SPI HAL
#include "../HALs/DM_Display_2.h" // Display HAL
#include "terminal.h"
#include "dbprintf.h"

// typedefs for the states
// State definitions for use with the query function
typedef enum
{
  InitGasconPseudoState, InitializeGasconState, GasconReadyState, GasconScrollingState
}GasconState_t;

// Public Function Prototypes

bool InitGasconService(uint8_t Priority);
bool PostGasconService(ES_Event_t ThisEvent);
ES_Event_t RunGasconService(ES_Event_t ThisEvent);
GasconState_t QueryGasconService(void);

#endif /* GASCONSERVICE_H */

