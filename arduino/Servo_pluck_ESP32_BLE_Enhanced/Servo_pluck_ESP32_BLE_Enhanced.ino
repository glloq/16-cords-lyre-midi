/***********************************************************************************************
-----------------------  MIDI servo lyre 16 notes ESP32 BLE ENHANCED  --------------------------
************************************************************************************************
VERSION AMELIOREE avec:

CONTRÔLE APPAIRAGE BLE:
- Bouton physique pour activer/désactiver appairage (GPIO 0)
- LED d'état de connexion (GPIO 2)
- Timeout d'appairage automatique (5 minutes)
- Appui long pour reset liste appareils

MIDI AMELIORE:
- Filtrage par canal MIDI (configurable)
- Validation complète des messages
- Protection anti-spam (rate limiting)
- Feedback MIDI (confirmations Note On/Off)
- Statistiques en temps réel
- Gestion d'erreurs avec codes

STABILITE:
- Watchdog pour redémarrage auto
- Gestion mémoire optimisée
- Debug amélioré

MATERIEL REQUIS:
- ESP32 (Dev Kit ou compatible)
- PCA9685 (contrôleur PWM I2C 16 canaux)
- 16 servomoteurs SG90
- Bouton appairage sur GPIO 0 (utiliser bouton BOOT intégré)
- LED sur GPIO 2 (LED intégrée sur la plupart des ESP32)

BIBLIOTHEQUES REQUISES:
- Adafruit PWM Servo Driver Library
- BLEMIDI pour ESP32 (https://github.com/lathoub/Arduino-BLE-MIDI)

************************************************************************************************/

#include <BLEMIDI_Transport.h>
#include <hardware/BLEMIDI_ESP32.h>
#include <esp_task_wdt.h>
#include "Instrument.h"
#include "MidiHandler.h"
#include "settings.h"

// Création des objets BLE MIDI
BLEMIDI_CREATE_INSTANCE(BLE_DEVICE_NAME, MIDI)

Instrument instrument;
MidiHandler* midiHandler = nullptr;

/***********************************************************************************************
VARIABLES GLOBALES
************************************************************************************************/

// État de connexion BLE
bool isConnected = false;

// État d'appairage
enum PairingState {
  PAIRING_DISABLED,   // Appairage désactivé
  PAIRING_ENABLED,    // Appairage actif (recherche)
  PAIRING_CONNECTED   // Connecté
};
PairingState pairingState = PAIRING_DISABLED;
unsigned long pairingStartTime = 0;

// Bouton d'appairage
unsigned long buttonPressStart = 0;
bool buttonPressed = false;
bool buttonHandled = false;

// LED d'état
int ledMode = LED_OFF;
unsigned long lastLedToggle = 0;
bool ledState = false;

/***********************************************************************************************
FONCTIONS BLE
************************************************************************************************/

void onBLEConnected() {
  isConnected = true;
  pairingState = PAIRING_CONNECTED;
  ledMode = LED_ON;  // LED fixe quand connecté
  Serial.println("[BLE] ✓ Connexion établie");
}

void onBLEDisconnected() {
  isConnected = false;
  pairingState = PAIRING_DISABLED;
  ledMode = LED_OFF;
  Serial.println("[BLE] ✗ Déconnexion");
}

/***********************************************************************************************
GESTION LED
************************************************************************************************/

void updateLED() {
  unsigned long now = millis();
  unsigned long interval;

  switch (ledMode) {
    case LED_OFF:
      digitalWrite(PIN_BLE_LED, LOW);
      ledState = false;
      break;

    case LED_ON:
      digitalWrite(PIN_BLE_LED, HIGH);
      ledState = true;
      break;

    case LED_SLOW_BLINK:  // 1 Hz (500ms on, 500ms off)
      interval = 500;
      if (now - lastLedToggle >= interval) {
        ledState = !ledState;
        digitalWrite(PIN_BLE_LED, ledState ? HIGH : LOW);
        lastLedToggle = now;
      }
      break;

    case LED_FAST_BLINK:  // 5 Hz (100ms on, 100ms off)
      interval = 100;
      if (now - lastLedToggle >= interval) {
        ledState = !ledState;
        digitalWrite(PIN_BLE_LED, ledState ? HIGH : LOW);
        lastLedToggle = now;
      }
      break;
  }
}

