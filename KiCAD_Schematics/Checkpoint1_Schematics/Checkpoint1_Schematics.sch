EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 7
Title "ME 218b Team 3 Project Schematic"
Date ""
Rev "v1"
Comp "Drafted by Ryan Brandt"
Comment1 "Checkpoint 1"
Comment2 "Teammates:  Andrew Sack and Afshan Chandani"
Comment3 ""
Comment4 ""
$EndDescr
Wire Wire Line
	9350 850  9350 750 
Wire Wire Line
	9350 950  9350 1050
Wire Wire Line
	10000 1050 9500 1050
Wire Wire Line
	10000 1150 9500 1150
$Comp
L power:+3.3V #PWR08
U 1 1 61564A4A
P 9350 750
F 0 "#PWR08" H 9350 600 50  0001 C CNN
F 1 "+3.3V" H 9365 923 50  0000 C CNN
F 2 "" H 9350 750 50  0001 C CNN
F 3 "" H 9350 750 50  0001 C CNN
	1    9350 750 
	1    0    0    -1  
$EndComp
$Comp
L power:GNDREF #PWR09
U 1 1 62281663
P 9350 1050
F 0 "#PWR09" H 9350 800 50  0001 C CNN
F 1 "GNDREF" H 9355 877 50  0000 C CNN
F 2 "" H 9350 1050 50  0001 C CNN
F 3 "" H 9350 1050 50  0001 C CNN
	1    9350 1050
	1    0    0    -1  
$EndComp
Text Label 9650 1150 0    50   ~ 0
PGEC1
Text Label 9650 1050 0    50   ~ 0
PGED1
Text Label 9650 750  0    50   ~ 0
MCLRbar
Wire Wire Line
	10000 950  9350 950 
Wire Wire Line
	10000 850  9350 850 
Wire Wire Line
	10000 750  9500 750 
$Comp
L ME218_BaseLib:USB2Serial U3
U 1 1 62281664
P 8450 850
F 0 "U3" H 8550 1115 50  0000 C CNN
F 1 "USB2Serial" H 8550 1024 50  0000 C CNN
F 2 "" H 8450 850 50  0001 C CNN
F 3 "" H 8450 850 50  0001 C CNN
	1    8450 850 
	1    0    0    -1  
$EndComp
$Comp
L power:GNDREF #PWR019
U 1 1 623811C9
P 8100 1100
F 0 "#PWR019" H 8100 850 50  0001 C CNN
F 1 "GNDREF" H 8105 927 50  0000 C CNN
F 2 "" H 8100 1100 50  0001 C CNN
F 3 "" H 8100 1100 50  0001 C CNN
	1    8100 1100
	1    0    0    -1  
$EndComp
NoConn ~ 9000 950 
NoConn ~ 9000 850 
Wire Wire Line
	8100 950  7100 950 
Wire Wire Line
	7100 850  8100 850 
Text Label 7350 850  0    50   ~ 0
TxD
Text Label 7350 950  0    50   ~ 0
RxD
$Comp
L ME218_BaseLib:MPLAB_Snap PRG1
U 1 1 62281661
P 10450 950
F 0 "PRG1" H 10730 814 50  0000 L CNN
F 1 "MPLAB_Snap" H 10730 723 50  0000 L CNN
F 2 "" H 10450 950 50  0001 C CNN
F 3 "" H 10450 950 50  0001 C CNN
	1    10450 950 
	1    0    0    -1  
$EndComp
$Comp
L ME218_BaseLib:Cap C5
U 1 1 62079BC5
P 7700 2950
F 0 "C5" H 7586 2904 50  0000 R CNN
F 1 "0.1uF" H 7586 2995 50  0000 R CNN
F 2 "" H 7738 2800 50  0001 C CNN
F 3 "" H 7700 2950 50  0001 C CNN
	1    7700 2950
	1    0    0    1   
$EndComp
$Comp
L ME218_BaseLib:Cap C7
U 1 1 62079BCB
P 7700 4400
F 0 "C7" H 7586 4354 50  0000 R CNN
F 1 "0.1uF" H 7586 4445 50  0000 R CNN
F 2 "" H 7738 4250 50  0001 C CNN
F 3 "" H 7700 4400 50  0001 C CNN
	1    7700 4400
	1    0    0    1   
$EndComp
$Comp
L ME218_BaseLib:Cap C8
U 1 1 62079BD1
P 8400 4950
F 0 "C8" V 8600 5050 50  0000 C CNN
F 1 "10uF" V 8600 4850 50  0000 C CNN
F 2 "" H 8438 4800 50  0001 C CNN
F 3 "" H 8400 4950 50  0001 C CNN
	1    8400 4950
	0    1    1    0   
