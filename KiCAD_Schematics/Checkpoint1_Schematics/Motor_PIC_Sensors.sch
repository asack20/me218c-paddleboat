EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 6 7
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
L ME218_BaseLib:LED D7
U 1 1 6238467E
P 6900 4100
F 0 "D7" V 6847 4178 50  0000 L CNN
F 1 "LED" V 6938 4178 50  0000 L CNN
F 2 "" H 6900 4100 50  0001 C CNN
F 3 "" H 6900 4100 50  0001 C CNN
	1    6900 4100
	0    1    -1   0   
$EndComp
$Comp
L ME218_BaseLib:Res1 R20
U 1 1 62384684
P 6500 4100
F 0 "R20" H 6432 4146 50  0000 R CNN
F 1 "47k" H 6432 4055 50  0000 R CNN
F 2 "" V 6540 4090 50  0001 C CNN
F 3 "" H 6500 4100 50  0001 C CNN
	1    6500 4100
	-1   0    0    -1  
$EndComp
$Comp
L ME218_BaseLib:Photo_NPN Q4
U 1 1 6238468A
P 6600 4450
F 0 "Q4" H 6791 4496 50  0000 L CNN
F 1 "Photo_NPN" H 6791 4405 50  0000 L CNN
F 2 "" H 6800 4550 50  0001 C CNN
F 3 "" H 6600 4450 50  0001 C CNN
	1    6600 4450
	-1   0    0    -1  
$EndComp
Wire Wire Line
	6900 4250 6900 4700
Wire Wire Line
	6900 4700 6700 4700
Wire Wire Line
	6500 4700 6500 4650
Wire Wire Line
	6900 3950 6900 3850
Wire Wire Line
	6900 3850 6700 3850
Wire Wire Line
	6500 3850 6500 3950
Wire Wire Line
	6700 3850 6700 3650
Connection ~ 6700 3850
Wire Wire Line
	6700 3850 6500 3850
Connection ~ 6700 4700
Wire Wire Line
	6700 4700 6500 4700
Connection ~ 6500 4250
Wire Notes Line
	5650 4900 5650 4050
Text Notes 5700 4850 0    50   ~ 0
Reflective Opto-Sensor
Wire Notes Line
	5650 4900 7150 4900
Wire Notes Line
	5650 4050 6300 4050
Wire Notes Line
	7150 3750 7150 4900
Wire Wire Line
	6700 4700 6700 5000
$Comp
L power:GNDREF #PWR057
U 1 1 623846A5
P 6700 5000
F 0 "#PWR057" H 6700 4750 50  0001 C CNN
F 1 "GNDREF" H 6500 5000 50  0000 C CNN
F 2 "" H 6700 5000 50  0001 C CNN
F 3 "" H 6700 5000 50  0001 C CNN
	1    6700 5000
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR056
U 1 1 623846AB
P 6700 3650
F 0 "#PWR056" H 6700 3500 50  0001 C CNN
F 1 "+3.3V" H 6715 3823 50  0000 C CNN
F 2 "" H 6700 3650 50  0001 C CNN
F 3 "" H 6700 3650 50  0001 C CNN
	1    6700 3650
	1    0    0    -1  
$EndComp
Text Label 5750 4250 0    50   ~ 0
Sensor_Output
Wire Notes Line
	6300 4050 6300 3750
Wire Notes Line
	6300 3750 7150 3750
Wire Wire Line
	4700 4250 6500 4250
Text Label 5400 4250 2    50   ~ 0
Tape_Sensor
Text GLabel 4700 4250 0    50   Input ~ 0
Tape_Sensor
Wire Wire Line
	4650 3800 5550 3800
Text Label 4900 3800 0    50   ~ 0
Wall_Sensor
Text GLabel 4650 3800 0    50   Input ~ 0
Wall_Sensor
Text Label 4900 3050 0    50   ~ 0
Bumper_Sensor
Text GLabel 4650 3050 0    50   Input ~ 0
Bumper_Sensor
$Comp
L power:+3.3V #PWR054
U 1 1 623C121C
P 5800 2550
F 0 "#PWR054" H 5800 2400 50  0001 C CNN
F 1 "+3.3V" H 5815 2723 50  0000 C CNN
F 2 "" H 5800 2550 50  0001 C CNN
F 3 "" H 5800 2550 50  0001 C CNN
	1    5800 2550
	1    0    0    -1  
$EndComp
$Comp
L ME218_BaseLib:SW-PB SW7
U 1 1 623C17D2
P 5800 2800
F 0 "SW7" V 5754 2948 50  0000 L CNN
F 1 "SW-PB" V 5845 2948 50  0000 L CNN
F 2 "" H 5800 3000 50  0001 C CNN
F 3 "" H 5800 3000 50  0001 C CNN
	1    5800 2800
	0    1    1    0   
$EndComp
$Comp
L ME218_BaseLib:Res1 R19
U 1 1 623C1FF6
P 5800 3250
F 0 "R19" H 5732 3296 50  0000 R CNN
F 1 "10k" H 5732 3205 50  0000 R CNN
F 2 "" V 5840 3240 50  0001 C CNN
F 3 "" H 5800 3250 50  0001 C CNN
	1    5800 3250
	-1   0    0    -1  
$EndComp
$Comp
L power:GNDREF #PWR055
U 1 1 623C28FA
P 5800 3500
F 0 "#PWR055" H 5800 3250 50  0001 C CNN
F 1 "GNDREF" H 5600 3500 50  0000 C CNN
F 2 "" H 5800 3500 50  0001 C CNN
F 3 "" H 5800 3500 50  0001 C CNN
	1    5800 3500
	1    0    0    -1  
$EndComp
Wire Wire Line
	5800 3500 5800 3400
Wire Wire Line
	5800 3100 5800 3050
Wire Wire Line
	4650 3050 5800 3050
Connection ~ 5800 3050
Wire Wire Line
	5800 3050 5800 3000
Wire Wire Line
	5800 2600 5800 2550
$EndSCHEMATC
