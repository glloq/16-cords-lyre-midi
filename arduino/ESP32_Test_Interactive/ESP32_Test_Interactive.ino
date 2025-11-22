/***********************************************************************************************
------------------------  ESP32 BLE MIDI - VERSION TEST INTERACTIVE  ---------------------------
************************************************************************************************
Version de test COMPLÈTE contrôlable via Serial Monitor
AUCUN matériel requis (pas de servos, boutons, LED)

FONCTIONNALITÉS:
- Réception et affichage de tous les messages MIDI
- Envoi de messages MIDI via commandes Serial
- Réponse automatique SysEx Identity Request
- Feedback MIDI configurable
- Statistiques en temps réel
- Heartbeat automatique
- État de connexion détaillé

COMMANDES SERIAL MONITOR:
  n<note> <vel>   - Envoyer Note On (ex: n60 100)
  o<note>         - Envoyer Note Off (ex: o60)
  c<cc> <val>     - Envoyer Control Change (ex: c7 127)
  i               - Envoyer SysEx Identity Reply
  s               - Afficher statistiques
  h               - Afficher aide
  f               - Toggle feedback MIDI ON/OFF

************************************************************************************************/

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Configuration
#define BLE_DEVICE_NAME "Lyre-Test"
#define SERIAL_BAUD_RATE 115200
#define FIRMWARE_VERSION "1.0"
#define MANUFACTURER_ID 0x7D

// UUIDs BLE MIDI
#define SERVICE_UUID        "03b80e5a-ede8-4b33-a751-6ce34ec4c700"
#define CHARACTERISTIC_UUID "7772e5db-3868-4112-a1a9-f2669d106bf3"

// Objets BLE
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool readyToSend = false;

// Configuration
bool feedbackEnabled = true;
bool autoHeartbeat = true;

// Statistiques
struct Stats {
  uint32_t messagesReceived;
  uint32_t messagesSent;
  uint32_t noteOnReceived;
  uint32_t noteOffReceived;
  uint32_t ccReceived;
  uint32_t sysexReceived;
  unsigned long connectionTime;
  unsigned long lastMessageTime;
} stats;

/***********************************************************************************************
FONCTIONS D'ENVOI MIDI
************************************************************************************************/

void sendBLEMIDI(uint8_t* data, size_t length) {
  if (!deviceConnected || !readyToSend || !pCharacteristic) {
    Serial.println("❌ Impossible d'envoyer: pas connecté");
    return;
  }

  uint8_t blePacket[length + 2];
  blePacket[0] = 0x80;
  blePacket[1] = 0x80;
  memcpy(&blePacket[2], data, length);

  pCharacteristic->setValue(blePacket, length + 2);
  pCharacteristic->notify();
  stats.messagesSent++;
}

void sendNoteOn(uint8_t note, uint8_t velocity, uint8_t channel = 0) {
  uint8_t msg[3] = {(uint8_t)(0x90 | channel), note, velocity};
  sendBLEMIDI(msg, 3);
  Serial.printf("📤 Envoyé: Note On %d, vel %d, ch %d\n", note, velocity, channel);
}

void sendNoteOff(uint8_t note, uint8_t channel = 0) {
  uint8_t msg[3] = {(uint8_t)(0x80 | channel), note, 0};
  sendBLEMIDI(msg, 3);
  Serial.printf("📤 Envoyé: Note Off %d, ch %d\n", note, channel);
}

void sendControlChange(uint8_t cc, uint8_t value, uint8_t channel = 0) {
  uint8_t msg[3] = {(uint8_t)(0xB0 | channel), cc, value};
  sendBLEMIDI(msg, 3);
  Serial.printf("📤 Envoyé: CC %d = %d, ch %d\n", cc, value, channel);
}

void sendIdentityReply() {
  uint8_t identity[] = {
    0xF0, 0x7E, 0x7F, 0x06, 0x02,
    MANUFACTURER_ID,
    0x00, 0x01,  // Family
    0x00, 0x01,  // Model
    0x01, 0x00, 0x00, 0x00,  // Version
    0xF7
  };
  sendBLEMIDI(identity, sizeof(identity));
  Serial.println("📤 Envoyé: SysEx Identity Reply");
  Serial.println("   F0 7E 7F 06 02 7D 00 01 00 01 01 00 00 00 F7");
}

