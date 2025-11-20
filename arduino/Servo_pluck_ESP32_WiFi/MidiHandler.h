#ifndef MIDIHANDLER_H
#define MIDIHANDLER_H

#include <AppleMIDI.h>
#include "instrument.h"
/***********************************************************************************************
----------------------------    MIDI message handler WiFi  -------------------------------------
************************************************************************************************
Recoit les messages MIDI via WiFi (RTP-MIDI/AppleMIDI) et les envoit a l'instrument
via les methodes .noteOn, .noteOff, etc...

L'objectif est d'etre le plus complet au niveau de la selection des messages qui peuvent
etre envoyes via MIDI:
- Messages de Note On
- Messages de Note Off
- Message de Pitch Bend
- Message de Channel Pressure (Aftertouch)
- Message de Polyphonic Key Pressure
- Message de Control Change
- Message de System Common
- Message de System Real-Time
- Message de System Exclusive (SysEx)

Chaque fonction qui peut etre utilisee doit etre decommentee et declaree dans instrument.h
************************************************************************************************/

class MidiHandler {
  private:
    Instrument& _instrument;
    void processMidiEvent(byte type, byte channel, byte note, byte velocity);
    void processControlChange(byte controller, byte value);

    // Callbacks pour AppleMIDI
    static void onNoteOn(byte channel, byte note, byte velocity);
    static void onNoteOff(byte channel, byte note, byte velocity);
    static void onControlChange(byte channel, byte controller, byte value);

    // Callbacks de connexion
    static void onConnected(const APPLEMIDI_NAMESPACE::ssrc_t & ssrc, const char* name);
    static void onDisconnected(const APPLEMIDI_NAMESPACE::ssrc_t & ssrc);

    // Instance statique pour les callbacks
    static MidiHandler* instance;

  public:
    MidiHandler(Instrument &instrument);
    void begin();
    void update();
};

#endif // MIDIHANDLER_H
