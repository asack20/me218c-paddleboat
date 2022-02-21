EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "ME 218b Team 3 Project Schematic"
Date ""
Rev "v1"
Comp "Drafted by Ryan Brandt"
Comment1 "Checkpoint 1"
Comment2 "Teammates:  Andrew Sack and Afshan Chandani"
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L ME218_BaseLib:Res1 R2
U 1 1 623811DA
P 8300 2900
F 0 "R2" H 8232 2854 50  0000 R CNN
F 1 "3.3k" H 8232 2945 50  0000 R CNN
F 2 "" V 8340 2890 50  0001 C CNN
F 3 "" H 8300 2900 50  0001 C CNN
	1    8300 2900
	-1   0    0    1   
$EndComp
$Comp
L power:+3.3V #PWR02
U 1 1 61DE2C1E
P 8300 2650
F 0 "#PWR02" H 8300 2500 50  0001 C CNN
F 1 "+3.3V" H 8315 2823 50  0000 C CNN
F 2 "" H 8300 2650 50  0001 C CNN
F 3 "" H 8300 2650 50  0001 C CNN
	1    8300 2650
	1    0    0    -1  
$EndComp
Wire Wire Line
	8300 2650 8300 2750
Wire Wire Line
	8300 3250 8300 3050
$Comp
L ME218_BaseLib:MCP6294 U4
U 2 1 623811E8
P 6600 4750
F 0 "U4" H 6944 4796 50  0000 L CNN
F 1 "MCP6294" H 6944 4705 50  0000 L CNN
F 2 "" H 6550 4850 50  0000 C CNN
F 3 "" H 6650 4950 50  0000 C CNN
	2    6600 4750
	1    0    0    -1  
$EndComp
Connection ~ 8200 3250
Wire Wire Line
	8200 3250 8300 3250
Wire Wire Line
	8200 3250 8200 3950
Connection ~ 4850 4850
Connection ~ 6900 4050
Wire Wire Line
	6900 4050 7600 4050
Text Label 5300 4050 0    50   ~ 0
2.5V
Wire Wire Line
	5550 4050 5100 4050
Wire Wire Line
	6900 4050 6900 4750
Wire Wire Line
	6700 4050 6900 4050
Connection ~ 6100 4050
Wire Wire Line
	5850 4050 6100 4050
Wire Wire Line
	6100 4050 6400 4050
Wire Wire Line
	6100 4650 6100 4050
Wire Wire Line
	6300 4650 6100 4650
Wire Wire Line
	5950 4850 5600 4850
Connection ~ 5950 4850
Wire Wire Line
	5950 5000 5950 4850
Wire Wire Line
	6300 4850 5950 4850
Wire Wire Line
	4850 4850 5300 4850
$Comp
L ME218_BaseLib:Res1 R16
U 1 1 62382578
P 5700 4050
F 0 "R16" V 5495 4050 50  0000 C CNN
F 1 "1k" V 5586 4050 50  0000 C CNN
F 2 "" V 5740 4040 50  0001 C CNN
F 3 "" H 5700 4050 50  0001 C CNN
	1    5700 4050
	0    1    1    0   
$EndComp
$Comp
L ME218_BaseLib:Res1 R17
U 1 1 623811ED
P 6550 4050
F 0 "R17" V 6345 4050 50  0000 C CNN
F 1 "5.6k" V 6436 4050 50  0000 C CNN
F 2 "" V 6590 4040 50  0001 C CNN
F 3 "" H 6550 4050 50  0001 C CNN
	1    6550 4050
	0    1    1    0   
$EndComp
$Comp
L power:+5V #PWR044
U 1 1 623811EC
P 6500 4450
F 0 "#PWR044" H 6500 4300 50  0001 C CNN
F 1 "+5V" H 6515 4623 50  0000 C CNN
F 2 "" H 6500 4450 50  0001 C CNN
F 3 "" H 6500 4450 50  0001 C CNN
	1    6500 4450
	1    0    0    -1  
$EndComp
Text Label 6050 5550 0    50   ~ 0
2.5V
Wire Wire Line
	5950 5550 6350 5550
Wire Wire Line
	5950 5300 5950 5550
$Comp
L ME218_BaseLib:Res1 R18
U 1 1 623811EA
P 5950 5150
F 0 "R18" H 6018 5196 50  0000 L CNN
F 1 "10k" H 6018 5105 50  0000 L CNN
F 2 "" V 5990 5140 50  0001 C CNN
F 3 "" H 5950 5150 50  0001 C CNN
	1    5950 5150
	1    0    0    -1  
$EndComp
$Comp
L ME218_BaseLib:Cap C10
U 1 1 623811E9
P 5450 4850
F 0 "C10" V 5650 4850 50  0000 L CNN
F 1 "0.1u" V 5650 4650 50  0000 L CNN
F 2 "" H 5488 4700 50  0001 C CNN
F 3 "" H 5450 4850 50  0001 C CNN
	1    5450 4850
	0    1    1    0   