void sendConnectionStatus(bool connected) {
  sendControlChange(102, connected ? 127 : 0);
  Serial.printf("📤 Envoyé: Connection Status = %s\n", connected ? "CONNECTED" : "DISCONNECTED");
}

void sendHeartbeat() {
  sendControlChange(103, 127);
  Serial.println("💓 Envoyé: Heartbeat");
}

/***********************************************************************************************
TRAITEMENT MESSAGES MIDI REÇUS
************************************************************************************************/

void processMIDIMessage(uint8_t* data, size_t length) {
  if (length < 3) return;

  stats.messagesReceived++;
  stats.lastMessageTime = millis();

  uint8_t status = data[2] & 0xF0;
  uint8_t channel = data[2] & 0x0F;

  Serial.println("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");

  switch (status) {
    case 0x90: // Note On
      if (length >= 5) {
        uint8_t note = data[3];
        uint8_t velocity = data[4];

        stats.noteOnReceived++;
        Serial.println("📥 REÇU: Note On");
        Serial.printf("   Note: %d (0x%02X)\n", note, note);
        Serial.printf("   Velocity: %d\n", velocity);
        Serial.printf("   Canal: %d\n", channel);

        if (velocity > 0) {
          Serial.println("   → Servo jouerait cette note");

          if (feedbackEnabled) {
            delay(2);  // Réduit à 2ms pour réactivité
            sendNoteOn(note, velocity, channel);
            Serial.println("   ✓ Feedback envoyé");
          }
        } else {
          stats.noteOffReceived++;
          Serial.println("   (velocity 0 = Note Off)");
          if (feedbackEnabled) {
            delay(2);  // Réduit à 2ms pour réactivité
            sendNoteOff(note, channel);
          }
        }
      }
      break;

    case 0x80: // Note Off
      if (length >= 4) {
        uint8_t note = data[3];

        stats.noteOffReceived++;
        Serial.println("📥 REÇU: Note Off");
        Serial.printf("   Note: %d (0x%02X)\n", note, note);
        Serial.printf("   Canal: %d\n", channel);
        Serial.println("   → Servo reviendrait au repos");

        if (feedbackEnabled) {
          delay(2);  // Réduit à 2ms pour réactivité
          sendNoteOff(note, channel);
          Serial.println("   ✓ Feedback envoyé");
        }
      }
      break;

    case 0xB0: // Control Change
      if (length >= 5) {
        uint8_t controller = data[3];
        uint8_t value = data[4];

        stats.ccReceived++;
        Serial.println("📥 REÇU: Control Change");
        Serial.printf("   Controller: %d\n", controller);
        Serial.printf("   Value: %d\n", value);
        Serial.printf("   Canal: %d\n", channel);

        if (controller == 123) {
          Serial.println("   → All Notes Off (tous les servos au repos)");
        }
      }
      break;

    case 0xF0: // System / SysEx
      stats.sysexReceived++;
      Serial.println("📥 REÇU: System / SysEx");

      // Afficher les bytes
      Serial.print("   Data: ");
      for (size_t i = 2; i < length && i < 20; i++) {
        Serial.printf("%02X ", data[i]);
      }
      Serial.println();

      // Identity Request
      if (length >= 7 && data[2] == 0xF0 && data[3] == 0x7E && data[5] == 0x06 && data[6] == 0x01) {
        Serial.println("   → Identity Request détecté!");
        Serial.println("   → Envoi Identity Reply...");
        delay(2);  // Réduit à 2ms pour réactivité
        sendIdentityReply();
      }
      break;

    default:
      Serial.printf("📥 REÇU: Autre (0x%02X)\n", status);
      Serial.print("   Data: ");
      for (size_t i = 2; i < length && i < 10; i++) {
        Serial.printf("%02X ", data[i]);
      }
      Serial.println();
      break;
  }

  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
}

