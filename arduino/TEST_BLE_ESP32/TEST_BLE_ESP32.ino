/*
 * TEST BLE ESP32 - Version minimale
 * Vérifie si le Bluetooth fonctionne
 */

#include <BLEMIDI_Transport.h>
#include <hardware/BLEMIDI_ESP32.h>

BLEMIDI_CREATE_INSTANCE("TEST-ESP32", MIDI)

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("\n=== TEST BLE ESP32 ===");
  Serial.println("Initialisation BLE...");

  MIDI.begin();

  Serial.println("✓ BLE démarré!");
  Serial.println("Cherchez 'TEST-ESP32' en Bluetooth");
  Serial.println("======================\n");
}

void loop() {
  static unsigned long lastPrint = 0;

  if (millis() - lastPrint > 5000) {
    Serial.println("BLE actif... (scan avec votre appareil)");
    lastPrint = millis();
  }

  delay(100);
}
