/***********************************************************************************************
----------------------------  MIDI servo lyre 16 notes ESP32 BLE  ------------------------------
************************************************************************************************
Systeme construit pour le controle d'une lyre de 16 notes avec des servomoteurs SG90 et une
carte PCA9685, utilisant un ESP32 avec Bluetooth Low Energy MIDI

Le systeme recoit les messages MIDI via Bluetooth (BLE MIDI)
- MidiHandler s'occupe de decoder les messages MIDI recus via BLE
- Instrument verifie si les notes peuvent etre jouees et demande a ServoController de les jouer
- ServoController controle les 16 servomoteurs via le PCA9685

Tous les parametres doivent etre configures dans settings.h

MATERIEL REQUIS:
- ESP32 (Dev Kit ou compatible)
- PCA9685 (controleur PWM I2C 16 canaux)
- 16 servomoteurs SG90
- Alimentation 5V 8A pour les servos
- Connexions I2C: SDA=GPIO21, SCL=GPIO22 (par defaut ESP32)

BIBLIOTHEQUES REQUISES:
- Adafruit PWM Servo Driver Library
- BLEMIDI pour ESP32 (https://github.com/lathoub/Arduino-BLE-MIDI)

UTILISATION:
1. Installer les bibliotheques requises via le gestionnaire de bibliotheques Arduino
2. Selectionner la carte ESP32 dans Arduino IDE
3. Compiler et telecharger le code
4. L'appareil apparaitra comme "Lyre-MIDI-ESP32" en Bluetooth
5. Se connecter depuis une application MIDI BLE (ex: MIDI BLE Connect sur iOS/Android)

************************************************************************************************/

#include <BLEMIDI_Transport.h>
#include <hardware/BLEMIDI_ESP32.h>
#include "Instrument.h"
#include "MidiHandler.h"
#include "settings.h"

// Creation des objets BLE MIDI
BLEMIDI_CREATE_INSTANCE(BLE_DEVICE_NAME, MIDI)

Instrument instrument;
MidiHandler* midiHandler = nullptr;

// Variable pour suivre l'etat de la connexion BLE
bool isConnected = false;

/***********************************************************************************************
Callback de connexion BLE
************************************************************************************************/
void onBLEConnected() {
  isConnected = true;
  Serial.println("[BLE] Connexion etablie");
}

/***********************************************************************************************
Callback de deconnexion BLE
************************************************************************************************/
void onBLEDisconnected() {
  isConnected = false;
  Serial.println("[BLE] Deconnexion");
}

/***********************************************************************************************
Setup
************************************************************************************************/
void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  delay(1000);  // Laisser le temps au serial de s'initialiser
  Serial.println("========================================");
  Serial.println("  Lyre MIDI 16 notes - ESP32 BLE");
  Serial.println("========================================");

  // Initialiser I2C avec les pins personnalises si definis
  #if defined(I2C_SDA) && defined(I2C_SCL)
    Wire.begin(I2C_SDA, I2C_SCL);
    Serial.print("[I2C] Pins personnalises - SDA: ");
    Serial.print(I2C_SDA);
    Serial.print(", SCL: ");
    Serial.println(I2C_SCL);
  #else
    Wire.begin();  // Utilise les pins par defaut (SDA=21, SCL=22)
    Serial.println("[I2C] Pins par defaut - SDA: 21, SCL: 22");
  #endif

  // Initialiser le MidiHandler
  Serial.println("[INIT] Initialisation du MidiHandler...");
  midiHandler = new MidiHandler(instrument);

  // Configurer les callbacks BLE MIDI
  MIDI.begin();

  // Callbacks de connexion
  BLEMIDI.setHandleConnected(onBLEConnected);
  BLEMIDI.setHandleDisconnected(onBLEDisconnected);

  // Callbacks MIDI - utilise des lambda pour appeler les methodes de MidiHandler
  MIDI.setHandleNoteOn([](byte channel, byte note, byte velocity) {
    if (midiHandler) midiHandler->onNoteOn(channel, note, velocity);
  });

  MIDI.setHandleNoteOff([](byte channel, byte note, byte velocity) {
    if (midiHandler) midiHandler->onNoteOff(channel, note, velocity);
  });

  MIDI.setHandleControlChange([](byte channel, byte controller, byte value) {
    if (midiHandler) midiHandler->onControlChange(channel, controller, value);
  });

  MIDI.setHandlePitchBend([](byte channel, int bend) {
    if (midiHandler) midiHandler->onPitchBend(channel, bend);
  });

  MIDI.setHandleAfterTouchChannel([](byte channel, byte pressure) {
    if (midiHandler) midiHandler->onAfterTouch(channel, pressure);
  });

  MIDI.setHandleAfterTouchPoly([](byte channel, byte note, byte pressure) {
    if (midiHandler) midiHandler->onPolyPressure(channel, note, pressure);
  });

  Serial.println("[INIT] Initialisation terminee");
  Serial.print("[BLE] Nom de l'appareil: ");
  Serial.println(BLE_DEVICE_NAME);
  Serial.println("[BLE] En attente de connexion...");
  Serial.println("========================================");
}

/***********************************************************************************************
Loop principal
************************************************************************************************/
void loop() {
  // Lire les evenements BLE MIDI
  MIDI.read();

  // Mettre a jour l'instrument (gestion de l'initialisation non-bloquante et timeouts)
  instrument.update();

  // Optionnel: afficher un message periodique si non connecte (pour debug)
  static unsigned long lastStatusTime = 0;
  if (!isConnected && millis() - lastStatusTime > 30000) {  // Toutes les 30 secondes
    Serial.println("[BLE] En attente de connexion...");
    lastStatusTime = millis();
  }
}
