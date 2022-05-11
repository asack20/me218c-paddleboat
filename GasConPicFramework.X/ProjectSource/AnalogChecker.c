/****************************************************************************
 Module
   AnalogChecker.c

 Revision
   1.0.1

 Description
   This is the analog event checker

 Notes
   Note the use of static variables in sample event checker to detect
   ONLY transitions.

****************************************************************************/

// this will pull in the symbolic definitions for events, which we will want
// to post in response to detecting events
#include "ES_Configure.h"
// This gets us the prototype for ES_PostAll
#include "ES_Framework.h"
// this will get us the structure definition for events, which we will need
// in order to post events in response to detecting events
#include "ES_Events.h"
// if you want to use distribution lists then you need those function
// definitions too.
#include "ES_PostList.h"
// This include will pull in all of the headers from the service modules
// providing the prototypes for all of the post functions
#include "ES_ServiceHeaders.h"
// this test harness for the framework references the serial routines that
// are defined in ES_Port.c
#include "ES_Port.h"
// include our own prototypes to insure consistency between header &
// actual functions definition
#include "AnalogChecker.h"

#include "../HALs/PIC32PortHAL.h"
#include "../HALs/PIC32_AD_Lib.h"
#include "../FrameworkHeaders/bitdefs.h"

// This is the event checking function sample. It is not intended to be
// included in the module. It is only here as a sample to guide you in writing
// your own event checkers
/****************************************************************************
****************************************************************************/

#define BRAID1_AN BIT1HI
#define BRAID1_IDX 0
#define BRAID2_AN BIT4HI
#define BRAID2_IDX 1
#define BRAID3_AN BIT5HI
#define BRAID3_IDX 2

// Distance Sensor
static uint32_t ConversionResults[3];
static FullBraidState CurrentBraidState;

/****************************************************************************
 Function
 * InitADC

 Parameters
     None

 Returns
 * void

 Description
 Initializes ADC Library
 
 Notes

 Author
 * Andrew Sack
****************************************************************************/
void InitADC(void)
{
    ADC_ConfigAutoScan((BRAID1_AN | BRAID2_AN | BRAID3_AN), 2);
}

void InitBraidStatus(void)
{
    PortSetup_ConfigureAnalogInputs(_Port_A, _Pin_1);
    PortSetup_ConfigureAnalogInputs(_Port_B, _Pin_2);
    PortSetup_ConfigureAnalogInputs(_Port_B, _Pin_3);
    InitADC();
    CurrentBraidState.FullState = 0;
    printf("Analog Checker INITIALIZED\r\n");
    return;
}

bool CheckBraid(void)
{
    bool ReturnVal = false;
    uint32_t Braid1Val;
    uint32_t Braid2Val;
    uint32_t Braid3Val;

    ADC_MultiRead(ConversionResults);
    Braid1Val = ConversionResults[BRAID1_IDX];
    Braid2Val = ConversionResults[BRAID2_IDX];
    Braid3Val = ConversionResults[BRAID3_IDX];
    CurrentBraidState.Braid1 = Braid1Val;
    CurrentBraidState.Braid2 = Braid2Val;
    CurrentBraidState.Braid3 = Braid3Val;
    ES_Event_t ThisEvent;
    ThisEvent.EventType = BRAID_UPDATE;
    ThisEvent.EventParam = CurrentBraidState.FullState;
    //PostBraidService(ThisEvent);
    
    return ReturnVal;
}