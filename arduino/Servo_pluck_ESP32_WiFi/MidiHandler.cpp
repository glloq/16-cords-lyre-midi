#include "MidiHandler.h"

// Initialisation de la variable statique
MidiHandler* MidiHandler::instance = nullptr;

// Creation de l'instance AppleMIDI
APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE();

MidiHandler::MidiHandler(Instrument &instrument) : _instrument(instrument) {
  instance = this;  // Stocker l'instance pour les callbacks statiques
  if (DEBUG) {
    Serial.println("[MIDI] Handler WiFi initialise");
  }
}

void MidiHandler::begin() {
  // Enregistrer les callbacks
  AppleMIDI.setHandleNoteOn(onNoteOn);
  AppleMIDI.setHandleNoteOff(onNoteOff);
  AppleMIDI.setHandleControlChange(onControlChange);

  // Callbacks de connexion (optionnels mais utiles pour le debug)
  AppleMIDI.setHandleConnected(onConnected);
  AppleMIDI.setHandleDisconnected(onDisconnected);

  // Demarrer AppleMIDI
  AppleMIDI.begin(APPLEMIDI_SESSION_NAME);

  if (DEBUG) {
    Serial.println("[MIDI] AppleMIDI demarre");
    Serial.println("[MIDI] En attente de connexion...");
  }
}

void MidiHandler::update() {
  // Lire les messages MIDI entrants
  AppleMIDI.run();
}

// Callbacks statiques
void MidiHandler::onNoteOn(byte channel, byte note, byte velocity) {
  if (instance && DEBUG) {
    Serial.print("[MIDI] Note ON - Canal: ");
    Serial.print(channel);
    Serial.print(" Note: ");
    Serial.print(note);
    Serial.print(" Velocite: ");
    Serial.println(velocity);
  }

  if (instance) {
    if (velocity > 0) {
      instance->_instrument.noteOn(note, velocity);
    } else {
      // Note Off (velocity 0 = Note Off)
      instance->_instrument.noteOff(note);
    }
  }
}

void MidiHandler::onNoteOff(byte channel, byte note, byte velocity) {
  if (instance && DEBUG) {
    Serial.print("[MIDI] Note OFF - Canal: ");
    Serial.print(channel);
    Serial.print(" Note: ");
    Serial.println(note);
  }

  if (instance) {
    instance->_instrument.noteOff(note);
  }
}

void MidiHandler::onControlChange(byte channel, byte controller, byte value) {
  if (instance && DEBUG) {
    Serial.print("[MIDI] Control Change - Canal: ");
    Serial.print(channel);
    Serial.print(" Controleur: ");
    Serial.print(controller);
    Serial.print(" Valeur: ");
    Serial.println(value);
  }

  if (instance) {
    instance->processControlChange(controller, value);
  }
}

void MidiHandler::onConnected(const APPLEMIDI_NAMESPACE::ssrc_t & ssrc, const char* name) {
  if (DEBUG) {
    Serial.print("[MIDI] Connecte a session: ");
    Serial.println(name);
  }
}

void MidiHandler::onDisconnected(const APPLEMIDI_NAMESPACE::ssrc_t & ssrc) {
  if (DEBUG) {
    Serial.println("[MIDI] Deconnecte");
  }
}

/*------------------------------------------------------------------
--------------        process Control Change             ----------
------------------------------------------------------------------*/
void MidiHandler::processControlChange(byte controller, byte value) {
  // Logique pour traiter les messages Control Change
  switch (controller) {
    case 1:
    case 91:
    case 92:
    case 94: // Gestion du vibrato
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
    // Ajouter d'autres cas selon les besoins
  }
}