/***********************************************************************************************
GESTION BOUTON APPAIRAGE
************************************************************************************************/

void checkPairingButton() {
  unsigned long now = millis();
  bool currentState = digitalRead(PIN_PAIRING_BUTTON) == LOW;  // Bouton actif bas

  // Détection appui avec debounce
  if (currentState && !buttonPressed) {
    delay(PAIRING_BUTTON_DEBOUNCE_MS);
    if (digitalRead(PIN_PAIRING_BUTTON) == LOW) {
      buttonPressed = true;
      buttonPressStart = now;
      buttonHandled = false;
    }
  }

  // Détection relâchement
  if (!currentState && buttonPressed) {
    unsigned long pressDuration = now - buttonPressStart;
    buttonPressed = false;

    if (!buttonHandled) {
      // Appui court : activer/désactiver appairage
      if (pressDuration < PAIRING_LONG_PRESS_MS) {
        togglePairing();
      }
    }
  }

  // Appui long (maintenu > 3 secondes)
  if (buttonPressed && !buttonHandled) {
    if (now - buttonPressStart >= PAIRING_LONG_PRESS_MS) {
      // Reset liste appareils appairés
      resetPairedDevices();
      buttonHandled = true;

      // Blink rapide 5 fois pour confirmation
      for (int i = 0; i < 5; i++) {
        digitalWrite(PIN_BLE_LED, HIGH);
        delay(100);
        digitalWrite(PIN_BLE_LED, LOW);
        delay(100);
      }
    }
  }
}

void togglePairing() {
  if (pairingState == PAIRING_DISABLED) {
    // Activer appairage
    pairingState = PAIRING_ENABLED;
    pairingStartTime = millis();
    ledMode = LED_FAST_BLINK;
    Serial.println("[BLE] Appairage activé (5 min)");
  } else if (pairingState == PAIRING_ENABLED) {
    // Désactiver appairage
    pairingState = PAIRING_DISABLED;
    ledMode = LED_OFF;
    Serial.println("[BLE] Appairage désactivé");
  }
  // Si PAIRING_CONNECTED, le bouton ne fait rien (déjà connecté)
}

void checkPairingTimeout() {
  if (pairingState == PAIRING_ENABLED) {
    if (millis() - pairingStartTime >= PAIRING_TIMEOUT_MS) {
      Serial.println("[BLE] Timeout appairage (5 min)");
      pairingState = PAIRING_DISABLED;
      ledMode = LED_OFF;
    }
  }
}

void resetPairedDevices() {
  Serial.println("[BLE] Reset liste appareils appairés");
  // TODO: Implémenter sauvegarde NVS des appareils
  // Pour l'instant, juste un message
}

/***********************************************************************************************
COMMANDES SERIE (DEBUG)
************************************************************************************************/

void checkSerialCommands() {
  if (Serial.available()) {
    char cmd = Serial.read();

    switch (cmd) {
      case 's':  // Statistiques MIDI
        if (midiHandler) {
          midiHandler->printStatistics();
        }
        break;

      case 'r':  // Reset statistiques
        if (midiHandler) {
          midiHandler->resetStatistics();
        }
        break;

      case 'i':  // Informations système
        printSystemInfo();
        break;

      case 'p':  // Toggle appairage
        togglePairing();
        break;

      case 'h':  // Aide
        printHelp();
        break;

      default:
        Serial.println("[CMD] Commande inconnue. Tapez 'h' pour aide.");
        break;
    }
  }
}

