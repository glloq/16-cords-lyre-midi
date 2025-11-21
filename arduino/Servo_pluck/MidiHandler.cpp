#include "MidiHandler.h"

MidiHandler::MidiHandler(Instrument &instrument) : _instrument(instrument) {
  sysexIndex = 0;
  sysexActive = false;
  if (DEBUG) {
    Serial.println("[MIDI] Handler initialise");
    Serial.println("[MIDI] MidiMind SysEx Protocol actif");
  }
}

void MidiHandler::readMidi() {
  midiEventPacket_t midiEvent;
  do {
    midiEvent = MidiUSB.read();
    if (midiEvent.header != 0) {
      processMidiEvent(midiEvent);
    }
  } while (midiEvent.header != 0);
}

void MidiHandler::processMidiEvent(midiEventPacket_t midiEvent) {
  // Gerer les paquets SysEx (headers 0x04, 0x05, 0x06, 0x07)
  if (midiEvent.header >= 0x04 && midiEvent.header <= 0x07) {
    processSysExPacket(midiEvent);
    return;
  }

  byte messageType = midiEvent.byte1 & 0xF0;
  byte channel = midiEvent.byte1 & 0x0F;
  byte note = midiEvent.byte2;
  byte velocity = midiEvent.byte3;

  switch (messageType) {
    case MIDI_NOTE_ON: // Note On
      if (velocity > 0) {
        _instrument.noteOn(note, velocity);
      } else {
        // Note Off (velocity 0 = Note Off)
        _instrument.noteOff(note);
      }
      break;
    case MIDI_NOTE_OFF: // Note Off
      _instrument.noteOff(note);
      break;
    case MIDI_PITCH_BEND: // Pitch Bend
      // int pitchBendValue = (midiEvent.byte3 << 7) | midiEvent.byte2;
      // _instrument.pitchBend(pitchBendValue);
      break;
    case MIDI_CHANNEL_PRESSURE: // Channel Pressure (Aftertouch)
      // int channelPressureValue = midiEvent.byte2;
      // _instrument.channelPressure(channelPressureValue);
      break;
    case MIDI_POLY_KEY_PRESSURE: // Polyphonic Key Pressure
      // int polyKeyPressureValue = midiEvent.byte3;
      // _instrument.polyKeyPressure(polyKeyPressureValue);
      break;
    case MIDI_CONTROL_CHANGE: // Control Change
      processControlChange(note, velocity);
      break;
    case MIDI_SYSTEM_COMMON: // System Common or System Real-Time
      break;
    case MIDI_SYSTEM_EXCLUSIVE_END: // End of System Exclusive
      break;
  }
}

/*------------------------------------------------------------------
--------------        process Control Change             ----------
------------------------------------------------------------------*/
void MidiHandler::processControlChange(byte controller, byte value) {
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
  }
}

/*------------------------------------------------------------------
--------------        SysEx Packet Processing            ----------
MIDIUSB envoie les SysEx en paquets de 4 bytes:
- Header 0x04: SysEx start ou continue (3 bytes data)
- Header 0x05: Single byte system common (1 byte)
- Header 0x06: SysEx end (2 bytes data)
- Header 0x07: SysEx end (3 bytes data)
------------------------------------------------------------------*/
void MidiHandler::processSysExPacket(midiEventPacket_t packet) {
  switch (packet.header) {
    case 0x04: // SysEx start or continue
      if (packet.byte1 == 0xF0) {
        // Debut d'un nouveau message SysEx
        sysexIndex = 0;
        sysexActive = true;
      }
      if (sysexActive && sysexIndex < SYSEX_BUFFER_SIZE - 3) {
        sysexBuffer[sysexIndex++] = packet.byte1;
        sysexBuffer[sysexIndex++] = packet.byte2;
        sysexBuffer[sysexIndex++] = packet.byte3;
      }
      break;

    case 0x05: // Single byte (F7 seul)
      if (sysexActive && packet.byte1 == 0xF7) {
        sysexBuffer[sysexIndex++] = 0xF7;
        processSysExMessage();
        sysexActive = false;
      }
      break;

    case 0x06: // SysEx end with 2 bytes
      if (sysexActive && sysexIndex < SYSEX_BUFFER_SIZE - 2) {
        sysexBuffer[sysexIndex++] = packet.byte1;
        sysexBuffer[sysexIndex++] = packet.byte2;
        processSysExMessage();
        sysexActive = false;
      }
      break;

    case 0x07: // SysEx end with 3 bytes
      if (sysexActive && sysexIndex < SYSEX_BUFFER_SIZE - 3) {
        sysexBuffer[sysexIndex++] = packet.byte1;
        sysexBuffer[sysexIndex++] = packet.byte2;
        sysexBuffer[sysexIndex++] = packet.byte3;
        processSysExMessage();
        sysexActive = false;
      }
      break;
  }
}

