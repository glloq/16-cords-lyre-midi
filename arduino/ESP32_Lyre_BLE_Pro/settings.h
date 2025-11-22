/***********************************************************************************************
----------------------------    SETTINGS ESP32 BLE PRO      ------------------------------------
Configuration pour version professionnelle avec SysEx et feedback MIDI
************************************************************************************************/
#ifndef SETTINGS_H
#define SETTINGS_H

#include "stdint.h"

// =============================================================================================
// CONFIGURATION DEBUG
// =============================================================================================
#define DEBUG 1  // 0=OFF, 1=ON (affiche messages MIDI dans Serial Monitor)

// =============================================================================================
// CONFIGURATION BLE MIDI
// =============================================================================================
#define BLE_DEVICE_NAME "Lyre-MIDI-ESP32"  // Nom Bluetooth (32 caractères max)

// Feedback MIDI (envoyer confirmations des notes jouées)
#define MIDI_SEND_FEEDBACK true  // true = envoie Note On/Off en retour

// =============================================================================================
// CONFIGURATION MATERIEL
// =============================================================================================
#define NUM_SERVOS 16
#define PLUCK_ANGLE 15
#define SERIAL_BAUD_RATE 115200
#define PIN_SERVO_OE 5  // Pin GPIO pour contrôler OE du PCA9685

// Configuration I2C (optionnel - par défaut SDA=21, SCL=22)
// Décommentez pour utiliser des pins personnalisés:
// #define I2C_SDA 21
// #define I2C_SCL 22

// =============================================================================================
// DELAIS SERVOS
// =============================================================================================
#define SERVO_INIT_DELAY_MS 500
#define SERVO_RESET_DELAY_MS 100
#define SERVO_AUTO_DISABLE_TIMEOUT_MS 2000  // Désactiver après 2s d'inactivité

// =============================================================================================
// MAPPING MIDI → SERVOS
// =============================================================================================

// Plage de notes MIDI supportées
#define MIDI_NOTE_MIN 55  // G3
#define MIDI_NOTE_MAX 81  // A5

// Tableau des angles d'initialisation pour chaque servo
const uint16_t initialAngles[NUM_SERVOS] = {
  85, 86, 96, 86, 88, 82, 95, 85, 94, 90, 108, 73, 110, 70, 105, 75
};

// Mapping MIDI note → Servo (G3=55 à A5=81)
const uint8_t MidiServoMapping[] = {
  55, 57, 59, 60, 62, 64, 65, 67, 69, 71, 72, 74, 76, 77, 79, 81
};

// Tableau de mapping inverse optimisé: MIDI note → index servo (-1 si non jouable)
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

// =============================================================================================
// REGLAGES PCA9685 POUR SERVOS SG90
// =============================================================================================
#define SERVO_MIN_ANGLE 0
#define SERVO_MAX_ANGLE 180
const uint16_t SERVO_PULSE_MIN = 500;
const uint16_t SERVO_PULSE_MAX = 2500;
const uint16_t SERVO_FREQUENCY = 50;
const uint32_t PCA9685_OSCILLATOR_FREQ = 27000000;

#endif