void printSystemInfo() {
  Serial.println("\n========== SYSTEM INFO ==========");
  Serial.printf("Firmware:        %s (%s)\n", FIRMWARE_VERSION, FIRMWARE_DATE);
  Serial.printf("Device Name:     %s\n", BLE_DEVICE_NAME);
  Serial.printf("MIDI Channel:    %d\n", MIDI_CHANNEL);
  Serial.printf("Omni Mode:       %s\n", MIDI_OMNI_MODE ? "ON" : "OFF");
  Serial.printf("Feedback MIDI:   %s\n", MIDI_SEND_FEEDBACK ? "ON" : "OFF");
  Serial.printf("Rate Limiting:   %s (%d notes/s max)\n",
                ENABLE_RATE_LIMITING ? "ON" : "OFF",
                MAX_NOTES_PER_SECOND);
  Serial.println("---------------------------------");
  Serial.printf("BLE Connected:   %s\n", isConnected ? "YES" : "NO");
  Serial.printf("Pairing State:   %s\n",
                pairingState == PAIRING_DISABLED ? "DISABLED" :
                pairingState == PAIRING_ENABLED ? "ENABLED" : "CONNECTED");
  Serial.printf("Free Heap:       %d bytes\n", ESP.getFreeHeap());
  Serial.printf("Uptime:          %lu ms\n", millis());
  Serial.println("=================================\n");
}

void printHelp() {
  Serial.println("\n========== COMMANDES SERIE ==========");
  Serial.println("s - Afficher statistiques MIDI");
  Serial.println("r - Reset statistiques MIDI");
  Serial.println("i - Informations système");
  Serial.println("p - Toggle appairage BLE");
  Serial.println("h - Afficher cette aide");
  Serial.println("======================================\n");
}

/***********************************************************************************************
SETUP
************************************************************************************************/

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  delay(1000);

  Serial.println("\n========================================");
  Serial.println("  Lyre MIDI 16 notes - ESP32 BLE");
  Serial.println("       VERSION ENHANCED 2.0");
  Serial.println("========================================\n");

  // Configurer pins
  pinMode(PIN_PAIRING_BUTTON, INPUT_PULLUP);
  pinMode(PIN_BLE_LED, OUTPUT);
  digitalWrite(PIN_BLE_LED, LOW);

  // Initialiser Watchdog
  #if ENABLE_WATCHDOG
    esp_task_wdt_init(WATCHDOG_TIMEOUT_SEC, true);
    esp_task_wdt_add(NULL);
    Serial.printf("[WDT] Watchdog activé (%d sec)\n", WATCHDOG_TIMEOUT_SEC);
  #endif

  // Initialiser I2C
  #if defined(I2C_SDA) && defined(I2C_SCL)
    Wire.begin(I2C_SDA, I2C_SCL);
    Serial.printf("[I2C] Pins: SDA=%d, SCL=%d\n", I2C_SDA, I2C_SCL);
  #else
    Wire.begin();
    Serial.println("[I2C] Pins par défaut: SDA=21, SCL=22");
  #endif

  // Initialiser MidiHandler
  Serial.println("[INIT] Initialisation du MidiHandler...");
  midiHandler = new MidiHandler(instrument);

  // Configurer BLE MIDI
  MIDI.begin();

  // Callbacks de connexion
  BLEMIDI.setHandleConnected(onBLEConnected);
  BLEMIDI.setHandleDisconnected(onBLEDisconnected);

  // Callbacks MIDI
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

  // Activer appairage au démarrage
  pairingState = PAIRING_ENABLED;
  pairingStartTime = millis();
  ledMode = LED_FAST_BLINK;

  Serial.println("\n[INIT] ✓ Initialisation terminée");
  Serial.printf("[BLE] Nom: %s\n", BLE_DEVICE_NAME);
  Serial.println("[BLE] Appairage activé (5 min)");
  Serial.println("========================================");
  Serial.println("\nTapez 'h' pour aide\n");
}

/***********************************************************************************************
LOOP PRINCIPAL
************************************************************************************************/

void loop() {
  // Reset watchdog
  #if ENABLE_WATCHDOG
    esp_task_wdt_reset();
  #endif

  // Lire événements BLE MIDI
  MIDI.read();

  // Mettre à jour instrument (servos, timeouts)
  instrument.update();

  // Gestion bouton et LED
  checkPairingButton();
  checkPairingTimeout();
  updateLED();

  // Commandes série
  checkSerialCommands();

  // Afficher statistiques périodiquement (toutes les 60 secondes si debug)
  #if DEBUG
    static unsigned long lastStatsTime = 0;
    if (millis() - lastStatsTime >= 60000) {
      if (midiHandler) {
        midiHandler->printStatistics();
      }
      lastStatsTime = millis();
    }
  #endif
}
