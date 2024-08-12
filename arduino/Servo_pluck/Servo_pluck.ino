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

Instrument* instrument= nullptr;
MidiHandler* midiHandler= nullptr;

void setup() {
  Serial.begin(115200);
 // while (!Serial) {
  //  delay(10); // Attendre que la connexion série soit établie
  //}
  Serial.println("init");
  instrument= new Instrument();
  midiHandler = new MidiHandler(*instrument);
  Serial.println("fin init");
}

void loop() {
  midiHandler->readMidi();
}
