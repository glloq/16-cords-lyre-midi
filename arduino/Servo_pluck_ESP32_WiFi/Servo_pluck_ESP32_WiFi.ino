/***********************************************************************************************
----------------------------    MIDI servo lyre 16 notes ESP32 WiFi  ---------------------------
************************************************************************************************
Systeme construit pour le controle d'une lyre de 16 notes avec des servomoteurs de type SG90
et une carte PCA9685, utilisant un ESP32 avec MIDI via WiFi (RTP-MIDI/AppleMIDI).

Le systeme recoit les messages MIDI via WiFi, MidiHandler s'occupe de dechiffrer les messages
MIDI, instrument verifie si il peut jouer les notes recues et demande a ServoController de
les jouer si c'est possible.

Tous les parametres doivent etre mis dans settings.h afin de simplifier les adaptations au
materiel.

BIBLIOTHEQUES REQUISES:
- WiFi (incluse avec ESP32)
- Wire (incluse avec ESP32)
- Adafruit_PWMServoDriver
- AppleMIDI (https://github.com/lathoub/Arduino-AppleMIDI-Library)

CONFIGURATION WIFI:
Modifier WIFI_SSID et WIFI_PASSWORD dans settings.h

CONNEXION MIDI:
1. Connecter l'ESP32 au meme reseau WiFi que votre ordinateur
2. Utiliser un logiciel compatible RTP-MIDI:
   - macOS: Audio MIDI Setup (natif)
   - Windows: rtpMIDI (https://www.tobias-erichsen.de/software/rtpmidi.html)
   - Linux: QmidiNet
3. Rechercher la session "ESP32-Lyre-MIDI" et se connecter

************************************************************************************************/
#include <WiFi.h>
#include "Instrument.h"
#include "MidiHandler.h"
#include "settings.h"

Instrument instrument;
MidiHandler* midiHandler = nullptr;

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  delay(500);
  Serial.println("\n==============================================");
  Serial.println("   ESP32 Lyre MIDI via WiFi");
  Serial.println("==============================================");

  // Initialiser I2C avec les pins ESP32
  Wire.begin(PIN_SDA, PIN_SCL);

  // Connexion WiFi
  Serial.print("[WiFi] Connexion a ");
  Serial.print(WIFI_SSID);
  Serial.print(" ");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(" OK!");
    Serial.print("[WiFi] Adresse IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println(" ECHEC!");
    Serial.println("[WiFi] Impossible de se connecter au WiFi");
    Serial.println("[WiFi] Verifiez WIFI_SSID et WIFI_PASSWORD dans settings.h");
    while(1) {
      delay(1000);
    }
  }

  // Initialiser le gestionnaire MIDI
  midiHandler = new MidiHandler(instrument);
  midiHandler->begin();

  Serial.println("==============================================");
  Serial.println("[INIT] Initialisation terminee");
  Serial.println("==============================================\n");
}

void loop() {
  // Mettre a jour l'instrument (gestion de l'initialisation non-bloquante)
  instrument.update();

  // Lire et traiter les messages MIDI seulement si l'instrument est pret
  if (instrument.isReady()) {
    midiHandler->update();
  }

  // Petite pause pour ne pas surcharger le CPU
  delay(1);
}
