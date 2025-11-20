/***********************************************************************************************
----------------------------    MIDI servo lyre  16 notes   ------------------------------------
************************************************************************************************
Systeme construit pour le controle d'une lyre de 16 notes avec des servomoteur de type sg90 et une carte pac9685
les systeme recoit les messages midi via le cable usb, midiHandler s'occupe de dechiffrer les messages midi
instrument s'occupe de verifier si il peut jouer les notes recues et demande ServoController de les jouer si c'est possible

tout les parametres doivent etre mis dan settings.h afin de simplifier les adaptations au materiel 
Un autre fichier .ino sera fournit afin d'initialiser les servo a 90° lors du montage et aussi pour trouver le reglage de la position centrale du servo 

************************************************************************************************/
#include <MIDIUSB.h>
#include "Instrument.h"
#include "MidiHandler.h"
#include "Arduino.h"

Instrument instrument;
MidiHandler* midiHandler = nullptr;

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println("init");
  midiHandler = new MidiHandler(instrument);
  Serial.println("fin init");
}

void loop() {
  // Mettre a jour l'instrument (gestion de l'initialisation non-bloquante)
  instrument.update();

  // Lire et traiter les messages MIDI seulement si l'instrument est pret
  if (instrument.isReady()) {
    midiHandler->readMidi();
  }
}
