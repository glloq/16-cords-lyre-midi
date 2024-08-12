#include "Instrument.h"

Instrument::Instrument() : servoController() {
  if (DEBUG) {
    Serial.println("DEBUG : Instrument--creation");
  } 
  //servoController= new ServoController(); //initialise toutles servomoteurs et le tableau
}

int Instrument::getServo(uint8_t midiNote) {
	for (int i=0; i<NUM_SERVOS;i++){
		if (MidiServoMapping[i]==midiNote){
			return i;
		}
	}
  if (DEBUG) {
    Serial.println("DEBUG : instrument : midi note not playable");
  } 
	return -1;
}

void Instrument::noteOn(uint8_t midiNote, uint8_t velocity) {
	int servo=getServo(midiNote);
	if (servo!=-1){
		servoController.pluck(servo);
	}
}

void Instrument::noteOff(uint8_t midiNote) {
  int servo=getServo(midiNote);
	if (servo!=-1){
		// Remet le servo à sa position initiale
		servoController.mute(servo);	    
  }
}

void Instrument::update(){


}