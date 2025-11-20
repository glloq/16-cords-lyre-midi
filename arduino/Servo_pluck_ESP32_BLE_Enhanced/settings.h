/***********************************************************************************************
----------------------------   SETTINGS ESP32 BLE ENHANCED   -----------------------------------
Fichier de configuration pour ESP32 avec Bluetooth Low Energy MIDI - VERSION AMELIOREE

NOUVELLES FONCTIONNALITES:
- Contrôle d'appairage avec bouton physique
- LED d'état de connexion BLE
- Filtrage par canal MIDI
- Feedback MIDI (envoi de confirmations)
- Protection anti-spam
- Statistiques MIDI en temps réel
- Watchdog pour stabilité
************************************************************************************************/
#ifndef SETTINGS_H
#define SETTINGS_H

#include "stdint.h"

// Configuration Debug
#define DEBUG 0

// Version firmware
#define FIRMWARE_VERSION "2.0"
#define FIRMWARE_DATE "2025-11-20"

/***********************************************************************************************
CONFIGURATION BLE
************************************************************************************************/
#define BLE_DEVICE_NAME "Lyre-MIDI-ESP32"  // Nom de l'appareil Bluetooth

// Contrôle d'appairage
#define PIN_PAIRING_BUTTON 0       // GPIO 0 (bouton BOOT sur ESP32)
#define PIN_BLE_LED 2              // GPIO 2 (LED intégrée ESP32)
#define PAIRING_TIMEOUT_MS 300000  // 5 minutes timeout d'appairage
#define PAIRING_BUTTON_DEBOUNCE_MS 50
#define PAIRING_LONG_PRESS_MS 3000  // 3 secondes pour effacer liste appairés

// Modes LED BLE
#define LED_OFF 0
#define LED_SLOW_BLINK 1    // 1 Hz - Recherche connexion
#define LED_FAST_BLINK 2    // 5 Hz - Appairage actif
#define LED_ON 3            // Fixe - Connecté

/***********************************************************************************************
CONFIGURATION MIDI
************************************************************************************************/
#define MIDI_CHANNEL 1             // Canal MIDI principal (1-16)
#define MIDI_OMNI_MODE false       // true = écoute tous canaux, false = canal unique
#define MIDI_SEND_FEEDBACK true    // Envoyer feedback MIDI (Note On/Off confirmations)
#define MIDI_SEND_ACTIVE_SENSING false  // Envoyer Active Sensing périodique

// Validation des messages
#define VELOCITY_MIN 1             // Vélocité minimale (1-127)
#define VELOCITY_MAX 127           // Vélocité maximale

// Protection anti-spam
#define MAX_NOTES_PER_SECOND 50    // Limite de notes/seconde
#define ENABLE_RATE_LIMITING true  // Activer protection anti-spam

/***********************************************************************************************
CONFIGURATION SERVOS & MATERIEL
************************************************************************************************/
#define NUM_SERVOS 16
#define PLUCK_ANGLE 15
#define SERIAL_BAUD_RATE 115200
#define PIN_SERVO_OE 5  // Pin GPIO pour contrôler OE du PCA9685

// Configuration I2C pour ESP32 (optionnel - par défaut SDA=21, SCL=22)
// Décommentez pour utiliser des pins personnalisés:
// #define I2C_SDA 21
// #define I2C_SCL 22

// Délais d'initialisation des servos (en millisecondes)
#define SERVO_INIT_DELAY_MS 500
#define SERVO_RESET_DELAY_MS 100
#define SERVO_AUTO_DISABLE_TIMEOUT_MS 2000  // Désactiver après 2s d'inactivité

/***********************************************************************************************
CONFIGURATION WATCHDOG
************************************************************************************************/
#define ENABLE_WATCHDOG true
#define WATCHDOG_TIMEOUT_SEC 30    // Timeout watchdog en secondes

/***********************************************************************************************
TYPES DE MESSAGES MIDI
************************************************************************************************/
#define MIDI_NOTE_ON 0x90
#define MIDI_NOTE_OFF 0x80
#define MIDI_PITCH_BEND 0xE0
#define MIDI_CONTROL_CHANGE 0xB0
#define MIDI_CHANNEL_PRESSURE 0xA0
#define MIDI_POLY_KEY_PRESSURE 0xD0
#define MIDI_SYSTEM_COMMON 0xF0
#define MIDI_SYSTEM_EXCLUSIVE_END 0xF7

/***********************************************************************************************
MAPPING MIDI → SERVOS
************************************************************************************************/

// Tableau des angles d'initialisation pour chaque servo
const uint16_t initialAngles[NUM_SERVOS] = {
  85, 86, 96, 86, 88, 82, 95, 85, 94, 90, 108, 73, 110, 70, 105, 75
};

// Mapping MIDI note → Servo (note midi de G3=55 à A5=81 pour ma lyre)
const uint8_t MidiServoMapping[] = {55, 57, 59, 60, 62, 64, 65, 67, 69, 71, 72, 74, 76, 77, 79, 81};

// Plage de notes MIDI supportées
#define MIDI_NOTE_MIN 55  // G3
#define MIDI_NOTE_MAX 81  // A5

// Tableau de mapping inverse optimisé: MIDI note → index servo (-1 si non jouable)
// Couvre la plage complète de 55 à 81 (27 valeurs)
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

/***********************************************************************************************
REGLAGES PCA9685 POUR SERVOS SG90
************************************************************************************************/
#define SERVO_MIN_ANGLE 0
#define SERVO_MAX_ANGLE 180
const uint16_t SERVO_PULSE_MIN = 500;
const uint16_t SERVO_PULSE_MAX = 2500;
const uint16_t SERVO_FREQUENCY = 50;
const uint32_t PCA9685_OSCILLATOR_FREQ = 27000000;

/***********************************************************************************************
CODES D'ERREUR MIDI
************************************************************************************************/
#define ERROR_NOTE_NOT_PLAYABLE 1
#define ERROR_SERVO_TIMEOUT     2
#define ERROR_RATE_LIMIT        3
#define ERROR_INVALID_CHANNEL   4
#define ERROR_INVALID_VELOCITY  5

#endif
