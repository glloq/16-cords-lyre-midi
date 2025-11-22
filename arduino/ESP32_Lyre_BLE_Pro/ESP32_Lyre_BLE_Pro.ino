/***********************************************************************************************
-----------------------  ESP32 BLE MIDI Lyre - Version Professionnelle  ------------------------
************************************************************************************************
Version optimisée avec:
- Appairage automatique (pas de bouton requis)
- Réponse SysEx Identity Request (identification automatique)
- Messages MIDI d'état de connexion
- Performance optimisée pour réactivité maximale
- Compatible toutes apps MIDI BLE

MATERIEL REQUIS:
- ESP32 (WROOM-32D testé)
- PCA9685 (contrôleur PWM I2C)
- 16 servomoteurs SG90
- Alimentation 5V 8A

BIBLIOTHEQUES:
- Adafruit PWM Servo Driver Library
- BLE natif ESP32 (inclus)
************************************************************************************************/

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "Instrument.h"
#include "settings.h"

// Configuration
#define FIRMWARE_NAME "ESP32-Lyre"
#define FIRMWARE_VERSION "1.0"
#define MANUFACTURER_ID 0x7D  // Educational/Development use

// UUIDs BLE MIDI standard
#define SERVICE_UUID        "03b80e5a-ede8-4b33-a751-6ce34ec4c700"
#define CHARACTERISTIC_UUID "7772e5db-3868-4112-a1a9-f2669d106bf3"

// Objets BLE
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool readyToSend = false;

// Instrument
Instrument instrument;

// Statistiques
uint32_t midiMessagesReceived = 0;
uint32_t lastActivityTime = 0;

/***********************************************************************************************
ENVOI MESSAGES MIDI BLE
************************************************************************************************/

void sendMIDIMessage(uint8_t* data, size_t length) {
  if (!deviceConnected || !readyToSend || !pCharacteristic) return;

  // Format BLE MIDI: [header][timestamp][MIDI bytes]
  uint8_t blePacket[length + 2];
  blePacket[0] = 0x80;  // Header (bit 7 = 1)
  blePacket[1] = 0x80;  // Timestamp high bit

  memcpy(&blePacket[2], data, length);

  pCharacteristic->setValue(blePacket, length + 2);
  pCharacteristic->notify();
}

// Envoyer Note On
void sendNoteOn(uint8_t note, uint8_t velocity, uint8_t channel = 0) {
  uint8_t msg[3] = {(uint8_t)(0x90 | channel), note, velocity};
  sendMIDIMessage(msg, 3);
}

// Envoyer Note Off
void sendNoteOff(uint8_t note, uint8_t channel = 0) {
  uint8_t msg[3] = {(uint8_t)(0x80 | channel), note, 0};
  sendMIDIMessage(msg, 3);
}

// Envoyer Control Change
void sendControlChange(uint8_t controller, uint8_t value, uint8_t channel = 0) {
  uint8_t msg[3] = {(uint8_t)(0xB0 | channel), controller, value};
  sendMIDIMessage(msg, 3);
}

// Envoyer SysEx Identity Reply
void sendIdentityReply() {
  // Format: F0 7E 7F 06 02 [ManufID] [Family] [Model] [Version] F7
  uint8_t identity[] = {
    0xF0,           // SysEx Start
    0x7E,           // Universal Non-Real Time
    0x7F,           // Device ID (all devices)
    0x06,           // General Information
    0x02,           // Identity Reply
    MANUFACTURER_ID,// Manufacturer ID
    0x00,           // Family LSB
    0x01,           // Family MSB
    0x00,           // Model LSB
    0x01,           // Model MSB
    0x01, 0x00, 0x00, 0x00, // Version 1.0.0.0
    0xF7            // SysEx End
  };

  sendMIDIMessage(identity, sizeof(identity));

  if (DEBUG) {
    Serial.println("[SYSEX] Identity Reply envoyé");
  }
}

// Envoyer message de connexion établie
void sendConnectionEstablished() {
  // CC 102 = Connection Status (127 = connected)
  sendControlChange(102, 127);

  if (DEBUG) {
    Serial.println("[MIDI OUT] Connection Status: Connected (CC 102 = 127)");
  }
}

// Envoyer message de déconnexion
void sendDisconnectionWarning() {
  // CC 102 = Connection Status (0 = disconnected)
  sendControlChange(102, 0);

  if (DEBUG) {
    Serial.println("[MIDI OUT] Connection Status: Disconnected (CC 102 = 0)");
  }
}

/***********************************************************************************************
TRAITEMENT MESSAGES MIDI RECUS
************************************************************************************************/

