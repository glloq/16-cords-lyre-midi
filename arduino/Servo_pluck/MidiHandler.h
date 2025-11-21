#ifndef MIDIHANDLER_H
#define MIDIHANDLER_H

#include <MIDIUSB.h>
#include "instrument.h"
/***********************************************************************************************
----------------------------    MIDI message handler    ----------------------------------------
************************************************************************************************
Recoit les messages midi et les envoit a l'instrument via les methodes .noteOn, .noteOff, ect ...

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

// Buffer SysEx
#define SYSEX_BUFFER_SIZE         64

class MidiHandler {
  private:
    Instrument& _instrument;

    // Buffer pour accumulation SysEx
    byte sysexBuffer[SYSEX_BUFFER_SIZE];
    uint8_t sysexIndex;
    bool sysexActive;

    void processMidiEvent(midiEventPacket_t midiEvent);
    void processControlChange(byte controller, byte value);

    // MidiMind SysEx handlers
    void processSysExPacket(midiEventPacket_t packet);
    void processSysExMessage();
    void sendBlock1Reply();
    void sendBlock2Reply();
    void sendSysEx(const byte* data, uint8_t length);
    void encode7BitBitmap(const byte* bitmap16, byte* encoded19);
    void buildNoteBitmap(byte* bitmap16);

  public:
    MidiHandler(Instrument &instrument);
    void readMidi();
};

#endif // MIDIHANDLER_H
