#include "MidiHandler.h"

// Déclaration externe de l'interface MIDI (définie dans le .ino)
extern BLEMIDI_NAMESPACE::BLEMIDI_Transport<BLEMIDI_NAMESPACE::BLEMIDI_ESP32> MIDI;

MidiHandler::MidiHandler(Instrument &instrument)
  : _instrument(instrument), _midiInterface(nullptr) {

  // Initialiser statistiques
  _stats.validMessages = 0;
  _stats.invalidMessages = 0;
  _stats.outOfRangeNotes = 0;
  _stats.droppedMessages = 0;
  _stats.noteOnCount = 0;
  _stats.noteOffCount = 0;
  _stats.controlChangeCount = 0;
  _stats.errorCount = 0;
  _stats.lastMessageTime = 0;
  _stats.messagesPerSecond = 0;

  // Initialiser rate limiter
  _rateLimiter.noteCount = 0;
  _rateLimiter.windowStart = millis();

  if (DEBUG) {
    Serial.println("[MIDI] Handler BLE Enhanced initialise");
    Serial.printf("[MIDI] Canal: %d | Omni: %s | Feedback: %s\n",
                  MIDI_CHANNEL,
                  MIDI_OMNI_MODE ? "ON" : "OFF",
                  MIDI_SEND_FEEDBACK ? "ON" : "OFF");
  }
}

/***********************************************************************************************
VALIDATION DES MESSAGES
************************************************************************************************/

bool MidiHandler::isValidMidiChannel(byte channel) {
  if (MIDI_OMNI_MODE) return true;
  return (channel == (MIDI_CHANNEL - 1));  // MIDI channels are 0-indexed internally
}

bool MidiHandler::isValidNote(byte note) {
  return (note >= MIDI_NOTE_MIN && note <= MIDI_NOTE_MAX);
}

bool MidiHandler::isValidVelocity(byte velocity) {
  return (velocity >= VELOCITY_MIN && velocity <= VELOCITY_MAX);
}

bool MidiHandler::checkRateLimit() {
  if (!ENABLE_RATE_LIMITING) return true;

  unsigned long now = millis();

  // Nouvelle fenêtre d'une seconde
  if (now - _rateLimiter.windowStart >= 1000) {
    _stats.messagesPerSecond = _rateLimiter.noteCount;
    _rateLimiter.windowStart = now;
    _rateLimiter.noteCount = 0;
  }

  // Vérifier limite
  if (_rateLimiter.noteCount >= MAX_NOTES_PER_SECOND) {
    if (DEBUG) {
      Serial.println("[MIDI] RATE LIMIT dépassé!");
    }
    _stats.droppedMessages++;
    sendMidiError(ERROR_RATE_LIMIT, _rateLimiter.noteCount);
    return false;
  }

  _rateLimiter.noteCount++;
  return true;
}

/***********************************************************************************************
ENVOI DE MESSAGES MIDI (FEEDBACK)
************************************************************************************************/

void MidiHandler::sendMidiFeedback(byte messageType, byte note, byte velocity) {
  if (!MIDI_SEND_FEEDBACK) return;

  // Envoyer via BLE MIDI
  if (messageType == MIDI_NOTE_ON) {
    MIDI.sendNoteOn(note, velocity, MIDI_CHANNEL);
    if (DEBUG) {
      Serial.printf("[MIDI OUT] Note On: %d (vel: %d)\n", note, velocity);
    }
  } else if (messageType == MIDI_NOTE_OFF) {
    MIDI.sendNoteOff(note, 0, MIDI_CHANNEL);
    if (DEBUG) {
      Serial.printf("[MIDI OUT] Note Off: %d\n", note);
    }
  }
}

void MidiHandler::sendMidiError(byte errorCode, byte data) {
  // Envoyer erreur via Control Change
  // CC 127 = Error Type
  // CC 126 = Error Data
  MIDI.sendControlChange(127, errorCode, MIDI_CHANNEL);
  MIDI.sendControlChange(126, data, MIDI_CHANNEL);

  _stats.errorCount++;

  if (DEBUG) {
    const char* errorNames[] = {
      "UNKNOWN",
      "NOTE_NOT_PLAYABLE",
      "SERVO_TIMEOUT",
      "RATE_LIMIT",
      "INVALID_CHANNEL",
      "INVALID_VELOCITY"
    };
    Serial.printf("[MIDI ERR] %s (data: %d)\n",
                  errorCode <= 5 ? errorNames[errorCode] : errorNames[0],
                  data);
  }
}

/***********************************************************************************************
CALLBACKS MIDI
************************************************************************************************/