$EndComp
$Comp
L ME218_BaseLib:Cap C6
U 1 1 62079BD7
P 10350 2950
F 0 "C6" H 10465 2996 50  0000 L CNN
F 1 "0.1uF" H 10465 2905 50  0000 L CNN
F 2 "" H 10388 2800 50  0001 C CNN
F 3 "" H 10350 2950 50  0001 C CNN
	1    10350 2950
	1    0    0    -1  
$EndComp
$Comp
L ME218_BaseLib:Res1 R6
U 1 1 62079BDD
P 9550 2800
F 0 "R6" V 9345 2800 50  0000 C CNN
F 1 "1k" V 9436 2800 50  0000 C CNN
F 2 "" V 9590 2790 50  0001 C CNN
F 3 "" H 9550 2800 50  0001 C CNN
	1    9550 2800
	0    1    1    0   
$EndComp
$Comp
L ME218_BaseLib:Res1 R5
U 1 1 62079BE3
P 10350 2550
F 0 "R5" H 10282 2504 50  0000 R CNN
F 1 "10k" H 10282 2595 50  0000 R CNN
F 2 "" V 10390 2540 50  0001 C CNN
F 3 "" H 10350 2550 50  0001 C CNN
	1    10350 2550
	-1   0    0    1   
$EndComp
Wire Wire Line
	9250 2800 9300 2800
Wire Wire Line
	10350 2700 10350 2750
Connection ~ 10350 2800
Wire Wire Line
	10550 2700 10550 2750
Wire Wire Line
	10550 2750 10350 2750
Connection ~ 10350 2750
Wire Wire Line
	10350 2750 10350 2800
$Comp
L power:+3.3V #PWR014
U 1 1 62079BF1
P 7700 4250
F 0 "#PWR014" H 7700 4100 50  0001 C CNN
F 1 "+3.3V" H 7715 4423 50  0000 C CNN
F 2 "" H 7700 4250 50  0001 C CNN
F 3 "" H 7700 4250 50  0001 C CNN
	1    7700 4250
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR010
U 1 1 62079BF7
P 10350 2400
F 0 "#PWR010" H 10350 2250 50  0001 C CNN
F 1 "+3.3V" H 10365 2573 50  0000 C CNN
F 2 "" H 10350 2400 50  0001 C CNN
F 3 "" H 10350 2400 50  0001 C CNN
	1    10350 2400
	1    0    0    -1  
$EndComp
$Comp
L power:GNDREF #PWR016
U 1 1 62079BFD
P 8250 4950
F 0 "#PWR016" H 8250 4700 50  0001 C CNN
F 1 "GNDREF" H 8050 4950 50  0000 C CNN
F 2 "" H 8250 4950 50  0001 C CNN
F 3 "" H 8250 4950 50  0001 C CNN
	1    8250 4950
	1    0    0    -1  
$EndComp
$Comp
L power:GNDREF #PWR015
U 1 1 62079C03
P 7700 4550
F 0 "#PWR015" H 7700 4300 50  0001 C CNN
F 1 "GNDREF" H 7705 4377 50  0000 C CNN
F 2 "" H 7700 4550 50  0001 C CNN
F 3 "" H 7700 4550 50  0001 C CNN
	1    7700 4550
	1    0    0    -1  
$EndComp
$Comp
L power:GNDREF #PWR012
U 1 1 62079C09
P 7700 3100
F 0 "#PWR012" H 7700 2850 50  0001 C CNN
F 1 "GNDREF" H 7705 2927 50  0000 C CNN
F 2 "" H 7700 3100 50  0001 C CNN
F 3 "" H 7700 3100 50  0001 C CNN
	1    7700 3100
	1    0    0    -1  
$EndComp
Wire Wire Line
	9300 2800 9300 2400
Wire Wire Line
	9300 2400 8800 2400
Connection ~ 9300 2800
Wire Wire Line
	9300 2800 9400 2800
Connection ~ 7700 3100
Connection ~ 7700 4550
Connection ~ 8250 4950
Connection ~ 7700 4250
Connection ~ 7700 2800
$Comp
L power:+3.3V #PWR011
U 1 1 62079C18
P 7700 2800
F 0 "#PWR011" H 7700 2650 50  0001 C CNN
F 1 "+3.3V" H 7715 2973 50  0000 C CNN
F 2 "" H 7700 2800 50  0001 C CNN
F 3 "" H 7700 2800 50  0001 C CNN
	1    7700 2800
	1    0    0    -1  
