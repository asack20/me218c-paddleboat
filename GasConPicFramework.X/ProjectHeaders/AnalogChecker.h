/****************************************************************************
 Module
     AnalogChecker.h
 Description
     header file for the event checking functions
 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 10/18/15 11:50 jec      added #include for stdint & stdbool
 08/06/13 14:37 jec      started coding
*****************************************************************************/

#ifndef AnalogChecker_H
#define AnalogChecker_H

// the common headers for C99 types
#include <stdint.h>
#include <stdbool.h>

// prototypes for event checkers
typedef enum
{
    Braid1Low, Braid1Mid, Braid1High
}Braid1State;

typedef enum
{
    Braid2Low, Braid2Mid, Braid2High
}Braid2State;

typedef enum
{
    Braid3Low, Braid3Mid, Braid3High
}Braid3State;


typedef union
{
    struct {
        uint16_t Unused : 10;
        Braid1State Braid1 : 2;
        Braid2State Braid2 : 2;
        Braid3State Braid3 : 2;
    };
    uint16_t FullState;
}FullBraidState;

void InitADC(void);
void InitBraidStatus(void);
bool CheckBraid(void);

#endif /* AnalogChecker_H */