/*------------------------------------------------------------------
--------------        MidiMind SysEx Protocol            ----------
------------------------------------------------------------------*/
void MidiHandler::processSysExMessage() {
  if (DEBUG) {
    Serial.print("[SYSEX] Recu ");
    Serial.print(sysexIndex);
    Serial.print(" bytes: ");
    for (uint8_t i = 0; i < sysexIndex && i < 20; i++) {
      Serial.print(sysexBuffer[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
  }

  // Verifier la taille minimale: F0 7D 00 XX 00 F7 = 6 bytes
  if (sysexIndex < 6) {
    if (DEBUG) Serial.println("[SYSEX] Message trop court");
    return;
  }

  // Verifier le header MidiMind
  uint8_t offset = 0;
  if (sysexBuffer[0] == 0xF0) offset = 1;

  // Verifier Manufacturer ID et Sub ID
  if (sysexBuffer[offset] != MIDIMIND_MANUFACTURER_ID ||
      sysexBuffer[offset + 1] != MIDIMIND_SUB_ID) {
    if (DEBUG) {
      Serial.print("[SYSEX] Pas un message MidiMind: ");
      Serial.print(sysexBuffer[offset], HEX);
      Serial.print(" ");
      Serial.println(sysexBuffer[offset + 1], HEX);
    }
    return;
  }

  byte blockId = sysexBuffer[offset + 2];
  byte msgType = sysexBuffer[offset + 3];

  // Traiter uniquement les requests
  if (msgType != MIDIMIND_REQUEST_TYPE) {
    if (DEBUG) Serial.println("[SYSEX] Pas une requete, ignore");
    return;
  }

  switch (blockId) {
    case MIDIMIND_BLOCK1_ID:
      if (DEBUG) Serial.println("[SYSEX] Block 1 Request recu");
      sendBlock1Reply();
      break;

    case MIDIMIND_BLOCK2_ID:
      if (DEBUG) Serial.println("[SYSEX] Block 2 Request recu");
      sendBlock2Reply();
      break;

    default:
      if (DEBUG) {
        Serial.print("[SYSEX] Block ID inconnu: ");
        Serial.println(blockId, HEX);
      }
      break;
  }
}

/*------------------------------------------------------------------
--------------        Block 1 Reply (Identification)     ----------
------------------------------------------------------------------*/
void MidiHandler::sendBlock1Reply() {
  // Taille: 47 bytes (avec bitmap car notes non consecutives)
  byte reply[47];
  uint8_t idx = 0;

  // Header
  reply[idx++] = 0xF0;                      // SysEx Start
  reply[idx++] = MIDIMIND_MANUFACTURER_ID;  // 0x7D
  reply[idx++] = MIDIMIND_SUB_ID;           // 0x00
  reply[idx++] = MIDIMIND_BLOCK1_ID;        // 0x01
  reply[idx++] = MIDIMIND_REPLY_TYPE;       // 0x01

  // Version
  reply[idx++] = MIDIMIND_VERSION;          // 0x01

  // Name (16 bytes, null-padded)
  const char* name = INSTRUMENT_NAME;
  for (uint8_t i = 0; i < 16; i++) {
    if (i < strlen(name)) {
      reply[idx++] = name[i];
    } else {
      reply[idx++] = 0x00;
    }
  }

  // GM Program
  reply[idx++] = INSTRUMENT_GM_PROGRAM;     // 46 (Harp)

  // First Note (pour info, bitmap prime)
  reply[idx++] = INSTRUMENT_FIRST_NOTE;     // 55 (G3)

  // Note Count
  reply[idx++] = INSTRUMENT_NOTE_COUNT;     // 16

  // Polyphony
  reply[idx++] = INSTRUMENT_POLYPHONY;      // 16

  // Flags: bit 0 = 0 (bitmap suit, notes non consecutives)
  reply[idx++] = 0x00;

  // Note Bitmap (19 bytes encodes en 7-bit)
  byte bitmap16[16] = {0};
  buildNoteBitmap(bitmap16);
  byte encoded19[19];
  encode7BitBitmap(bitmap16, encoded19);
  for (uint8_t i = 0; i < 19; i++) {
    reply[idx++] = encoded19[i];
  }

  // End
  reply[idx++] = 0xF7;

  // Envoyer via MIDIUSB
  sendSysEx(reply, idx);

  if (DEBUG) {
    Serial.print("[SYSEX] Block 1 Reply envoye (");
    Serial.print(idx);
    Serial.println(" bytes)");
  }
}

/*------------------------------------------------------------------
--------------        Block 2 Reply (Capacites)          ----------
------------------------------------------------------------------*/
void MidiHandler::sendBlock2Reply() {
  // Taille: 9 bytes (pas de CC support)
  byte reply[9];
  uint8_t idx = 0;

  // Header
  reply[idx++] = 0xF0;                      // SysEx Start
  reply[idx++] = MIDIMIND_MANUFACTURER_ID;  // 0x7D
  reply[idx++] = MIDIMIND_SUB_ID;           // 0x00
  reply[idx++] = MIDIMIND_BLOCK2_ID;        // 0x02
  reply[idx++] = MIDIMIND_REPLY_TYPE;       // 0x01

  // Version
  reply[idx++] = MIDIMIND_VERSION;          // 0x01

  // Capability Flags (16 bits encodes en 2 bytes 7-bit)
  // Pas de capacites avancees pour la lyre
  reply[idx++] = 0x00;  // Bits 0-6
  reply[idx++] = 0x00;  // Bits 7-13

  // End
  reply[idx++] = 0xF7;

  // Envoyer via MIDIUSB
  sendSysEx(reply, idx);

  if (DEBUG) {
    Serial.print("[SYSEX] Block 2 Reply envoye (");
    Serial.print(idx);
    Serial.println(" bytes)");
  }
}

/*------------------------------------------------------------------
--------------        Send SysEx via MIDIUSB             ----------
------------------------------------------------------------------*/
void MidiHandler::sendSysEx(const byte* data, uint8_t length) {
  uint8_t i = 0;

  while (i < length) {
    midiEventPacket_t packet;
    uint8_t remaining = length - i;

    if (i == 0 && remaining >= 3) {
      // Premier paquet (SysEx start)
      packet = {0x04, data[i], data[i+1], data[i+2]};
      i += 3;
    } else if (remaining >= 3 && data[i+2] != 0xF7) {
      // Paquet de continuation
      packet = {0x04, data[i], data[i+1], data[i+2]};
      i += 3;
    } else if (remaining == 3) {
      // Fin avec 3 bytes (incluant F7)
      packet = {0x07, data[i], data[i+1], data[i+2]};
      i += 3;
    } else if (remaining == 2) {
      // Fin avec 2 bytes
      packet = {0x06, data[i], data[i+1], 0};
      i += 2;
    } else if (remaining == 1) {
      // Fin avec 1 byte (F7 seul)
      packet = {0x05, data[i], 0, 0};
      i += 1;
    } else {
      break;
    }

    MidiUSB.sendMIDI(packet);
  }

  MidiUSB.flush();
}

/*------------------------------------------------------------------
--------------        Build Note Bitmap                  ----------
------------------------------------------------------------------*/
void MidiHandler::buildNoteBitmap(byte* bitmap16) {
  memset(bitmap16, 0, 16);

  // Notes jouables (depuis settings.h MidiServoMapping)
  const byte playableNotes[] = {55, 57, 59, 60, 62, 64, 65, 67, 69, 71, 72, 74, 76, 77, 79, 81};

  for (uint8_t i = 0; i < sizeof(playableNotes); i++) {
    byte note = playableNotes[i];
    byte byteIndex = note / 8;
    byte bitIndex = note % 8;
    bitmap16[byteIndex] |= (1 << bitIndex);
  }

  if (DEBUG) {
    Serial.print("[SYSEX] Note bitmap: ");
    for (uint8_t i = 0; i < 16; i++) {
      Serial.print(bitmap16[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
  }
}

/*------------------------------------------------------------------
--------------        Encode 7-bit Bitmap                ----------
------------------------------------------------------------------*/
void MidiHandler::encode7BitBitmap(const byte* bitmap16, byte* encoded19) {
  // Copier les 7 bits de poids faible de chaque byte
  for (uint8_t i = 0; i < 16; i++) {
    encoded19[i] = bitmap16[i] & 0x7F;
  }

  // Packer les MSB (bit 7) dans les bytes 16, 17, 18
  encoded19[16] = 0;
  encoded19[17] = 0;
  encoded19[18] = 0;

  for (uint8_t i = 0; i < 16; i++) {
    if (bitmap16[i] & 0x80) {
      if (i < 7) {
        encoded19[16] |= (1 << i);
      } else if (i < 14) {
        encoded19[17] |= (1 << (i - 7));
      } else {
        encoded19[18] |= (1 << (i - 14));
      }
    }
  }
}