$EndComp
$Comp
L ME218_BaseLib:PIC32MX170F256B U2
U 1 1 62079C1E
P 8750 3650
F 0 "U2" H 8475 4765 50  0000 C CNN
F 1 "PIC32MX170F256B" H 8475 4674 50  0000 C CNN
F 2 "" H 8750 3650 50  0001 C CNN
F 3 "" H 8750 3650 50  0001 C CNN
	1    8750 3650
	1    0    0    -1  
$EndComp
Text Label 8900 2400 0    50   ~ 0
MCLRbar
Wire Wire Line
	9250 3150 9650 3150
Wire Wire Line
	9250 3250 9650 3250
Text Label 9350 3150 0    50   ~ 0
PGED1
Text Label 9350 3250 0    50   ~ 0
PGEC1
Connection ~ 10750 3100
$Comp
L power:GNDREF #PWR013
U 1 1 62079C2A
P 10750 3100
F 0 "#PWR013" H 10750 2850 50  0001 C CNN
F 1 "GNDREF" H 10755 2927 50  0000 C CNN
F 2 "" H 10750 3100 50  0001 C CNN
F 3 "" H 10750 3100 50  0001 C CNN
	1    10750 3100
	1    0    0    -1  
$EndComp
Wire Wire Line
	10350 3100 10750 3100
Wire Wire Line
	10750 2700 10550 2700
$Comp
L ME218_BaseLib:SW-PB SW2
U 1 1 62079C32
P 10750 2900
F 0 "SW2" V 10704 3048 50  0000 L CNN
F 1 "Reset" V 10795 3048 50  0000 L CNN
F 2 "" H 10750 3100 50  0001 C CNN
F 3 "" H 10750 3100 50  0001 C CNN
	1    10750 2900
	0    1    1    0   
$EndComp
Wire Wire Line
	9250 3750 9750 3750
Wire Wire Line
	9250 3850 9750 3850
Text Label 9350 3750 0    50   ~ 0
TxD
Text Label 9350 3850 0    50   ~ 0
RxD
Text Label 1450 3800 0    50   ~ 0
SPI_SS1
Text Label 1450 3900 0    50   ~ 0
SPI_MOSI1
Wire Wire Line
	7700 3500 7050 3500
Wire Wire Line
	7700 3600 7050 3600
Wire Wire Line
	9250 3650 9750 3650
Wire Wire Line
	9250 4550 9750 4550
Text Label 7200 3500 0    50   ~ 0
SPI_SS1
Text Label 7200 3600 0    50   ~ 0
SPI_MISO1
Text Label 9350 3650 0    50   ~ 0
SPI_MOSI1
Text Label 9350 4550 0    50   ~ 0
SPI_SCK1
Wire Wire Line
	9250 4650 10000 4650
Text Label 1450 4200 0    50   ~ 0
Start_Button
Text Label 9350 4650 0    50   ~ 0
Bumper_Sensor
Text Label 9350 3350 0    50   ~ 0
Tape_Sensor
Text Label 9350 3450 0    50   ~ 0
Wall_Sensor
Text Label 6650 3800 0    50   ~ 0
Motor_1_Digital_Output
Wire Wire Line
	6550 3800 7700 3800
Wire Wire Line
	6550 3900 7700 3900
Text Label 9350 3550 0    50   ~ 0
Motor_1_PWM_OC1
Text Label 6650 3900 0    50   ~ 0
Motor_1_Encoder_A_IC1
Text Label 9350 3950 0    50   ~ 0
Motor_2_PWM_OC2
Wire Wire Line
	9250 3550 10050 3550
Text Label 9350 4050 0    50   ~ 0
Motor_2_Digital_Output
Text Label 9350 4150 0    50   ~ 0
Motor_2_Encoder_A_IC2
Text Label 9350 4350 0    50   ~ 0
Motor_1_Encoder_B_Digital_Input
Text Label 9350 4450 0    50   ~ 0
Motor_2_Encoder_B_Digital_Input
Wire Wire Line
	9250 4150 10250 4150
Wire Wire Line
	9250 4050 10250 4050
Wire Wire Line
	9250 3950 10250 3950
