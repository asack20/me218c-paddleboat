#include "PIC32PortHAL.h"
#include <xc.h>
#include <stdbool.h>
#include "bitdefs.h"

/*Easy Toggle for running test harness code
 * 1 = Run Test Harness
 * 0 = Don't */
#define RUN_TESTS 0 //easy toggle for running test harness at top of file

/* TEST_TYPE - Which function to run test harness for
 * 0 = PortSetup_ConfigureDigitalOutputs
 * 1 = PortSetup_ConfigureDigitalInputs
 * 2 = PortSetup_ConfigureAnalogInputs (NOT IMPLEMENTED)
 * 3 = PortSetup_ConfigurePullUps
 * 4 = PortSetup_ConfigurePullDowns
 * 5 = PortSetup_ConfigureOpenDrain
 * 6 = PortSetup_ConfigureChangeNotification */
#define TEST_TYPE 0

// For test harnesses that only work for 1 pin at a time
// specify port and pin number
#define TEST_PORT _Port_B
#define TEST_PIN _Pin_6

/****************************** Constants *******************************/
//The bits corresponding to each port type are Hi, all else is Low
const uint32_t A_DIGITAL_PINS = _Pin_0 | _Pin_1 | _Pin_2 | _Pin_3 | _Pin_4;
const uint32_t A_ANALOG_PINS = _Pin_0 | _Pin_1;
const uint32_t B_DIGITAL_PINS = 0xFFFF; // All 16 Pins
const uint32_t B_ANALOG_PINS = 0xF00F; //Pins 0-3 and 12-15

/*********************** Static Function Prototypes ************************/
static bool ValidateDigitalPorts(PortSetup_Port_t WhichPort, PortSetup_Pin_t WhichPin);
static bool ValidateAnalogPorts(PortSetup_Port_t WhichPort, PortSetup_Pin_t WhichPin);

/****************************************************************************
 Function
    PortSetup_ConfigureDigitalOutputs

 Parameters
   PortSetup_Port_t: the port to be configured
   PortSetup_Pin_t: the pin(s) to be configured as digital outputs

 Returns
   bool: true if port and pins represent legal ports and pins; otherwise, false

 Description
   Configures the specified pin(s) on the specified port as digital outputs
Example
   PortSetup_ConfigureDigitalOutputs(_Port_A, _Pin_0 | _Pin_1);
****************************************************************************/
bool PortSetup_ConfigureDigitalOutputs( PortSetup_Port_t WhichPort, PortSetup_Pin_t WhichPin)
{
    bool is_valid;
    is_valid = ValidateDigitalPorts(WhichPort, WhichPin);
    
    //Only set registers if all pins are valid
    if (is_valid){ 
        if (WhichPort == _Port_A)
        {
            ANSELACLR = WhichPin; // 0 is digital
            CNPUACLR = WhichPin; // Pull Ups/Downs don't work with output 
            CNPDACLR = WhichPin;
            TRISACLR = WhichPin; // 0 is Output
        }
        else if (WhichPort == _Port_B)
        {
            ANSELBCLR = WhichPin;
            CNPUBCLR = WhichPin;
            CNPDBCLR = WhichPin;
            TRISBCLR = WhichPin;
        } 
    }
    return is_valid;
}

/****************************************************************************
 Function
    PortSetup_ConfigureDigitalInputs

 Parameters
   PortSetup_Port_t: the port to be configured
   PortSetup_Pin_t: the pin to be configured as digital inputs

 Returns
   bool: true if port and pins represent legal ports and pins; otherwise, false

 Description
   Configures the specified pin(s) on the specified port as digital inputs, disabling analog input(s).
Example
   PortSetup_ConfigureDigitalInputs(_Port_A, _Pin_0 | _Pin_1);
****************************************************************************/
bool PortSetup_ConfigureDigitalInputs( PortSetup_Port_t WhichPort, PortSetup_Pin_t WhichPin)
{
    bool is_valid;
    is_valid = ValidateDigitalPorts(WhichPort, WhichPin);
    
    //Only set registers if all pins are valid
    if (is_valid){ 
        if (WhichPort == _Port_A)
        {
            ANSELACLR = WhichPin; // Digital is 0
            TRISASET = WhichPin; // Input is 1
        }
        else if (WhichPort == _Port_B)
        {
            ANSELBCLR = WhichPin;
            TRISBSET = WhichPin;
        }
    }
    
    return is_valid;
}