$EndComp
Text Label 4750 2950 0    50   ~ 0
2.5V
Connection ~ 4650 2950
Wire Wire Line
	4650 2950 5100 2950
Wire Wire Line
	4650 2950 4650 3550
Wire Wire Line
	3850 2950 4650 2950
Wire Wire Line
	3850 3450 3850 2950
Wire Wire Line
	4050 3450 3850 3450
Connection ~ 3100 3650
Wire Wire Line
	4050 3650 3100 3650
Connection ~ 3100 3750
Wire Wire Line
	3100 3750 3100 3650
Wire Wire Line
	2700 4050 3100 4050
Wire Wire Line
	2700 3750 3100 3750
$Comp
L ME218_BaseLib:Cap C9
U 1 1 61FCF5C4
P 2700 3900
F 0 "C9" H 2815 3946 50  0000 L CNN
F 1 "0.1u" H 2815 3855 50  0000 L CNN
F 2 "" H 2738 3750 50  0001 C CNN
F 3 "" H 2700 3900 50  0001 C CNN
	1    2700 3900
	1    0    0    -1  
$EndComp
$Comp
L ME218_BaseLib:Res1 R15
U 1 1 61FCF202
P 3100 3900
F 0 "R15" H 3168 3946 50  0000 L CNN
F 1 "10k" H 3168 3855 50  0000 L CNN
F 2 "" V 3140 3890 50  0001 C CNN
F 3 "" H 3100 3900 50  0001 C CNN
	1    3100 3900
	1    0    0    -1  
$EndComp
$Comp
L ME218_BaseLib:Res1 R14
U 1 1 623811E4
P 3100 3500
F 0 "R14" H 3168 3546 50  0000 L CNN
F 1 "10k" H 3168 3455 50  0000 L CNN
F 2 "" V 3140 3490 50  0001 C CNN
F 3 "" H 3100 3500 50  0001 C CNN
	1    3100 3500
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR040
U 1 1 623811E3
P 3100 3350
F 0 "#PWR040" H 3100 3200 50  0001 C CNN
F 1 "+5V" H 3115 3523 50  0000 C CNN
F 2 "" H 3100 3350 50  0001 C CNN
F 3 "" H 3100 3350 50  0001 C CNN
	1    3100 3350
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR039
U 1 1 623811E1
P 4250 3250
F 0 "#PWR039" H 4250 3100 50  0001 C CNN
F 1 "+5V" H 4265 3423 50  0000 C CNN
F 2 "" H 4250 3250 50  0001 C CNN
F 3 "" H 4250 3250 50  0001 C CNN
	1    4250 3250
	1    0    0    -1  
$EndComp
$Comp
L ME218_BaseLib:MCP6294 U4
U 1 1 623811E0
P 4350 3550
F 0 "U4" H 4694 3596 50  0000 L CNN
F 1 "MCP6294" H 4694 3505 50  0000 L CNN
F 2 "" H 4300 3650 50  0000 C CNN
F 3 "" H 4400 3750 50  0000 C CNN
	1    4350 3550
	1    0    0    -1  
$EndComp
Wire Wire Line
	8000 3250 8200 3250
Wire Wire Line
	7600 3250 7700 3250
Wire Wire Line
	7600 3850 7600 3250
$Comp
L ME218_BaseLib:Res1 R3
U 1 1 623811D9
P 7850 3250
F 0 "R3" V 7645 3250 50  0000 C CNN
F 1 "1M" V 7736 3250 50  0000 C CNN
F 2 "" V 7890 3240 50  0001 C CNN
F 3 "" H 7850 3250 50  0001 C CNN
	1    7850 3250
	0    1    1    0   
$EndComp
$Comp
L power:+5V #PWR03
U 1 1 61DDDD40
P 7800 3650
F 0 "#PWR03" H 7800 3500 50  0001 C CNN
F 1 "+5V" H 7815 3823 50  0000 C CNN
F 2 "" H 7800 3650 50  0001 C CNN
F 3 "" H 7800 3650 50  0001 C CNN
	1    7800 3650
	1    0    0    -1  
$EndComp
$Comp
L ME218_BaseLib:MCP6546 U1
U 1 1 61DDC9B3
P 7900 3950
F 0 "U1" H 8244 3996 50  0000 L CNN
F 1 "MCP6546" H 8244 3905 50  0000 L CNN
F 2 "" H 7850 4050 50  0001 C CNN
F 3 "" H 7950 4150 50  0001 C CNN
	1    7900 3950
	1    0    0    -1  
$EndComp
Wire Wire Line
	4600 4850 4850 4850
Wire Wire Line
	4050 4850 4300 4850
$Comp
L ME218_BaseLib:Res1 R1
U 1 1 6238256F
P 4450 4850
F 0 "R1" V 4245 4850 50  0000 C CNN
F 1 "6.8k" V 4336 4850 50  0000 C CNN
F 2 "" V 4490 4840 50  0001 C CNN
F 3 "" H 4450 4850 50  0001 C CNN
	1    4450 4850
	0    1    1    0   