$Sheet
S 500  550  2900 1900
U 621CA905
F0 "Servo_Sheet" 50
F1 "Servos.sch" 50
$EndSheet
$Sheet
S 650  5700 2450 1900
U 621CB711
F0 "User_Control_and_Lights" 50
F1 "User_Control_and_Lights.sch" 50
$EndSheet
$Sheet
S 3300 5700 3000 1900
U 621CBBB4
F0 "Beacon_Analog_Signal_Conditioning" 50
F1 "Beacon_Analog_Signal_Conditioning.sch" 50
$EndSheet
$Sheet
S 3550 550  2150 1650
U 621CC060
F0 "Power_and_Voltage_Regulation" 50
F1 "Power_and_Voltage_Regulation.sch" 50
$EndSheet
Wire Notes Line
	6500 6350 6500 1550
Wire Notes Line
	11150 1550 11150 6350
Text Notes 8050 2050 0    236  ~ 0
Motor PIC
Text Notes 650  3000 0    236  ~ 0
Main PIC
$Sheet
S 9600 5300 1400 900 
U 621E0BC1
F0 "Motor_PIC_Sensors" 49
F1 "Motor_PIC_Sensors.sch" 49
$EndSheet
$Sheet
S 6900 5300 2400 900 
U 621E1073
F0 "Motor_Control" 49
F1 "Motor_Control.sch" 49
$EndSheet
Text GLabel 1300 4000 0    49   Input ~ 0
Latch_Servo_PWM
Text GLabel 1300 4100 0    49   Input ~ 0
Launcher_Servo_PWM
NoConn ~ 9250 4250
NoConn ~ 7700 3700
Text GLabel 1300 4200 0    49   Input ~ 0
Start_Button
Text GLabel 4800 3850 2    49   Input ~ 0
Beacon_Detect
NoConn ~ 4050 4350
Text GLabel 5200 4950 2    49   Input ~ 0
Team_Identity_Blue_Control
Text GLabel 5150 4550 2    49   Input ~ 0
Team_Identity_Red_Control
Text GLabel 4800 4450 2    49   Input ~ 0
Strategy_Switch
Text GLabel 4850 4250 2    49   Input ~ 0
Refill_Done_Button
NoConn ~ 4050 4750
NoConn ~ 4050 4650
Text GLabel 5300 3750 2    49   Input ~ 0
Reloader_Servo_PWM
Text GLabel 5300 3650 2    49   Input ~ 0
Game_Status_Flag_Servo_PWM
Wire Wire Line
	4050 4950 5200 4950
Wire Wire Line
	4050 4550 5150 4550
Text Label 4150 4950 0    50   ~ 0
Team_Identity_Blue_Control
Text Label 4150 4550 0    50   ~ 0
Team_Identity_Red_Control
Text Label 4150 4450 0    50   ~ 0
Strategy_Switch
Wire Wire Line
	4050 4250 4850 4250
Text Label 4150 4250 0    50   ~ 0
Refill_Done_Button
Text Label 4150 3850 0    50   ~ 0
Beacon_Detect
Wire Wire Line
	4050 3750 5300 3750
Wire Wire Line
	4050 3650 5300 3650
Wire Wire Line
	4500 3050 4700 3050
Wire Wire Line
	4500 3050 4500 3100
Text Label 4150 3750 0    50   ~ 0
Reloader_Servo_PWM
Text Label 4150 3650 0    50   ~ 0
Game_Status_Flag_Servo_PWM
Wire Wire Line
	4050 4850 4800 4850
Wire Wire Line
	4050 4450 4800 4450
Wire Wire Line
	4050 4150 4800 4150
Wire Wire Line
	4050 4050 4800 4050
Wire Wire Line
	4050 3950 4800 3950
Wire Wire Line
	4050 3850 4800 3850
Wire Wire Line
	1300 4200 2500 4200
Wire Wire Line
	1300 3800 2500 3800
Wire Wire Line
	1300 3900 2500 3900
Wire Wire Line
	1300 4000 2500 4000
Wire Wire Line
	1300 4100 2500 4100
Text Label 1450 4100 0    50   ~ 0
Launcher_Servo_PWM
Text Label 1450 4000 0    50   ~ 0
Latch_Servo_PWM
Text Label 4150 4850 0    50   ~ 0
SPI_SCK1
Text Label 4150 3950 0    50   ~ 0
SPI_MISO1
$Comp
L ME218_BaseLib:Cap C1
U 1 1 62281653
P 2500 3250
F 0 "C1" H 2386 3204 50  0000 R CNN
F 1 "0.1uF" H 2386 3295 50  0000 R CNN
F 2 "" H 2538 3100 50  0001 C CNN
F 3 "" H 2500 3250 50  0001 C CNN
	1    2500 3250
	1    0    0    1   
