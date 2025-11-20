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
	int16_t getServo(uint8_t midiNote); //renvoit le numero du servo de 0 a 15 et -1 si la note ne peut pas etre jouee
	
public:
	Instrument();
	void update();  // A appeler dans loop() pour gerer les taches non-bloquantes
	bool isReady();  // Retourne true quand l'instrument est pret a jouer
	void noteOn(uint8_t midiNote, uint8_t velocity);
	void noteOff(uint8_t midiNote);
};

#endif // INSTRUMENT_H
