/*
 * TEST BLE MIDI - Version alternative
 * Utilise l'approche BLE native ESP32
 */

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer* pServer = NULL;
bool deviceConnected = false;

// Callback de connexion
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("✓ APPAREIL CONNECTÉ !");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("✗ Déconnecté");
      // Redémarrer l'advertising
      BLEDevice::startAdvertising();
      Serial.println("→ Advertising redémarré");
    }
};

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("\n=== TEST BLE NATIF ESP32 ===");
  Serial.println("Initialisation BLE...");

  // Créer appareil BLE
  BLEDevice::init("TEST-ESP32-NATIF");

  // Créer serveur BLE
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Démarrer advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->start();

  Serial.println("✓ BLE démarré en mode natif!");
  Serial.println("Cherchez 'TEST-ESP32-NATIF' en Bluetooth");
  Serial.println("(Devrait apparaître dans Bluetooth normal ET apps BLE)");
  Serial.println("============================\n");
}

void loop() {
  static unsigned long lastPrint = 0;

  if (millis() - lastPrint > 5000) {
    if (deviceConnected) {
      Serial.println("État: CONNECTÉ ✓");
    } else {
      Serial.println("État: En attente de connexion...");
    }
    lastPrint = millis();
  }

  delay(100);
}
