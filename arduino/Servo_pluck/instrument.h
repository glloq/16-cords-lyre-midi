#ifndef INSTRUMENT_H
#define INSTRUMENT_H

#include "settings.h"
#include "ServoController.h"
/***********************************************************************************************
----------------------------    instrument.h   ----------------------------------------
************************************************************************************************

execute les messages noteOn et noteOff

************************************************************************************************/
class Instrument {
private:
  ServoController servoController;
	int getServo(uint8_t midiNote); //renvoit le numero du servo de 1 a 16 et 0 si la note ne peut pas etre jouée
	
public:
	Instrument();
	void noteOn(uint8_t midiNote, uint8_t velocity);
	void noteOff(uint8_t midiNote);
  void update();
};

#endif // INSTRUMENT_H
