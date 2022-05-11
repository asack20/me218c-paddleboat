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
    BraidLow = 0, BraidMid = 1, BraidHigh = 2
}BraidState;

typedef union
{
    struct {
        uint16_t Unused : 10;
        BraidState Braid1 : 2;
        BraidState Braid2 : 2;
        BraidState Braid3 : 2;
    };
    uint16_t FullState;
}FullBraidState;

void InitADC(void);
void InitBraidStatus(void);
bool CheckBraid(void);

#endif /* AnalogChecker_H */
