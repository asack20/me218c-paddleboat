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
    BraidWhite = 0, BraidGreen = 1, BraidBlue = 2, BraidRed = 3, BraidOpen = 4
}BraidState;

typedef union
{
    struct {
        uint16_t Unused : 7;
        BraidState Braid1 : 3;
        BraidState Braid2 : 3;
        BraidState Braid3 : 3;
    };
    uint16_t FullState;
}FullBraidState;

void InitADC(void);
void InitBraidStatus(void);
bool CheckBraid(void);

#endif /* AnalogChecker_H */
