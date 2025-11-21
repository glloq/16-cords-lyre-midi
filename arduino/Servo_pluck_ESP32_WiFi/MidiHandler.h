#ifndef MIDIHANDLER_H
#define MIDIHANDLER_H

#include <AppleMIDI.h>
#include "instrument.h"
/***********************************************************************************************
----------------------------    MIDI message handler WiFi  -------------------------------------
************************************************************************************************
Recoit les messages MIDI via WiFi (RTP-MIDI/AppleMIDI) et les envoit a l'instrument
via les methodes .noteOn, .noteOff, etc...

Supporte le protocole MidiMind SysEx pour l'identification de l'instrument:
- Block 1: Identification (nom, notes jouables, polyphonie)
- Block 2: Capacites avancees (CC, aftertouch, pitch bend, etc.)
************************************************************************************************/

// Constantes MidiMind SysEx Protocol
#define MIDIMIND_MANUFACTURER_ID  0x7D  // Educational/Development
#define MIDIMIND_SUB_ID           0x00  // MidiMind
#define MIDIMIND_BLOCK1_ID        0x01  // Block 1: Identification
#define MIDIMIND_BLOCK2_ID        0x02  // Block 2: Capacites
#define MIDIMIND_REQUEST_TYPE     0x00  // Request
#define MIDIMIND_REPLY_TYPE       0x01  // Reply
#define MIDIMIND_VERSION          0x01  // Version 1.0

// Configuration instrument pour MidiMind
#define INSTRUMENT_NAME           "Lyre 16 cordes"
#define INSTRUMENT_GM_PROGRAM     46    // Harp
#define INSTRUMENT_POLYPHONY      16
#define INSTRUMENT_FIRST_NOTE     55    // G3 (pour info, bitmap utilise)
#define INSTRUMENT_NOTE_COUNT     16    // 16 notes

class MidiHandler {
  private:
    Instrument& _instrument;
    void processControlChange(byte controller, byte value);

    // Callbacks pour AppleMIDI
    static void onNoteOn(byte channel, byte note, byte velocity);
    static void onNoteOff(byte channel, byte note, byte velocity);
    static void onControlChange(byte channel, byte controller, byte value);
    static void onSysEx(const byte* data, uint16_t length);

    // Callbacks de connexion
    static void onConnected(const APPLEMIDI_NAMESPACE::ssrc_t & ssrc, const char* name);
    static void onDisconnected(const APPLEMIDI_NAMESPACE::ssrc_t & ssrc);

    // MidiMind SysEx handlers
    static void processSysEx(const byte* data, uint16_t length);
    static void sendBlock1Reply();
    static void sendBlock2Reply();
    static void encode7BitBitmap(const byte* bitmap16, byte* encoded19);
    static void buildNoteBitmap(byte* bitmap16);

    // Instance statique pour les callbacks
    static MidiHandler* instance;

  public:
    MidiHandler(Instrument &instrument);
    void begin();
    void update();
};

#endif // MIDIHANDLER_H
