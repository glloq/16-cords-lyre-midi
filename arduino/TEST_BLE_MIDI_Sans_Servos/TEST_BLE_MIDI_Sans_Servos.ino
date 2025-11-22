/***********************************************************************************************
TEST BLE MIDI ESP32 - Version sans servos
Pour tester uniquement la connexion BLE MIDI sans matériel
************************************************************************************************/

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Configuration
#define BLE_DEVICE_NAME "Lyre-MIDI-Test"
#define PIN_LED 2
#define SERIAL_BAUD_RATE 115200

// UUIDs pour BLE MIDI
#define SERVICE_UUID        "03b80e5a-ede8-4b33-a751-6ce34ec4c700"
#define CHARACTERISTIC_UUID "7772e5db-3868-4112-a1a9-f2669d106bf3"

// Objets BLE
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;

// Statistiques
uint32_t midiMessagesReceived = 0;
uint32_t noteOnCount = 0;
uint32_t noteOffCount = 0;

/***********************************************************************************************
CALLBACKS BLE
************************************************************************************************/

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("\n[BLE] ✓✓✓ CONNEXION ETABLIE ✓✓✓");
      digitalWrite(PIN_LED, HIGH);
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("\n[BLE] ✗ Déconnexion");
      digitalWrite(PIN_LED, LOW);

      // Redémarrer advertising
      delay(500);
      pServer->getAdvertising()->start();
      Serial.println("[BLE] Advertising redémarré");
    }
};

/***********************************************************************************************
TRAITEMENT MESSAGES MIDI
************************************************************************************************/

void processMIDIMessage(uint8_t* data, size_t length) {
  if (length < 3) return;

  midiMessagesReceived++;

  uint8_t status = data[2] & 0xF0;
  uint8_t channel = data[2] & 0x0F;

  switch (status) {
    case 0x90: // Note On
      if (length >= 5) {
        uint8_t note = data[3];
        uint8_t velocity = data[4];

        Serial.printf("[MIDI] ♪ Note On: %d (vel: %d) canal: %d\n",
                      note, velocity, channel + 1);

        if (velocity > 0) {
          noteOnCount++;
          // Ici on jouerait la note sur les servos
          Serial.println("       → Servo jouerait cette note!");
        } else {
          noteOffCount++;
        }
      }
      break;

    case 0x80: // Note Off
      if (length >= 4) {
        uint8_t note = data[3];
        Serial.printf("[MIDI] ♪ Note Off: %d canal: %d\n",
                      note, channel + 1);
        noteOffCount++;
        Serial.println("       → Servo reviendrait au repos");
      }
      break;

    case 0xB0: // Control Change
      if (length >= 5) {
        uint8_t controller = data[3];
        uint8_t value = data[4];
        Serial.printf("[MIDI] CC: %d = %d\n", controller, value);

        if (controller == 123) {
          Serial.println("[MIDI] ♪♪♪ ALL NOTES OFF");
        }
      }
      break;
  }
}

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
SETUP
************************************************************************************************/

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  delay(2000);

  Serial.println("\n\n");
  Serial.println("================================================");
  Serial.println("     TEST BLE MIDI ESP32 (sans servos)");
  Serial.println("================================================");
  Serial.println();

  // LED
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  // Initialiser BLE
  Serial.println("[BLE] Initialisation BLE...");
  BLEDevice::init(BLE_DEVICE_NAME);
  Serial.println("[BLE] ✓ BLE Device initialisé");

  // Créer serveur
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  Serial.println("[BLE] ✓ Serveur BLE créé");

  // Créer service MIDI
  BLEService *pService = pServer->createService(SERVICE_UUID);
  Serial.println("[BLE] ✓ Service MIDI créé");

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
  Serial.println("[BLE] ✓ Caractéristique MIDI créée");

  // Démarrer service
  pService->start();
  Serial.println("[BLE] ✓ Service démarré");

  // Démarrer advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);
  pAdvertising->start();

  Serial.println();
  Serial.println("================================================");
  Serial.println("  ✓✓✓ BLE MIDI PRET ✓✓✓");
  Serial.println("================================================");
  Serial.printf("  Nom Bluetooth: %s\n", BLE_DEVICE_NAME);
  Serial.println("  Cherchez cet appareil dans votre app MIDI");
  Serial.println("================================================");
  Serial.println();
}

/***********************************************************************************************
LOOP
************************************************************************************************/

void loop() {
  // LED clignotante si non connecté
  static unsigned long lastLedToggle = 0;
  static bool ledState = false;

  if (!deviceConnected) {
    unsigned long now = millis();
    if (now - lastLedToggle >= 500) {
      ledState = !ledState;
      digitalWrite(PIN_LED, ledState);
      lastLedToggle = now;
    }
  }

  // Statistiques périodiques
  static unsigned long lastStats = 0;
  if (millis() - lastStats >= 10000) {
    Serial.println("\n--- Statistiques ---");
    Serial.printf("Connecté: %s\n", deviceConnected ? "OUI" : "NON");
    Serial.printf("Messages MIDI reçus: %d\n", midiMessagesReceived);
    Serial.printf("Note On: %d | Note Off: %d\n", noteOnCount, noteOffCount);
    Serial.println("-------------------\n");
    lastStats = millis();
  }

  delay(10);
}