/****************************************************************************
 Function
    PortSetup_ConfigureAnalogInputs

 Parameters
   PortSetup_Port_t: the port to be configured
   PortSetup_Pin_t: the pin(s) to be configured as analog inputs

 Returns
   bool: true if port and pins represent legal ports and pins; otherwise, false

 Description
   Configures the specified pin(s) on the specified port as analog inputs
Example
   PortSetup_ConfigureAnalogInputs(_Port_A, _Pin_0 | _Pin_1);
****************************************************************************/
bool PortSetup_ConfigureAnalogInputs( PortSetup_Port_t WhichPort, PortSetup_Pin_t WhichPin)
{
    bool is_valid;
    is_valid = ValidateAnalogPorts(WhichPort, WhichPin);
    
    //Only set registers if all pins are valid
    if (is_valid){ 
        if (WhichPort == _Port_A)
        {
            TRISASET = WhichPin; // Input is 1
            ANSELASET = WhichPin; // Analog is 1
        }
        else if (WhichPort == _Port_B)
        {
            TRISBSET = WhichPin;
            ANSELBSET = WhichPin; 
        }
    }
    return is_valid;
}

/****************************************************************************
 Function
    PortSetup_ConfigurePullUps

 Parameters
   PortSetup_Port_t: the port to be configured
   PortSetup_Pin_t: the pin(s) to be configured with weak pull-ups

 Returns
   bool: true if port and pins represent legal ports and pins; otherwise, false

 Description
   Configures the specified pin(s) on the specified port with weak pull-ups
Example
   PortSetup_ConfigurePullUps(_Port_A, _Pin_0 | _Pin_1);
****************************************************************************/
bool PortSetup_ConfigurePullUps( PortSetup_Port_t WhichPort, PortSetup_Pin_t WhichPin)
{
    bool is_valid;
    is_valid = ValidateDigitalPorts(WhichPort, WhichPin);
    
    //Check if pins are inputs
    if (WhichPort == _Port_A)
    {
        //Any pin in WhichPin not set as input
        if (~(TRISAbits.w | ~WhichPin))
        {
            is_valid = false;
        }
    }
    else if (WhichPort == _Port_B)
    {
        //Any pin in WhichPin not set as input
        if (~(TRISBbits.w | ~WhichPin))
        {
            is_valid = false;
        }
    }
 
    // Only set registers if pins were configured as inputs already
    if (is_valid){ 
        if (WhichPort == _Port_A)
        {
            CNPDACLR = WhichPin; // Clear pull down 
            CNPUASET = WhichPin; // set pull up resistor
        }
        else if (WhichPort == _Port_B)
        {
            CNPDBCLR = WhichPin;
            CNPUBSET = WhichPin;
        }
    }
    
    return is_valid;
}

/****************************************************************************
 Function
    PortSetup_ConfigurePullDowns

 Parameters
   PortSetup_Port_t: the port to be configured
   PortSetup_Pin_t: the pin(s) to be configured with weak pull-downs

 Returns
   bool: true if port and pins represent legal ports and pins; otherwise, false

 Description
   Configures the specified pin(s) on the specified port with weak pull-downs. 
Example
   PortSetup_ConfigurePullDowns(_Port_A, _Pin_0 | _Pin_1);
****************************************************************************/
bool PortSetup_ConfigurePullDowns( PortSetup_Port_t WhichPort, PortSetup_Pin_t WhichPin)
{
    bool is_valid;
    is_valid = ValidateDigitalPorts(WhichPort, WhichPin);
    
    //Check if pins are configured as inputs
    if (WhichPort == _Port_A)
    {
        //Any pin in WhichPin not set as input
        if (~(TRISAbits.w | ~WhichPin))
        {
            is_valid = false;
        }
    }
    else if (WhichPort == _Port_B)
    {
        //Any pin in WhichPin not set as input
        if (~(TRISBbits.w | ~WhichPin))
        {
            is_valid = false;
        }
    }
    
    // Only set registers if pins are already inputs
    if (is_valid){ 
        if (WhichPort == _Port_A)
        {
            CNPUACLR = WhichPin; // clear pull up
            CNPDASET = WhichPin; // set pull down resistor
        }
        else if (WhichPort == _Port_B)
        {
            CNPUBCLR = WhichPin;
            CNPDBSET = WhichPin;
        }
    }
    
    return is_valid;
}