/***********************************************************************************************
CALLBACKS BLE
************************************************************************************************/

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      readyToSend = true;
      stats.connectionTime = millis();

      Serial.println("\n╔═══════════════════════════════════╗");
      Serial.println("║   ✓✓✓ CONNEXION ETABLIE ✓✓✓      ║");
      Serial.println("╚═══════════════════════════════════╝\n");

      delay(10);  // Réduit à 10ms pour connexion rapide
      sendConnectionStatus(true);
      delay(5);   // Réduit à 5ms
      sendIdentityReply();
    };

    void onDisconnect(BLEServer* pServer) {
      readyToSend = false;
      deviceConnected = false;

      Serial.println("\n╔═══════════════════════════════════╗");
      Serial.println("║      ✗ DECONNEXION ✗             ║");
      Serial.println("╚═══════════════════════════════════╝\n");

      delay(500);
      pServer->getAdvertising()->start();
      Serial.println("🔄 Advertising redémarré\n");
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      uint8_t* data = pCharacteristic->getData();
      size_t length = pCharacteristic->getValue().length();

      if (length > 0) {
        processMIDIMessage(data, length);
      }
    }
};

/***********************************************************************************************
STATISTIQUES
************************************************************************************************/

void printStats() {
  Serial.println("\n╔════════════════════ STATISTIQUES ═════════════════════╗");
  Serial.printf("║ Connecté:          %s\n", deviceConnected ? "OUI ✓" : "NON ✗");

  if (deviceConnected) {
    unsigned long uptime = (millis() - stats.connectionTime) / 1000;
    Serial.printf("║ Durée connexion:   %lu s (%lu min)\n", uptime, uptime / 60);
  }

  Serial.println("║ ─────────────────────────────────────────────────────── ║");
  Serial.printf("║ Messages reçus:    %lu\n", stats.messagesReceived);
  Serial.printf("║ Messages envoyés:  %lu\n", stats.messagesSent);
  Serial.println("║ ─────────────────────────────────────────────────────── ║");
  Serial.printf("║ Note On reçus:     %lu\n", stats.noteOnReceived);
  Serial.printf("║ Note Off reçus:    %lu\n", stats.noteOffReceived);
  Serial.printf("║ CC reçus:          %lu\n", stats.ccReceived);
  Serial.printf("║ SysEx reçus:       %lu\n", stats.sysexReceived);
  Serial.println("║ ─────────────────────────────────────────────────────── ║");
  Serial.printf("║ Feedback MIDI:     %s\n", feedbackEnabled ? "ACTIVÉ ✓" : "DÉSACTIVÉ ✗");
  Serial.printf("║ Auto Heartbeat:    %s\n", autoHeartbeat ? "ACTIVÉ ✓" : "DÉSACTIVÉ ✗");
  Serial.println("╚════════════════════════════════════════════════════════╝\n");
}

void printHelp() {
  Serial.println("\n╔═══════════════════ COMMANDES ══════════════════════╗");
  Serial.println("║ ENVOI DE MESSAGES:                                  ║");
  Serial.println("║   n<note> <vel>  - Note On (ex: n60 100)           ║");
  Serial.println("║   o<note>        - Note Off (ex: o60)              ║");
  Serial.println("║   c<cc> <val>    - Control Change (ex: c7 127)     ║");
  Serial.println("║   i              - Envoyer SysEx Identity Reply    ║");
  Serial.println("║ ─────────────────────────────────────────────────── ║");
  Serial.println("║ INFORMATION:                                        ║");
  Serial.println("║   s              - Statistiques                    ║");
  Serial.println("║   h              - Cette aide                      ║");
  Serial.println("║ ─────────────────────────────────────────────────── ║");
  Serial.println("║ CONFIGURATION:                                      ║");
  Serial.println("║   f              - Toggle Feedback MIDI            ║");
  Serial.println("║   t              - Toggle Auto Heartbeat           ║");
  Serial.println("╚═════════════════════════════════════════════════════╝\n");
}

/***********************************************************************************************
COMMANDES SERIAL
************************************************************************************************/