void processMIDIMessage(uint8_t* data, size_t length) {
  if (length < 3) return;

  lastActivityTime = millis();
  midiMessagesReceived++;

  uint8_t status = data[2] & 0xF0;
  uint8_t channel = data[2] & 0x0F;

  switch (status) {
    case 0x90: // Note On
      if (length >= 5) {
        uint8_t note = data[3];
        uint8_t velocity = data[4];

        if (DEBUG) {
          Serial.printf("[MIDI IN] Note On: %d (vel: %d)\n", note, velocity);
        }

        if (velocity > 0) {
          instrument.noteOn(note, velocity);
          // Feedback: confirmer note jouée
          if (MIDI_SEND_FEEDBACK) {
            sendNoteOn(note, velocity, channel);
          }
        } else {
          instrument.noteOff(note);
          if (MIDI_SEND_FEEDBACK) {
            sendNoteOff(note, channel);
          }
        }
      }
      break;

    case 0x80: // Note Off
      if (length >= 4) {
        uint8_t note = data[3];

        if (DEBUG) {
          Serial.printf("[MIDI IN] Note Off: %d\n", note);
        }

        instrument.noteOff(note);
        if (MIDI_SEND_FEEDBACK) {
          sendNoteOff(note, channel);
        }
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

    case 0xF0: // System Messages (SysEx)
      // Vérifier si c'est Identity Request
      if (length >= 6 &&
          data[2] == 0xF0 &&  // SysEx Start
          data[3] == 0x7E &&  // Universal Non-Real Time
          data[5] == 0x06) {  // General Information

        uint8_t subID = data[6];
        if (subID == 0x01) {  // Identity Request
          if (DEBUG) {
            Serial.println("[SYSEX] Identity Request reçu");
          }
          sendIdentityReply();
        }
      }
      break;
  }
}

/***********************************************************************************************
CALLBACKS BLE
************************************************************************************************/

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      readyToSend = true;

      Serial.println("\n[BLE] ✓ CONNEXION ETABLIE");

      // Délai court pour stabiliser la connexion
      delay(100);

      // Envoyer message de connexion
      sendConnectionEstablished();

      // Envoyer Identity (pour que l'interface sache immédiatement qui on est)
      delay(50);
      sendIdentityReply();
    };

    void onDisconnect(BLEServer* pServer) {
      readyToSend = false;
      deviceConnected = false;

      Serial.println("[BLE] ✗ Déconnexion");

      // Redémarrer advertising automatiquement
      delay(500);
      pServer->getAdvertising()->start();
      Serial.println("[BLE] Advertising redémarré (prêt pour nouvelle connexion)");
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      // Compatible toutes versions BLE ESP32
      uint8_t* data = pCharacteristic->getData();
      size_t length = pCharacteristic->getValue().length();

      if (length > 0) {
        processMIDIMessage(data, length);
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
  Serial.println("  ESP32 BLE MIDI Lyre - Pro Version");
  Serial.println("========================================");
  Serial.printf("Firmware: %s v%s\n", FIRMWARE_NAME, FIRMWARE_VERSION);
  Serial.println();

  // Initialiser I2C
  #if defined(I2C_SDA) && defined(I2C_SCL)
    Wire.begin(I2C_SDA, I2C_SCL);
    Serial.printf("[I2C] SDA=%d, SCL=%d\n", I2C_SDA, I2C_SCL);
  #else
    Wire.begin();
    Serial.println("[I2C] Pins par défaut (SDA=21, SCL=22)");
  #endif

  // Initialiser BLE
  Serial.println("[BLE] Initialisation...");
  BLEDevice::init(BLE_DEVICE_NAME);
  BLEDevice::setMTU(517); // MTU max pour BLE MIDI (optimisation)

  // Créer serveur
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Créer service MIDI
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Créer caractéristique
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

  // Configurer advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x06); // Connexion rapide (7.5ms)

  // Démarrer advertising
  pAdvertising->start();

  Serial.println("[BLE] ✓ Service MIDI actif");
  Serial.printf("[BLE] Nom: %s\n", BLE_DEVICE_NAME);
  Serial.println("[BLE] Appairage permanent activé");
  Serial.println("========================================");
  Serial.println("\nEn attente de connexion...\n");

  lastActivityTime = millis();
}

/***********************************************************************************************
LOOP
************************************************************************************************/

void loop() {
  // Mettre à jour instrument
  instrument.update();

  // Heartbeat périodique si connecté (toutes les 30 secondes)
  static unsigned long lastHeartbeat = 0;
  if (deviceConnected && millis() - lastHeartbeat >= 30000) {
    // CC 103 = Heartbeat/Alive
    sendControlChange(103, 127);

    if (DEBUG) {
      Serial.printf("[STATS] Messages reçus: %lu | Uptime: %lu s\n",
                    midiMessagesReceived, millis() / 1000);
    }

    lastHeartbeat = millis();
  }

  // Message périodique si non connecté
  static unsigned long lastStatusMsg = 0;
  if (!deviceConnected && millis() - lastStatusMsg >= 30000) {
    Serial.println("[BLE] En attente de connexion...");
    lastStatusMsg = millis();
  }

  // Délai minimal pour réactivité
  delay(1);
}