$EndComp
$Comp
L ME218_BaseLib:Cap C3
U 1 1 62281654
P 2500 4700
F 0 "C3" H 2386 4654 50  0000 R CNN
F 1 "0.1uF" H 2386 4745 50  0000 R CNN
F 2 "" H 2538 4550 50  0001 C CNN
F 3 "" H 2500 4700 50  0001 C CNN
	1    2500 4700
	1    0    0    1   
$EndComp
$Comp
L ME218_BaseLib:Cap C4
U 1 1 62281655
P 3200 5250
F 0 "C4" V 3400 5350 50  0000 C CNN
F 1 "10uF" V 3400 5150 50  0000 C CNN
F 2 "" H 3238 5100 50  0001 C CNN
F 3 "" H 3200 5250 50  0001 C CNN
	1    3200 5250
	0    1    1    0   
$EndComp
$Comp
L ME218_BaseLib:Cap C2
U 1 1 62281656
P 4700 3200
F 0 "C2" H 4815 3246 50  0000 L CNN
F 1 "0.1uF" H 4815 3155 50  0000 L CNN
F 2 "" H 4738 3050 50  0001 C CNN
F 3 "" H 4700 3200 50  0001 C CNN
	1    4700 3200
	1    0    0    -1  
$EndComp
$Comp
L ME218_BaseLib:Res1 R2
U 1 1 62281657
P 4350 3100
F 0 "R2" V 4145 3100 50  0000 C CNN
F 1 "1k" V 4236 3100 50  0000 C CNN
F 2 "" V 4390 3090 50  0001 C CNN
F 3 "" H 4350 3100 50  0001 C CNN
	1    4350 3100
	0    1    1    0   
$EndComp
$Comp
L ME218_BaseLib:Res1 R1
U 1 1 62281658
P 4700 2800
F 0 "R1" H 4632 2754 50  0000 R CNN
F 1 "10k" H 4632 2845 50  0000 R CNN
F 2 "" V 4740 2790 50  0001 C CNN
F 3 "" H 4700 2800 50  0001 C CNN
	1    4700 2800
	-1   0    0    1   
$EndComp
Wire Wire Line
	4050 3100 4100 3100
Wire Wire Line
	4700 2950 4700 3000
Connection ~ 4700 3050
Wire Wire Line
	4900 2950 4900 3000
Wire Wire Line
	4900 3000 4700 3000
Connection ~ 4700 3000
Wire Wire Line
	4700 3000 4700 3050
$Comp
L power:+3.3V #PWR05
U 1 1 6228165B
P 2500 4550
F 0 "#PWR05" H 2500 4400 50  0001 C CNN
F 1 "+3.3V" H 2515 4723 50  0000 C CNN
F 2 "" H 2500 4550 50  0001 C CNN
F 3 "" H 2500 4550 50  0001 C CNN
	1    2500 4550
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR01
U 1 1 623811C3
P 4700 2650
F 0 "#PWR01" H 4700 2500 50  0001 C CNN
F 1 "+3.3V" H 4715 2823 50  0000 C CNN
F 2 "" H 4700 2650 50  0001 C CNN
F 3 "" H 4700 2650 50  0001 C CNN
	1    4700 2650
	1    0    0    -1  
$EndComp
$Comp
L power:GNDREF #PWR07
U 1 1 614EA4AC
P 3050 5250
F 0 "#PWR07" H 3050 5000 50  0001 C CNN
F 1 "GNDREF" H 2850 5250 50  0000 C CNN
F 2 "" H 3050 5250 50  0001 C CNN
F 3 "" H 3050 5250 50  0001 C CNN
	1    3050 5250
	1    0    0    -1  
$EndComp
$Comp
L power:GNDREF #PWR06
U 1 1 614EABE0
P 2500 4850
F 0 "#PWR06" H 2500 4600 50  0001 C CNN
F 1 "GNDREF" H 2505 4677 50  0000 C CNN
F 2 "" H 2500 4850 50  0001 C CNN
F 3 "" H 2500 4850 50  0001 C CNN
	1    2500 4850
	1    0    0    -1  
$EndComp
$Comp
L power:GNDREF #PWR03
U 1 1 614EAE76
P 2500 3400
F 0 "#PWR03" H 2500 3150 50  0001 C CNN
F 1 "GNDREF" H 2505 3227 50  0000 C CNN
F 2 "" H 2500 3400 50  0001 C CNN
F 3 "" H 2500 3400 50  0001 C CNN
	1    2500 3400
	1    0    0    -1  
