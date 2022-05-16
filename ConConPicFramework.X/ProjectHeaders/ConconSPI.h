/**************************************************************************** 
 * File:   ConconSPI.h
 * Implement ConconSPI state machine
 * 
 * Author: Ryan Brandt
 * 
 * Created on May 14, 2022, 9:00 PM
 ***************************************************************************/

#ifndef ConconSPI_H
#define ConconSPI_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// typedefs for the states
// State definitions for use with the query function
typedef enum
{
    SPILeaderInitState, SPILeaderSendState, SPILeaderReceiveState
}ConconSPIState_t;

// Public Function Prototypes

bool InitConconSPI(uint8_t Priority);
bool PostConconSPI(ES_Event_t ThisEvent);
ES_Event_t RunConconSPI(ES_Event_t ThisEvent);
ConconSPIState_t QueryConconSPI(void);

bool QueryRefuelBitForComms(void);

//Event checker
bool CheckSPIRBF(void);

#endif /* SPILeaderSM_H */