/****************************************************************************
 Function
    PortSetup_ConfigureOpenDrain

 Parameters
   PortSetup_Port_t: the port to be configured
   PortSetup_Pin_t: the pin(s) to be configured as open drain outputs

 Returns
   bool: true if port and pins represent legal ports and pins; otherwise, false

 Description
   Configures the specified pin(s) on the specified port as open drain outputs
Example
   PortSetup_ConfigureOpenDrain(_Port_A, _Pin_0 | _Pin_1);
****************************************************************************/
bool PortSetup_ConfigureOpenDrain( PortSetup_Port_t WhichPort, PortSetup_Pin_t WhichPin)
{

    bool is_valid;
    //Check if pin and port are valid options for digital port
    is_valid = ValidateDigitalPorts(WhichPort, WhichPin);
    
    //Check if pins are configured as outputs
    if (WhichPort == _Port_A)
    {
        //Any pin in WhichPin not set as output
        if (~(~TRISAbits.w | ~WhichPin))
        {
            is_valid = false;
        }
    }
    else if (WhichPort == _Port_B)
    {
        //Any pin in WhichPin not set as output
        if (~(~TRISBbits.w | ~WhichPin))
        {
            is_valid = false;
        }
    }
    
    // If port and pin are valid digital outputs enable Open Drain
    if (is_valid){ 
        if (WhichPort == _Port_A)
        {
            ODCASET = WhichPin;
        }
        else if (WhichPort == _Port_B)
        {
            ODCBSET = WhichPin;
        } 
    }
    return is_valid;
}

/****************************************************************************
 Function
    PortSetup_ConfigureChangeNotification

 Parameters
   PortSetup_Port_t: the port to be configured
   PortSetup_Pin_t: the pin(s) to be enabled for change notification

 Returns
   bool: true if port and pins represent legal ports and pins; otherwise, false

 Description
   Configures the specified pin(s) on the specified port to enable change notifications. If any bits are set in the PortSetup_Pin_t parameter, then change notifications are enabled. If that parameter is 0, then change notifications are disabled globally.
Example
   PortSetup_ConfigureChangeNotification(_Port_A, _Pin_0 | _Pin_1);
****************************************************************************/
bool PortSetup_ConfigureChangeNotification( PortSetup_Port_t WhichPort, PortSetup_Pin_t WhichPin)
{
    bool is_valid;
    is_valid = ValidateDigitalPorts(WhichPort, WhichPin);
    
    //Check if pins are configured as inputs
    if (WhichPort == _Port_A)
    {
        //Any pin in WhichPin not set as input
        if (~(TRISAbits.w | ~WhichPin))
        {
            is_valid = false;
        }
    }
    else if (WhichPort == _Port_B)
    {
        //Any pin in WhichPin not set as input
        if (~(TRISBbits.w | ~WhichPin))
        {
            is_valid = false;
        }
    }
    
    // if pins are configured as inputs
    if (is_valid){ 
        if (WhichPort == _Port_A)
        {
            CNCONA = 0x8000; //turn on CN control bit for port
            CNENASET = WhichPin; //enable CN on pins
            PORTA; // read port after enabling CN to avoid false trigger
        }
        else if (WhichPort == _Port_B)
        {
            CNCONB = 0x8000; //turn on CN control bit for port
            CNENBSET = WhichPin; //enable CN on pins
            PORTB; // read port after enabling CN to avoid false trigger
        }
    }
    
    return is_valid;
}

/****************************************************************************
 Helper Function
    ValidateDigitalPorts

 Parameters
   PortSetup_Port_t: the port to be validated
   PortSetup_Pin_t: the pin(s) to be validated

 Returns
   bool: true if port and pins represent legal ports and pins; otherwise, false

 Description
    Checks if the port and pins passed in are legal digital ports and pins on 
    the PIC32
Example
   ValidateDigitalPorts(_Port_A, _Pin_0 | _Pin_1);
****************************************************************************/
static bool ValidateDigitalPorts(PortSetup_Port_t WhichPort, PortSetup_Pin_t WhichPin)
{
    bool is_valid;
    is_valid = false;
    
    // If all pins are in constant of valid digital pins
    if (WhichPort == _Port_A){
        if ( (A_DIGITAL_PINS | WhichPin) == A_DIGITAL_PINS )
        {
            is_valid = true;
        }
    } else if (WhichPort == _Port_B){
        if ( (B_DIGITAL_PINS | WhichPin) == B_DIGITAL_PINS )
        {
            is_valid = true;
        }  
    }
    return is_valid;
}

