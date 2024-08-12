/***********************************************************************************************
----------------------------         SETTINGS               ------------------------------------
fichiers pour la configuration du systeme 

************************************************************************************************/
#ifndef SETTINGS_H
#define SETTINGS_H

#include "stdint.h"
#define DEBUG 0

#define PIN_SERVO_OFF 5
#define NUM_SERVOS 16
#define PLUCK_ANGLE 15
// Tableau des angles d'initialisation pour chaque servo
const uint16_t initialAngles[NUM_SERVOS] = {85, 86, 96, 86, 88, 82, 95, 85, 94, 90, 108, 73, 110, 70, 105, 75};// angle du servo contre la corde 
const uint8_t MidiServoMapping[] = {55, 57, 59, 60, 62, 64, 65, 67, 69, 71, 72, 74, 76, 77, 79, 81}; //note midi de G3 a A5 pour ma lyre

//reglages du PCA9685 pour des servo sg90 
#define SERVO_MIN_ANGLE 0
#define SERVO_MAX_ANGLE 180
const uint16_t SERVO_PULSE_MIN = 500;
const uint16_t SERVO_PULSE_MAX = 2500;
const uint16_t SERVO_FREQUENCY = 50;

#endif
