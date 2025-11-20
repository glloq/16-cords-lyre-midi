#include "Instrument.h"

Instrument::Instrument() : servoController() {
  if (DEBUG) {
    Serial.println("[INSTRUMENT] Demarrage de l'initialisation");
  }
}

void Instrument::update() {
  servoController.update();
}

bool Instrument::isReady() {
  return servoController.isInitComplete();
}

int16_t Instrument::getServo(uint8_t midiNote) {
  // Recherche optimisee O(1) au lieu de O(n)
  if (midiNote < MIDI_NOTE_MIN || midiNote > MIDI_NOTE_MAX) {
    if (DEBUG) {
      Serial.print("[INSTRUMENT] Note MIDI ");
      Serial.print(midiNote);
      Serial.println(" hors plage supportee");
    }
    return -1;
  }

  int8_t servo = ServoMidiMapping[midiNote - MIDI_NOTE_MIN];

  if (servo == -1 && DEBUG) {
    Serial.print("[INSTRUMENT] Note MIDI ");
    Serial.print(midiNote);
    Serial.println(" non jouable (non mappee)");
  }

  return servo;
}

void Instrument::noteOn(uint8_t midiNote, uint8_t velocity) {
	int16_t servo = getServo(midiNote);
	if (servo != -1){
		servoController.pluck(servo);
	}
}

void Instrument::noteOff(uint8_t midiNote) {
  int16_t servo = getServo(midiNote);
	if (servo != -1){
		// Remet le servo a sa position initiale
		servoController.mute(servo);
  }
}