void processSerialCommand() {
  if (!Serial.available()) return;

  String cmd = Serial.readStringUntil('\n');
  cmd.trim();

  if (cmd.length() == 0) return;

  Serial.printf("\n> %s\n", cmd.c_str());

  char command = cmd.charAt(0);

  switch (command) {
    case 'n': { // Note On
      int note, vel;
      if (sscanf(cmd.c_str(), "n%d %d", &note, &vel) == 2) {
        if (note >= 0 && note <= 127 && vel >= 0 && vel <= 127) {
          sendNoteOn(note, vel);
        } else {
          Serial.println("❌ Note et vélocité doivent être 0-127");
        }
      } else {
        Serial.println("❌ Format: n<note> <vel> (ex: n60 100)");
      }
      break;
    }

    case 'o': { // Note Off
      int note;
      if (sscanf(cmd.c_str(), "o%d", &note) == 1) {
        if (note >= 0 && note <= 127) {
          sendNoteOff(note);
        } else {
          Serial.println("❌ Note doit être 0-127");
        }
      } else {
        Serial.println("❌ Format: o<note> (ex: o60)");
      }
      break;
    }

    case 'c': { // Control Change
      int cc, val;
      if (sscanf(cmd.c_str(), "c%d %d", &cc, &val) == 2) {
        if (cc >= 0 && cc <= 127 && val >= 0 && val <= 127) {
          sendControlChange(cc, val);
        } else {
          Serial.println("❌ CC et value doivent être 0-127");
        }
      } else {
        Serial.println("❌ Format: c<cc> <val> (ex: c7 127)");
      }
      break;
    }

    case 'i': // Identity Reply
      sendIdentityReply();
      break;

    case 's': // Stats
      printStats();
      break;

    case 'h': // Help
      printHelp();
      break;

    case 'f': // Toggle Feedback
      feedbackEnabled = !feedbackEnabled;
      Serial.printf("🔄 Feedback MIDI: %s\n", feedbackEnabled ? "ACTIVÉ" : "DÉSACTIVÉ");
      break;

    case 't': // Toggle Heartbeat
      autoHeartbeat = !autoHeartbeat;
      Serial.printf("🔄 Auto Heartbeat: %s\n", autoHeartbeat ? "ACTIVÉ" : "DÉSACTIVÉ");
      break;

    default:
      Serial.printf("❌ Commande inconnue: %c\n", command);
      Serial.println("Tapez 'h' pour aide");
      break;
  }
}

/***********************************************************************************************
SETUP
************************************************************************************************/

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  delay(500);  // Réduit de 2000 à 500ms pour démarrage rapide

  Serial.println("\n\n");
  Serial.println("╔════════════════════════════════════════════════════╗");
  Serial.println("║  ESP32 BLE MIDI - VERSION TEST INTERACTIVE        ║");
  Serial.println("║  Contrôle complet via Serial Monitor              ║");
  Serial.println("╚════════════════════════════════════════════════════╝");
  Serial.printf("  Firmware: v%s\n", FIRMWARE_VERSION);
  Serial.println();

  // Initialiser stats
  memset(&stats, 0, sizeof(stats));

  // Initialiser BLE
  Serial.println("[BLE] Initialisation...");
  BLEDevice::init(BLE_DEVICE_NAME);
  BLEDevice::setMTU(517);

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ   |
    BLECharacteristic::PROPERTY_WRITE  |
    BLECharacteristic::PROPERTY_NOTIFY |
    BLECharacteristic::PROPERTY_WRITE_NR
  );

  pCharacteristic->setCallbacks(new MyCallbacks());
  pCharacteristic->addDescriptor(new BLE2902());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);  // Active scan response

  // OPTIMISATION APPAIRAGE RAPIDE
  pAdvertising->setMinInterval(0x20);   // 20ms (rapide)
  pAdvertising->setMaxInterval(0x40);   // 40ms
  pAdvertising->setMinPreferred(0x06);  // 7.5ms connexion
  pAdvertising->setMaxPreferred(0x0C);  // 15ms max

  pAdvertising->start();

  Serial.println("[BLE] ✓ Service MIDI actif (appairage optimisé)");
  Serial.printf("[BLE] Nom: %s\n", BLE_DEVICE_NAME);
  Serial.println("═══════════════════════════════════════════════════════");
  Serial.println();
  Serial.println("📱 Cherchez 'Lyre-Test' dans votre app MIDI BLE");
  Serial.println();

  printHelp();
}

/***********************************************************************************************
LOOP
************************************************************************************************/

void loop() {
  // Traiter commandes Serial
  processSerialCommand();

  // Heartbeat automatique (toutes les 30s)
  static unsigned long lastHeartbeat = 0;
  if (deviceConnected && autoHeartbeat && millis() - lastHeartbeat >= 30000) {
    sendHeartbeat();
    lastHeartbeat = millis();
  }

  delay(10);
}