$EndComp
Wire Wire Line
	4100 3100 4100 2700
Wire Wire Line
	4100 2700 3600 2700
Connection ~ 4100 3100
Wire Wire Line
	4100 3100 4200 3100
Connection ~ 2500 3400
Connection ~ 2500 4850
Connection ~ 3050 5250
Connection ~ 2500 4550
Connection ~ 2500 3100
$Comp
L power:+3.3V #PWR02
U 1 1 6228165A
P 2500 3100
F 0 "#PWR02" H 2500 2950 50  0001 C CNN
F 1 "+3.3V" H 2515 3273 50  0000 C CNN
F 2 "" H 2500 3100 50  0001 C CNN
F 3 "" H 2500 3100 50  0001 C CNN
	1    2500 3100
	1    0    0    -1  
$EndComp
$Comp
L ME218_BaseLib:PIC32MX170F256B U1
U 1 1 62281652
P 3550 3950
F 0 "U1" H 3275 5065 50  0000 C CNN
F 1 "PIC32MX170F256B" H 3275 4974 50  0000 C CNN
F 2 "" H 3550 3950 50  0001 C CNN
F 3 "" H 3550 3950 50  0001 C CNN
	1    3550 3950
	1    0    0    -1  
$EndComp
Text Label 3700 2700 0    50   ~ 0
MCLRbar
Wire Wire Line
	4050 3450 4450 3450
Wire Wire Line
	4050 3550 4450 3550
Text Label 4150 3450 0    50   ~ 0
PGED1
Text Label 4150 3550 0    50   ~ 0
PGEC1
Connection ~ 5100 3350
$Comp
L power:GNDREF #PWR04
U 1 1 62382565
P 5100 3350
F 0 "#PWR04" H 5100 3100 50  0001 C CNN
F 1 "GNDREF" H 5105 3177 50  0000 C CNN
F 2 "" H 5100 3350 50  0001 C CNN
F 3 "" H 5100 3350 50  0001 C CNN
	1    5100 3350
	1    0    0    -1  
$EndComp
Wire Wire Line
	4700 3350 5100 3350
Wire Wire Line
	5100 2950 4900 2950
$Comp
L ME218_BaseLib:SW-PB SW1
U 1 1 62281659
P 5100 3150
F 0 "SW1" V 5054 3298 50  0000 L CNN
F 1 "Reset" V 5145 3298 50  0000 L CNN
F 2 "" H 5100 3350 50  0001 C CNN
F 3 "" H 5100 3350 50  0001 C CNN
	1    5100 3150
	0    1    1    0   
$EndComp
Text Label 4150 4050 0    50   ~ 0
TxD
Text Label 4150 4150 0    50   ~ 0
RxD
Text GLabel 6550 3250 2    49   Input ~ 0
Motor_1_Digital_Output
Text GLabel 10250 4050 2    49   Input ~ 0
Motor_2_Digital_Output
Text GLabel 10050 3550 2    49   Input ~ 0
Motor_1_PWM_OC1
Text GLabel 10250 3950 2    49   Input ~ 0
Motor_2_PWM_OC2
Wire Wire Line
	6550 3800 6550 3250
Wire Notes Line
	6500 6350 11150 6350
Wire Notes Line
	6500 1550 11150 1550
Text GLabel 6550 4150 2    49   Input ~ 0
Motor_1_Encoder_A_IC1
Wire Wire Line
	6550 4150 6550 3900
Wire Wire Line
	9700 2800 10350 2800
Text GLabel 10250 4150 2    49   Input ~ 0
Motor_2_Encoder_A_IC2
Text GLabel 10800 4850 0    49   Input ~ 0
Motor_1_Encoder_B_Digital_Input
Text GLabel 10950 5000 0    49   Input ~ 0
Motor_2_Encoder_B_Digital_Input
Wire Wire Line
	10800 4850 10800 4350
Wire Wire Line
	9250 4350 10800 4350
Wire Wire Line
	10950 4450 10950 5000
Wire Wire Line
	9250 4450 10950 4450
Text GLabel 10050 3350 2    50   Input ~ 0
Tape_Sensor
Text GLabel 10050 3450 2    50   Input ~ 0
Wall_Sensor
Wire Wire Line
	9250 3350 10050 3350
Wire Wire Line
	9250 3450 10050 3450
Text GLabel 10000 4650 2    50   Input ~ 0
Bumper_Sensor
$EndSCHEMATC
