/***********************************************************************************************
----------------------------         SETTINGS               ------------------------------------
fichiers pour la configuration du systeme 

************************************************************************************************/
#ifndef SETTINGS_H
#define SETTINGS_H

#include "stdint.h"

// Configuration Debug
#define DEBUG 0

// Configuration generale
#define NUM_SERVOS 16
#define PLUCK_ANGLE 15
#define SERIAL_BAUD_RATE 115200
#define PIN_SERVO_OE 5  // Pin pour controler OE du PCA9685 (LOW=actif, HIGH=inactif)

// Delais d'initialisation des servos (en millisecondes)
#define SERVO_INIT_DELAY_MS 500
#define SERVO_RESET_DELAY_MS 100
#define SERVO_AUTO_DISABLE_TIMEOUT_MS 2000  // Desactiver apres 2s d'inactivite

// Types de messages MIDI
#define MIDI_NOTE_ON 0x90
#define MIDI_NOTE_OFF 0x80
#define MIDI_PITCH_BEND 0xE0
#define MIDI_CONTROL_CHANGE 0xB0
#define MIDI_CHANNEL_PRESSURE 0xA0
#define MIDI_POLY_KEY_PRESSURE 0xD0
#define MIDI_SYSTEM_COMMON 0xF0
#define MIDI_SYSTEM_EXCLUSIVE_END 0xF7

// Tableau des angles d'initialisation pour chaque servo
const uint16_t initialAngles[NUM_SERVOS] = {85, 86, 96, 86, 88, 82, 95, 85, 94, 90, 108, 73, 110, 70, 105, 75}; // angle du servo contre la corde

// Mapping MIDI note -> Servo (note midi de G3=55 a A5=81 pour ma lyre)
const uint8_t MidiServoMapping[] = {55, 57, 59, 60, 62, 64, 65, 67, 69, 71, 72, 74, 76, 77, 79, 81};

// Plage de notes MIDI supportees
#define MIDI_NOTE_MIN 55  // G3
#define MIDI_NOTE_MAX 81  // A5

// Tableau de mapping inverse optimise: MIDI note -> index servo (-1 si non jouable)
// Couvre la plage complete de 55 a 81 (27 valeurs)
const int8_t ServoMidiMapping[] = {
  0,  // 55 - G3
  -1, // 56
  1,  // 57 - A3
  -1, // 58
  2,  // 59 - B3
  3,  // 60 - C4
  -1, // 61
  4,  // 62 - D4
  -1, // 63
  5,  // 64 - E4
  6,  // 65 - F4
  -1, // 66
  7,  // 67 - G4
  -1, // 68
  8,  // 69 - A4
  -1, // 70
  9,  // 71 - B4
  10, // 72 - C5
  -1, // 73
  11, // 74 - D5
  -1, // 75
  12, // 76 - E5
  13, // 77 - F5
  -1, // 78
  14, // 79 - G5
  -1, // 80
  15  // 81 - A5
};

// Reglages du PCA9685 pour des servo sg90
#define SERVO_MIN_ANGLE 0
#define SERVO_MAX_ANGLE 180
const uint16_t SERVO_PULSE_MIN = 500;
const uint16_t SERVO_PULSE_MAX = 2500;
const uint16_t SERVO_FREQUENCY = 50;
const uint32_t PCA9685_OSCILLATOR_FREQ = 27000000;

#endif
