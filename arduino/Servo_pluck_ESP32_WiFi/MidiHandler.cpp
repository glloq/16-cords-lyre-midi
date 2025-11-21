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
  AppleMIDI.setHandleSysEx(onSysEx);

  // Callbacks de connexion (optionnels mais utiles pour le debug)
  AppleMIDI.setHandleConnected(onConnected);
  AppleMIDI.setHandleDisconnected(onDisconnected);

  // Demarrer AppleMIDI
  AppleMIDI.begin(APPLEMIDI_SESSION_NAME);

  if (DEBUG) {
    Serial.println("[MIDI] AppleMIDI demarre");
    Serial.println("[MIDI] MidiMind SysEx Protocol actif");
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

void MidiHandler::onSysEx(const byte* data, uint16_t length) {
  if (DEBUG) {
    Serial.print("[SYSEX] Recu ");
    Serial.print(length);
    Serial.print(" bytes: ");
    for (uint16_t i = 0; i < length && i < 20; i++) {
      Serial.print(data[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
  }
  processSysEx(data, length);
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

/*------------------------------------------------------------------
--------------        MidiMind SysEx Protocol            ----------
------------------------------------------------------------------*/

void MidiHandler::processSysEx(const byte* data, uint16_t length) {
  // Verifier la taille minimale: F0 7D 00 XX 00 F7 = 6 bytes
  if (length < 6) {
    if (DEBUG) Serial.println("[SYSEX] Message trop court");
    return;
  }

  // Verifier le header MidiMind (sans F0 qui peut etre exclu par certaines libs)
  uint16_t offset = 0;
  if (data[0] == 0xF0) offset = 1;

  // Verifier Manufacturer ID et Sub ID
  if (data[offset] != MIDIMIND_MANUFACTURER_ID || data[offset + 1] != MIDIMIND_SUB_ID) {
    if (DEBUG) {
      Serial.print("[SYSEX] Pas un message MidiMind: ");
      Serial.print(data[offset], HEX);
      Serial.print(" ");
      Serial.println(data[offset + 1], HEX);
    }
    return;
  }

  byte blockId = data[offset + 2];
  byte msgType = data[offset + 3];

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
Structure: F0 7D 00 01 01 <Version> <Name[16]> <GM> <FirstNote>
           <Count> <Poly> <Flags> [NoteBitmap] F7
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

  // Envoyer via AppleMIDI
  AppleMIDI.sendSysEx(reply, idx);

  if (DEBUG) {
    Serial.print("[SYSEX] Block 1 Reply envoye (");
    Serial.print(idx);
    Serial.println(" bytes)");
  }
}

/*------------------------------------------------------------------
--------------        Block 2 Reply (Capacites)          ----------
Structure: F0 7D 00 02 01 <Version> <CapFlags[2]> [CCBitmap] F7
La lyre n'a pas de CC, donc message minimal (9 bytes)
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
  // Flags = 0x0000
  reply[idx++] = 0x00;  // Bits 0-6
  reply[idx++] = 0x00;  // Bits 7-13

  // Pas de CC Bitmap (CC_SUPPORT = 0)

  // End
  reply[idx++] = 0xF7;

  // Envoyer via AppleMIDI
  AppleMIDI.sendSysEx(reply, idx);

  if (DEBUG) {
    Serial.print("[SYSEX] Block 2 Reply envoye (");
    Serial.print(idx);
    Serial.println(" bytes)");
  }
}

/*------------------------------------------------------------------
--------------        Build Note Bitmap                  ----------
Construit le bitmap 128 bits des notes jouables
Notes de la lyre: 55, 57, 59, 60, 62, 64, 65, 67, 69, 71, 72, 74, 76, 77, 79, 81
------------------------------------------------------------------*/
void MidiHandler::buildNoteBitmap(byte* bitmap16) {
  // Initialiser a zero
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
Encode 16 bytes natifs en 19 bytes SysEx-safe (7-bit)
Les MSB (bit 7) de chaque byte sont packes dans les 3 derniers bytes
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
