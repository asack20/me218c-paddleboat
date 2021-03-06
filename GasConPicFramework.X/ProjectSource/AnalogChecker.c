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
#define LOW_THRESH 211
#define MID_THRESH 420
#define HI_THRESH 630
#define OPEN_THRESH 885

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
    //ADC_ConfigAutoScan((BRAID1_AN | BRAID2_AN | BRAID3_AN), 2);
    ADC_ConfigAutoScan((BRAID1_AN | BRAID2_AN | BRAID3_AN), 3);
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
    uint32_t Braid1RawVal;
    uint32_t Braid2RawVal;
    uint32_t Braid3RawVal;
    BraidState Braid1Val;
    BraidState Braid2Val;
    BraidState Braid3Val;

    ADC_MultiRead(ConversionResults);
    Braid1RawVal = ConversionResults[BRAID1_IDX];
    Braid2RawVal = ConversionResults[BRAID2_IDX];
    Braid3RawVal = ConversionResults[BRAID3_IDX];
    
    //Convert these into the Enums
    if(Braid1RawVal <= LOW_THRESH){
        Braid1Val = BraidWhite;
    } else if((Braid1RawVal <= MID_THRESH) && (Braid1RawVal > LOW_THRESH)){
        Braid1Val = BraidGreen;
    } else if((Braid1RawVal <= HI_THRESH) && (Braid1RawVal > MID_THRESH)){
        Braid1Val = BraidBlue;
    } else if((Braid1RawVal <= OPEN_THRESH) && (Braid1RawVal > HI_THRESH)){
        Braid1Val = BraidRed;
    } else {
        Braid1Val = BraidOpen;
    }
    if(Braid2RawVal <= LOW_THRESH){
        Braid2Val = BraidWhite;
    } else if((Braid2RawVal <= MID_THRESH) && (Braid2RawVal > LOW_THRESH)){
        Braid2Val = BraidGreen;
    } else if((Braid2RawVal <= HI_THRESH) && (Braid2RawVal > MID_THRESH)){
        Braid2Val = BraidBlue;
    } else if((Braid2RawVal <= OPEN_THRESH) && (Braid2RawVal > HI_THRESH)){
        Braid2Val = BraidRed;
    } else {
        Braid2Val = BraidOpen;
    }
    if(Braid3RawVal <= LOW_THRESH){
        Braid3Val = BraidWhite;
    } else if((Braid3RawVal <= MID_THRESH) && (Braid3RawVal > LOW_THRESH)){
        Braid3Val = BraidGreen;
    } else if((Braid3RawVal <= HI_THRESH) && (Braid3RawVal > MID_THRESH)){
        Braid3Val = BraidBlue;
    } else if((Braid3RawVal <= OPEN_THRESH) && (Braid3RawVal > HI_THRESH)){
        Braid3Val = BraidRed;
    } else {
        Braid3Val = BraidOpen;
    }
    
    //printf("%d", Braid3Val);
    //printf("Braid Input 2:  %d\r\n", Braid2Val);
    //printf("Braid Input 3:  %d\r\n", Braid3Val);
    
    CurrentBraidState.Braid1 = Braid1Val;
    CurrentBraidState.Braid2 = Braid2Val;
    //CurrentBraidState.Braid3 = Braid3Val;  //Don't include braid 3 anymore
    ES_Event_t ThisEvent;
    ThisEvent.EventType = BRAID_UPDATE;
    ThisEvent.EventParam = CurrentBraidState.FullState;
    PostBraidService(ThisEvent);
    
    return ReturnVal;
}