/****************************************************************************
 Helper Function
    ValidateAnalogPorts

 Parameters
   PortSetup_Port_t: the port to be validated
   PortSetup_Pin_t: the pin(s) to be validated

 Returns
   bool: true if port and pins represent legal ports and pins; otherwise, false

 Description
    Checks if the port and pins passed in are legal analog ports and pins on the 
    PIC32
Example
   ValidateAnalogPorts(_Port_A, _Pin_0 | _Pin_1);
****************************************************************************/

static bool ValidateAnalogPorts(PortSetup_Port_t WhichPort, PortSetup_Pin_t WhichPin)
{
    bool is_valid;
    is_valid = false;
    
    // If all pins are in constant of valid analog pins
    if (WhichPort == _Port_A){
        if ( (A_ANALOG_PINS | WhichPin) == A_ANALOG_PINS )
        {
            is_valid = true;
        }
    } else if (WhichPort == _Port_B){
        if ( (B_ANALOG_PINS | WhichPin) == B_ANALOG_PINS )
        {
            is_valid = true;
        }  
    }
    return is_valid;
}

/*-----------------------TEST HARNESS-------------------------------------*/

//Enables test harness to be disabled easily
#if RUN_TESTS == 1
// CONFIG PRAGMAS for running as Main (Taken from SimMain.c)
// DEVCFG3
#pragma config USERID = 0xFFFF          // Enter Hexadecimal value (Enter Hexadecimal value)
#pragma config PMDL1WAY = OFF           // Peripheral Module Disable Configuration (Allow multiple reconfigurations)
#pragma config IOL1WAY = OFF            // Peripheral Pin Select Configuration (Allow multiple reconfigurations)

// DEVCFG2
#pragma config FPLLIDIV = DIV_2         // PLL Input Divider (2x Divider)
#pragma config FPLLMUL = MUL_20         // PLL Multiplier (20x Multiplier)
#pragma config FPLLODIV = DIV_2         // System PLL Output Clock Divider (PLL Divide by 2)

// DEVCFG1
#pragma config FNOSC = FRCPLL           // Oscillator Selection Bits (Fast RC Osc with PLL)
#pragma config FSOSCEN = OFF            // Secondary Oscillator Enable (Disabled)
#pragma config IESO = OFF               // Internal/External Switch Over (Disabled)
#pragma config POSCMOD = OFF            // Primary Oscillator Configuration (Primary osc disabled)
#pragma config OSCIOFNC = OFF           // CLKO Output Signal Active on the OSCO Pin (Disabled)
#pragma config FPBDIV = DIV_2           // Peripheral Clock Divisor (Pb_Clk is Sys_Clk/2)
#pragma config FCKSM = CSDCMD           // Clock Switching and Monitor Selection (Clock Switch Disable, FSCM Disabled)
#pragma config WDTPS = PS1048576        // Watchdog Timer Postscaler (1:1048576)
#pragma config WINDIS = OFF             // Watchdog Timer Window Enable (Watchdog Timer is in Non-Window Mode)
#pragma config FWDTEN = OFF             // Watchdog Timer Enable (WDT Disabled (SWDTEN Bit Controls))
#pragma config FWDTWINSZ = WINSZ_25     // Watchdog Timer Window Size (Window Size is 25%)