$EndComp
Wire Wire Line
	4050 5350 4250 5350
Connection ~ 4050 5350
Wire Wire Line
	4850 4850 4850 5450
Wire Wire Line
	4050 5350 4050 4850
Wire Wire Line
	3450 5350 4050 5350
Wire Wire Line
	3450 4950 3450 4800
$Comp
L power:+5V #PWR03
U 1 1 61DD40EA
P 4450 5150
F 0 "#PWR03" H 4450 5000 50  0001 C CNN
F 1 "+5V" H 4465 5323 50  0000 C CNN
F 2 "" H 4450 5150 50  0001 C CNN
F 3 "" H 4450 5150 50  0001 C CNN
	1    4450 5150
	1    0    0    -1  
$EndComp
$Comp
L ME218_BaseLib:Photo_NPN Q1
U 1 1 6238256B
P 3350 5150
F 0 "Q1" H 3541 5196 50  0000 L CNN
F 1 "Photo_NPN" H 3541 5105 50  0000 L CNN
F 2 "" H 3550 5250 50  0001 C CNN
F 3 "" H 3350 5150 50  0001 C CNN
	1    3350 5150
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR02
U 1 1 6238256A
P 3450 4800
F 0 "#PWR02" H 3450 4650 50  0001 C CNN
F 1 "+5V" H 3465 4973 50  0000 C CNN
F 2 "" H 3450 4800 50  0001 C CNN
F 3 "" H 3450 4800 50  0001 C CNN
	1    3450 4800
	1    0    0    -1  
$EndComp
Connection ~ 8300 3250
Wire Wire Line
	8300 3250 9150 3250
Text Label 8450 3250 0    49   ~ 0
Beacon_Detect
Text GLabel 9150 3250 2    49   Input ~ 0
Beacon_Detect
Text Notes 3400 2250 0    50   ~ 0
This topology will have to be changed to match the universal signal conditioning board and the values changed during testing.
$Comp
L power:GNDREF #PWR046
U 1 1 623EC563
P 4450 5750
F 0 "#PWR046" H 4450 5500 50  0001 C CNN
F 1 "GNDREF" H 4455 5577 50  0000 C CNN
F 2 "" H 4450 5750 50  0001 C CNN
F 3 "" H 4450 5750 50  0001 C CNN
	1    4450 5750
	1    0    0    -1  
$EndComp
$Comp
L power:GNDREF #PWR045
U 1 1 623EC85B
P 6500 5050
F 0 "#PWR045" H 6500 4800 50  0001 C CNN
F 1 "GNDREF" H 6505 4877 50  0000 C CNN
F 2 "" H 6500 5050 50  0001 C CNN
F 3 "" H 6500 5050 50  0001 C CNN
	1    6500 5050
	1    0    0    -1  
$EndComp
$Comp
L power:GNDREF #PWR043
U 1 1 623ED00D
P 7800 4250
F 0 "#PWR043" H 7800 4000 50  0001 C CNN
F 1 "GNDREF" H 7805 4077 50  0000 C CNN
F 2 "" H 7800 4250 50  0001 C CNN
F 3 "" H 7800 4250 50  0001 C CNN
	1    7800 4250
	1    0    0    -1  
$EndComp
$Comp
L power:GNDREF #PWR042
U 1 1 623EEBCB
P 4250 3850
F 0 "#PWR042" H 4250 3600 50  0001 C CNN
F 1 "GNDREF" H 4255 3677 50  0000 C CNN
F 2 "" H 4250 3850 50  0001 C CNN
F 3 "" H 4250 3850 50  0001 C CNN
	1    4250 3850
	1    0    0    -1  
$EndComp
Text Label 6550 3250 0    50   ~ 0
2.5V
$Comp
L ME218_BaseLib:Res1 R4
U 1 1 6212ADA5
P 7250 3250
F 0 "R4" V 7045 3250 50  0000 C CNN
F 1 "51k" V 7136 3250 50  0000 C CNN
F 2 "" V 7290 3240 50  0001 C CNN
F 3 "" H 7250 3250 50  0001 C CNN
	1    7250 3250
	0    1    1    0   
$EndComp
Wire Wire Line
	6450 3250 7100 3250
Wire Wire Line
	7400 3250 7600 3250
Connection ~ 7600 3250
Text Label 3750 5550 0    50   ~ 0
2.5V
$Comp
L ME218_BaseLib:MCP6294 U1
U 1 1 6238256C
P 4550 5450
F 0 "U1" H 4894 5496 50  0000 L CNN
F 1 "MCP6294" H 4894 5405 50  0000 L CNN
F 2 "" H 4500 5550 50  0000 C CNN
F 3 "" H 4600 5650 50  0000 C CNN
	1    4550 5450
	1    0    0    -1  
$EndComp
Wire Wire Line
	3650 5550 4250 5550
$EndSCHEMATC