void MidiHandler::onNoteOn(byte channel, byte note, byte velocity) {
  _stats.lastMessageTime = millis();

  // Vérifier canal
  if (!isValidMidiChannel(channel)) {
    if (DEBUG) Serial.printf("[MIDI] Canal ignoré: %d\n", channel + 1);
    updateStats(false);
    return;
  }

  // Vérifier vélocité (velocity 0 = Note Off)
  if (velocity == 0) {
    onNoteOff(channel, note, 0);
    return;
  }

  if (!isValidVelocity(velocity)) {
    if (DEBUG) Serial.printf("[MIDI] Vélocité invalide: %d\n", velocity);
    sendMidiError(ERROR_INVALID_VELOCITY, velocity);
    updateStats(false);
    return;
  }

  // Vérifier note
  if (!isValidNote(note)) {
    if (DEBUG) Serial.printf("[MIDI] Note hors plage: %d\n", note);
    _stats.outOfRangeNotes++;
    sendMidiError(ERROR_NOTE_NOT_PLAYABLE, note);
    updateStats(false);
    return;
  }

  // Vérifier rate limit
  if (!checkRateLimit()) {
    updateStats(false);
    return;
  }

  // Message valide
  updateStats(true);
  _stats.noteOnCount++;

  // Jouer la note
  _instrument.noteOn(note, velocity);

  // Envoyer feedback si la note a été jouée
  // (vérifié dans Instrument::noteOn)
  sendMidiFeedback(MIDI_NOTE_ON, note, velocity);
}

void MidiHandler::onNoteOff(byte channel, byte note, byte velocity) {
  _stats.lastMessageTime = millis();

  // Vérifier canal
  if (!isValidMidiChannel(channel)) {
    updateStats(false);
    return;
  }

  // Vérifier note
  if (!isValidNote(note)) {
    _stats.outOfRangeNotes++;
    updateStats(false);
    return;
  }

  // Message valide
  updateStats(true);
  _stats.noteOffCount++;

  // Arrêter la note
  _instrument.noteOff(note);

  // Envoyer feedback
  sendMidiFeedback(MIDI_NOTE_OFF, note, 0);
}

void MidiHandler::onControlChange(byte channel, byte controller, byte value) {
  _stats.lastMessageTime = millis();

  if (!isValidMidiChannel(channel)) {
    updateStats(false);
    return;
  }

  updateStats(true);
  _stats.controlChangeCount++;

  processControlChange(controller, value);
}

void MidiHandler::onPitchBend(byte channel, int bend) {
  if (!isValidMidiChannel(channel)) return;

  // Implémentation future
  // _instrument.pitchBend(bend);
}

void MidiHandler::onAfterTouch(byte channel, byte pressure) {
  if (!isValidMidiChannel(channel)) return;

  // Implémentation future
  // _instrument.channelPressure(pressure);
}

void MidiHandler::onPolyPressure(byte channel, byte note, byte pressure) {
  if (!isValidMidiChannel(channel)) return;

  // Implémentation future
  // _instrument.polyKeyPressure(note, pressure);
}

/***********************************************************************************************
CONTROL CHANGE
************************************************************************************************/

void MidiHandler::processControlChange(byte controller, byte value) {
  switch (controller) {
    case 1:   // Modulation Wheel
    case 91:  // Reverb
    case 92:  // Tremolo
    case 94:  // Celeste/Detune
      // Implémentation future
      break;

    case 0x07: // Volume
      // Implémentation future
      break;

    case 120: // All Sound Off
      // Couper tous les sons immédiatement
      if (DEBUG) Serial.println("[MIDI] All Sound Off");
      break;

    case 121: // Reset All Controllers
      if (DEBUG) Serial.println("[MIDI] Reset Controllers");
      break;

    case 123: // All Notes Off
      if (DEBUG) Serial.println("[MIDI] All Notes Off");
      // Arrêter toutes les notes
      for (int i = MIDI_NOTE_MIN; i <= MIDI_NOTE_MAX; i++) {
        _instrument.noteOff(i);
      }
      break;

    default:
      if (DEBUG) {
        Serial.printf("[MIDI] CC non géré: %d = %d\n", controller, value);
      }
      break;
  }
}

/***********************************************************************************************
STATISTIQUES
************************************************************************************************/

void MidiHandler::updateStats(bool valid) {
  if (valid) {
    _stats.validMessages++;
  } else {
    _stats.invalidMessages++;
  }
}

void MidiHandler::printStatistics() {
  Serial.println("\n========== MIDI STATISTICS ==========");
  Serial.printf("Messages valides:    %lu\n", _stats.validMessages);
  Serial.printf("Messages invalides:  %lu\n", _stats.invalidMessages);
  Serial.printf("Notes hors plage:    %lu\n", _stats.outOfRangeNotes);
  Serial.printf("Messages dropped:    %lu\n", _stats.droppedMessages);
  Serial.printf("Erreurs envoyées:    %lu\n", _stats.errorCount);
  Serial.println("-------------------------------------");
  Serial.printf("Note On:             %lu\n", _stats.noteOnCount);
  Serial.printf("Note Off:            %lu\n", _stats.noteOffCount);
  Serial.printf("Control Change:      %lu\n", _stats.controlChangeCount);
  Serial.println("-------------------------------------");
  Serial.printf("Messages/seconde:    %lu\n", _stats.messagesPerSecond);
  Serial.printf("Dernier message:     %lu ms\n",
                millis() - _stats.lastMessageTime);
  Serial.println("=====================================\n");
}

void MidiHandler::resetStatistics() {
  _stats.validMessages = 0;
  _stats.invalidMessages = 0;
  _stats.outOfRangeNotes = 0;
  _stats.droppedMessages = 0;
  _stats.noteOnCount = 0;
  _stats.noteOffCount = 0;
  _stats.controlChangeCount = 0;
  _stats.errorCount = 0;
  _stats.messagesPerSecond = 0;

  Serial.println("[MIDI] Statistiques réinitialisées");
}
