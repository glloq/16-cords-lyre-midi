#include "MidiHandler.h"
#include "settings.h"

MidiHandler::MidiHandler(Instrument &instrument) : _instrument(instrument) {
  if (DEBUG) {
    Serial.println("[MIDI] Handler BLE initialise");
  }
}

// Callback pour Note On
void MidiHandler::onNoteOn(byte channel, byte note, byte velocity) {
  if (velocity > 0) {
    _instrument.noteOn(note, velocity);
  } else {
    // Note Off (velocity 0 = Note Off)
    _instrument.noteOff(note);
  }
}

// Callback pour Note Off
void MidiHandler::onNoteOff(byte channel, byte note, byte velocity) {
  _instrument.noteOff(note);
}

// Callback pour Control Change
void MidiHandler::onControlChange(byte channel, byte controller, byte value) {
  processControlChange(controller, value);
}

// Callback pour Pitch Bend
void MidiHandler::onPitchBend(byte channel, int bend) {
  // int pitchBendValue = bend;
  // _instrument.pitchBend(pitchBendValue);
}

// Callback pour Aftertouch (Channel Pressure)
void MidiHandler::onAfterTouch(byte channel, byte pressure) {
  // _instrument.channelPressure(pressure);
}

// Callback pour Polyphonic Pressure
void MidiHandler::onPolyPressure(byte channel, byte note, byte pressure) {
  // _instrument.polyKeyPressure(note, pressure);
}

/*------------------------------------------------------------------
--------------        process Control Change             ----------
------------------------------------------------------------------*/
void MidiHandler::processControlChange(byte controller, byte value) {
  // Logique pour gerer les messages Control Change
  switch (controller) {
    case 1:
    case 91:
    case 92:
    case 94: // Gestion du vibrato/modulation
      //_instrument.modulationWheel(value);
      break;
    case 0x07: // Volume
      //_instrument.volumeControl(value);
      break;
    case 121: // Reinitialisation de tous les controleurs
      //_instrument.reset();
      break;
    case 123: // Desactiver toutes les notes
      //_instrument.mute();
      break;
    // Ajouter d'autres cas si necessaire
  }
}
