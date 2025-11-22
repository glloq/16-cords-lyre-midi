/***********************************************************************************************
----------------------------  MIDI servo lyre 16 notes ESP32 BLE NATIF  ------------------------
************************************************************************************************
Version utilisant le BLE NATIF ESP32 (sans bibliothèque externe)
Implémentation directe du protocole BLE MIDI

AVANTAGES:
- Aucune bibliothèque externe requise (seulement BLE natif ESP32)
- Plus stable et contrôlable
- Compatible avec toutes les apps MIDI BLE

MATERIEL REQUIS:
- ESP32 (WROOM-32D ou autre)
- PCA9685 (contrôleur PWM I2C 16 canaux)
- 16 servomoteurs SG90
- Alimentation 5V 8A pour les servos

BIBLIOTHEQUES REQUISES:
- Adafruit PWM Servo Driver Library (seulement)
- BLE natif ESP32 (inclus dans le core ESP32)

************************************************************************************************/

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "Instrument.h"
#include "settings.h"

// UUIDs pour BLE MIDI (standard Apple MIDI)
#define SERVICE_UUID        "03b80e5a-ede8-4b33-a751-6ce34ec4c700"
#define CHARACTERISTIC_UUID "7772e5db-3868-4112-a1a9-f2669d106bf3"

// Objets BLE
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

// Instrument
Instrument instrument;

// LED d'état
#define PIN_LED 2
unsigned long lastLedToggle = 0;
bool ledState = false;

/***********************************************************************************************
CALLBACKS BLE
************************************************************************************************/

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("[BLE] ✓ Connexion établie");
      digitalWrite(PIN_LED, HIGH);
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("[BLE] ✗ Déconnexion");
      digitalWrite(PIN_LED, LOW);
    }
};

/***********************************************************************************************
TRAITEMENT DES MESSAGES MIDI
************************************************************************************************/

void processMIDIMessage(uint8_t* data, size_t length) {
  // Format BLE MIDI: [header][timestamp][status][data1][data2]
  // On saute les 2 premiers bytes (header + timestamp)
  if (length < 3) return;

  uint8_t status = data[2] & 0xF0;  // Type de message
  uint8_t channel = data[2] & 0x0F; // Canal MIDI

  switch (status) {
    case 0x90: // Note On
      if (length >= 5) {
        uint8_t note = data[3];
        uint8_t velocity = data[4];

        if (DEBUG) {
          Serial.printf("[MIDI IN] Note On: %d (vel: %d) canal: %d\n",
                        note, velocity, channel + 1);
        }

        if (velocity > 0) {
          instrument.noteOn(note, velocity);
        } else {
          instrument.noteOff(note);  // velocity 0 = Note Off
        }
      }
      break;

    case 0x80: // Note Off
      if (length >= 4) {
        uint8_t note = data[3];

        if (DEBUG) {
          Serial.printf("[MIDI IN] Note Off: %d canal: %d\n",
                        note, channel + 1);
        }

        instrument.noteOff(note);
      }
      break;

    case 0xB0: // Control Change
      if (length >= 5) {
        uint8_t controller = data[3];
        uint8_t value = data[4];

        if (DEBUG) {
          Serial.printf("[MIDI IN] CC: %d = %d\n", controller, value);
        }

        // All Notes Off
        if (controller == 123) {
          Serial.println("[MIDI] All Notes Off");
          for (int i = MIDI_NOTE_MIN; i <= MIDI_NOTE_MAX; i++) {
            instrument.noteOff(i);
          }
        }
      }
      break;
  }
}

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      if (value.length() > 0) {
        processMIDIMessage((uint8_t*)value.data(), value.length());
      }
    }
};

/***********************************************************************************************
SETUP
************************************************************************************************/

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  delay(1000);

  Serial.println("\n========================================");
  Serial.println("  Lyre MIDI 16 notes - ESP32 BLE");
  Serial.println("      VERSION BLE NATIF 3.0");
  Serial.println("========================================\n");

  // LED
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  // Initialiser I2C
  #if defined(I2C_SDA) && defined(I2C_SCL)
    Wire.begin(I2C_SDA, I2C_SCL);
    Serial.printf("[I2C] Pins: SDA=%d, SCL=%d\n", I2C_SDA, I2C_SCL);
  #else
    Wire.begin();
    Serial.println("[I2C] Pins par défaut: SDA=21, SCL=22");
  #endif

  // Initialiser BLE
  Serial.println("[BLE] Initialisation...");
  BLEDevice::init(BLE_DEVICE_NAME);

  // Créer serveur BLE
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Créer service MIDI
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Créer caractéristique MIDI
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ   |
    BLECharacteristic::PROPERTY_WRITE  |
    BLECharacteristic::PROPERTY_NOTIFY |
    BLECharacteristic::PROPERTY_WRITE_NR
  );

  pCharacteristic->setCallbacks(new MyCallbacks());
  pCharacteristic->addDescriptor(new BLE2902());

  // Démarrer service
  pService->start();

  // Démarrer advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);
  pAdvertising->start();

  Serial.println("[BLE] ✓ Service MIDI démarré");
  Serial.printf("[BLE] Nom: %s\n", BLE_DEVICE_NAME);
  Serial.println("[BLE] En attente de connexion...");
  Serial.println("========================================\n");

  if (!DEBUG) {
    Serial.println("Pour activer le debug, définir DEBUG=1 dans settings.h\n");
  }
}

/***********************************************************************************************
LOOP
************************************************************************************************/

void loop() {
  // Mettre à jour instrument (servos, timeouts)
  instrument.update();

  // Gérer reconnexion
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // Laisser le temps à la pile BLE
    pServer->startAdvertising();
    Serial.println("[BLE] Redémarrage advertising...");
    oldDeviceConnected = deviceConnected;
  }

  // Gérer nouvelle connexion
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }

  // LED clignotante si non connecté
  if (!deviceConnected) {
    unsigned long now = millis();
    if (now - lastLedToggle >= 500) {
      ledState = !ledState;
      digitalWrite(PIN_LED, ledState);
      lastLedToggle = now;
    }
  }

  // Message périodique si non connecté
  static unsigned long lastStatusTime = 0;
  if (!deviceConnected && millis() - lastStatusTime > 30000) {
    Serial.println("[BLE] En attente de connexion...");
    lastStatusTime = millis();
  }

  delay(10);
}
