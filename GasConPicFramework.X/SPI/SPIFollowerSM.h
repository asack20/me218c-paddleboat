/**************************************************************************** 
 * File:   SPIFollowerSM.h
 * FSM to interface with 2-motor PWM drive train
 * 
 * Author: Afshan Chandani
 * 
 * Created on February 2, 2022, 11:00 PM
 ***************************************************************************/

#ifndef SPIFollowerSM_H
#define SPIFollowerSM_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// typedefs for the states
// State definitions for use with the query function
typedef enum
{
    SPIFollowerInitState, SPIFollowerReceiveState
}SPIFollowerSMState_t;

typedef enum
{
    Stopped, Low, Medium, High
}SPI_Speed_t;

typedef enum
{
    Forward = 0, Backward = 1
}SPI_Direction_t;

typedef enum
{
    Translation = 0, Rotation = 1
}SPI_Drive_Type_t;

typedef enum
{
    SPI_POLL = 0xF, SPI_STOP = 0, SPI_DRIVE_DISTANCE = 1, SPI_DRIVE_UNTIL_BUMP = 2, 
                SPI_DO_TAPE_ALIGN = 3, SPI_DO_BEACON_SWEEP = 4, SPI_BEACON_FOUND = 5, SPI_UNDO_ROTATE = 7
}SPI_MOSI_Event_Name_t;

typedef union
{
    struct {
        uint8_t Data;
        SPI_Speed_t Speed : 2;
        SPI_Direction_t Direction : 1;
        SPI_Drive_Type_t DriveType : 1;
        SPI_MOSI_Event_Name_t Name : 4;
    };
    uint16_t FullCommand;
}SPI_MOSI_Command_t;

typedef enum
{
    SPI_STILL_WORKING = 0xF, SPI_DRIVE_GOAL_REACHED = 1, SPI_BUMP_SUCCESS = 2,
            SPI_TAPE_SUCCESS = 3, SPI_STOP_ACKNOWLEDGED = 4, SPI_BEACON_ACKNOWLEDGED = 5
}SPI_MISO_Event_Name_t;

typedef union
{
	struct{
		uint8_t Data;
        uint8_t Unused : 4;
        SPI_MISO_Event_Name_t Name : 4;
	}CommandBits;
	uint16_t FullCommand;
}SPI_MISO_Command_t;

// Public Function Prototypes

bool InitSPIFollowerSM(uint8_t Priority);
bool PostSPIFollowerSM(ES_Event_t ThisEvent);
ES_Event_t RunSPIFollowerSM(ES_Event_t ThisEvent);
SPIFollowerSMState_t QuerySPIFollowerSM(void);

bool CheckSPIRBF(void);
#endif /* SPIFollowerSM_H */