// DEVCFG0
#pragma config JTAGEN = OFF             // JTAG Enable (JTAG Disabled)
#pragma config ICESEL = ICS_PGx1        // ICE/ICD Comm Channel Select (Communicate on PGEC1/PGED1)
#pragma config PWP = OFF                // Program Flash Write Protect (Disable)
#pragma config BWP = OFF                // Boot Flash Write Protect bit (Protection Disabled)
#pragma config CP = OFF                 // Code Protect (Protection Disabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

    #if TEST_TYPE == 0 //Test PortSetup_ConfigureDigitalOutputs
        /*Testing Instructions:
         * Flash PIC32
         * Hook up oscilliscope to ground and loose jumper
         * OR
         * Wire LED and resistor from loose jumper to ground
         * 
         * Touch other end of Jumper to each signal pin one by one
         * The scope should display a square wave for all pins
         * OR
         * The LED should blink for all pins */
        int main(void) {
            // Set up all pins as DIG OUT
            bool a_config_success;
            bool b_config_success;
            a_config_success = PortSetup_ConfigureDigitalOutputs(_Port_A, A_DIGITAL_PINS);
            b_config_success = PortSetup_ConfigureDigitalOutputs(_Port_B, B_DIGITAL_PINS);

            uint32_t index;
            while (true) // do forever 
            {
                // set all outputs hi
                if (a_config_success)
                {
                    LATASET = A_DIGITAL_PINS;
                }
                if (b_config_success)
                {
                    LATBSET = B_DIGITAL_PINS;
                }
                // large for loop to cause delay   
                //100,000 loops = 325 ms
                for( index = 0; index < 100000; index++ ){
                   //Do nothing
                }
                // Set all outputs low
                if (a_config_success)
                {
                    LATACLR = A_DIGITAL_PINS;
                }
                if (b_config_success)
                {
                    LATBCLR = B_DIGITAL_PINS;
                }
                // large for loop to cause delay   
                for( index = 0; index < 100000; index++ ){
                   //Do nothing
                }
            }
        }
        
    #elif TEST_TYPE == 1 //Test PortSetup_ConfigureDigitalInputs
        /*Testing Instructions:
        * Flash PIC32
        * Hook up oscilliscope to ground and A0
        * OR
        * Wire LED and resistor from A0 to ground
        * Wire touch sensor to 3.3V, GND, and TEST_PORT:TEST_PIN
        * Touch and/or release touch sensor
        * The scope/LED should match state of input */
        int main(void)
        {
            //Set A0 as Output
            //RA0 = Pin 2
            //RB0 = Pin 4
            PortSetup_ConfigureDigitalOutputs(_Port_A, _Pin_0);
            
            PortSetup_ConfigureDigitalInputs(TEST_PORT, TEST_PIN);
            //do infinitely
            while (true)
            {
                if (TEST_PORT == _Port_A) // input is an A pin
                {
                    //If Pin is high, turn on LED
                    if (PORTAbits.w & TEST_PIN) // Pin is high
                    {
                        LATASET = _Pin_0;
                    }
                    else //if low, turn off LED
                    {
                        LATACLR = _Pin_0;
                    }
                }
                else if (TEST_PORT == _Port_B) // input is a B Pin
                {
                    //If Pin is high, turn on LED
                    if (PORTBbits.w & TEST_PIN) // Pin is high
                    {
                        LATASET = _Pin_0;
                    }
                    else 
                    {
                        LATACLR = _Pin_0;
                    }
                }                  
            }     
        }
        
    #elif TEST_TYPE == 2 //Test PortSetup_ConfigureAnalogInputs
        // CURRENTLY NO TEST HARNESS 
        int main(void)
        {
            
        }
        
    #elif TEST_TYPE == 3 //Test PortSetup_ConfigurePullUps
        /*Testing Instructions:
         * Flash PIC32
         * Unplug yellow and green wire of snap cable (REQUIRED)
         * Wire LED and resistor from RA0 (Pin 2) to ground
         * Connect jumper to ground
         * Touch other end of Jumper to each pin except A0 one by one
         * The LED should light up if any pin is connected to ground*/
        int main(void)
        {
            //Set A0 as Output
            //RA0 = Pin 2
            PortSetup_ConfigureDigitalOutputs(_Port_A, _Pin_0);
            
            //Set rest of pins as inputs and turn on pull ups
            uint16_t a_digital_in;
            uint16_t b_digital_in;
            a_digital_in = _Pin_1 | _Pin_2 | _Pin_3 | _Pin_4; // all but pin 0
            b_digital_in = B_DIGITAL_PINS; 
            PortSetup_ConfigureDigitalInputs(_Port_A, a_digital_in);
            PortSetup_ConfigurePullUps(_Port_A, a_digital_in);
            PortSetup_ConfigureDigitalInputs(_Port_B, b_digital_in);
            PortSetup_ConfigurePullUps(_Port_B, b_digital_in);
            
            //do infinitely
            while (true)
            {
                //If any pin is low turn on the output
                // NOTE: Need to disconnect yellow & green wire on snap to test
                //If any pin is low
                if (~(PORTAbits.w | ~(a_digital_in)) | ~(PORTBbits.w | ~(b_digital_in))) 
                {
                    LATASET = _Pin_0; // turn on LED
                }
                else // All A and B Pins hi
                {
                    LATACLR = _Pin_0; //turn off LED
                }       
            }  
        }
        
    #elif TEST_TYPE == 4 //Test PortSetup_ConfigurePullDowns
        /*Testing Instructions:
         * Flash PIC32
         * Unplug yellow and green wire of snap cable (REQUIRED)
         * Wire LED and resistor from RA0 (Pin 2) to ground
         * Connect jumper to 3.3V
         * Touch other end of Jumper to each pin except A0 one by one
         * The LED should light up if any pin is connected to HI*/
        int main(void)
        {
            //Set A0 as Output
            //RA0 = Pin 2
            PortSetup_ConfigureDigitalOutputs(_Port_A, _Pin_0);
            
            //Set rest of pins as inputs and turn on pull downs
            uint16_t a_digital_in;
            uint16_t b_digital_in;
            a_digital_in = _Pin_1 | _Pin_2 | _Pin_3 | _Pin_4; // all but pin 0
            b_digital_in = B_DIGITAL_PINS; 
            PortSetup_ConfigureDigitalInputs(_Port_A, a_digital_in);
            PortSetup_ConfigurePullDowns(_Port_A, a_digital_in);
            PortSetup_ConfigureDigitalInputs(_Port_B, b_digital_in);
            PortSetup_ConfigurePullDowns(_Port_B, b_digital_in);
            
            //do infinitely
            while (true)
            {
                //If any pin is hi turn on the output
                if ((PORTAbits.w & (a_digital_in)) | (PORTBbits.w & (b_digital_in))) 
                {
                    LATASET = _Pin_0; // turn on LED
                }
                else // All A and B Pins lo
                {
                    LATACLR = _Pin_0; //turn off LED
                }       
            }  
        }
        
    #elif TEST_TYPE == 5 //Test PortSetup_ConfigureOpenDrain
         /*Testing Instructions:
         * Flash PIC32
         * Wire 10k resistor from TEST_PORT:TEST_PIN to 3.3 or 5V
         * Connect Osciliscope to test pin
         * Scope should display a square wave*/
        int main(void)
        {
            // Config Output w Open Drain
            PortSetup_ConfigureDigitalOutputs(TEST_PORT, TEST_PIN);
            PortSetup_ConfigureOpenDrain(TEST_PORT, TEST_PIN);
            
            uint32_t index;
            while (true) //Do forever
            {
                // set pin high
                if (TEST_PORT == _Port_A)
                {
                    LATASET = TEST_PIN;
                }
                else if (TEST_PORT == _Port_B)
                {
                    LATBSET = TEST_PIN;
                }
                // large for loop to cause delay   
                //100,000 loops = 32 ms
                for( index = 0; index < 100000; index++ ){
                   //Do nothing
                }
                // Set pin low
                if (TEST_PORT == _Port_A)
                {
                    LATACLR = TEST_PIN;
                }
                else if (TEST_PORT == _Port_B)
                {
                    LATBCLR = TEST_PIN;
                }
                // large for loop to cause delay   
                for( index = 0; index < 100000; index++ ){
                   //Do nothing
                }
            }        
        }
        
    #elif TEST_TYPE == 6 //Test PortSetup_ConfigureChangeNotification
        /*Testing Instructions:
        * Flash PIC32
        * Wire LED and resistor from A0 to ground
        * Wire touch sensor to 3.3V, GND, and TEST_PORT:TEST_PIN
        * Touch and/or release touch sensor
        * The LED should briefly turn on when touch sensor state changes */
        int main(void)
        {
            //Set A0 as Output
            //RA0 = Pin 2
            PortSetup_ConfigureDigitalOutputs(_Port_A, _Pin_0);
            
            //Set up input with CN
            PortSetup_ConfigureDigitalInputs(TEST_PORT, TEST_PIN);
            PortSetup_ConfigureChangeNotification(TEST_PORT, TEST_PIN);
            
            uint32_t index;
            uint32_t dummy;
            while (true) //Do forever
            {
                if (TEST_PORT == _Port_A)
                {
                    // If change is registered on TEST_PIN
                    if (CNSTATAbits.w & TEST_PIN)
                    {
                        dummy = PORTA; //read again to avoid double CN trigger
                        LATASET = _Pin_0; // Turn on LED
                        //Wait a bit ~300 ms
                        for(index = 0; index < 100000; index++ )
                        {
                        //Do nothing
                        }
                        LATACLR = _Pin_0; //turn off LED
                    }
                }
                else if (TEST_PORT == _Port_B)
                {
                    // If change is registered on TEST_PIN
                    if (CNSTATBbits.w & TEST_PIN)
                    {
                        dummy = PORTB; //read again to avoid double CN trigger
                        LATASET = _Pin_0; // Turn on LED
                        //Wait a bit ~300 ms
                        for(index = 0; index < 100000; index++ )
                        {
                        //Do nothing
                        }
                        LATACLR = _Pin_0; //turn off LED
                    }
                }   
            }      
        }
        
    #endif
#endif