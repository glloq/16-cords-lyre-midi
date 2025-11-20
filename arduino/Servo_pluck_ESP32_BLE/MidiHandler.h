#ifndef MIDIHANDLER_H
#define MIDIHANDLER_H

#include "instrument.h"
/***********************************************************************************************
----------------------------    MIDI message handler BLE   -------------------------------------
************************************************************************************************
Version BLE MIDI pour ESP32

Recoit les messages MIDI via Bluetooth Low Energy et les envoie a l'instrument
via les methodes .noteOn, .noteOff, etc...

L'objectif est d'etre le plus complet au niveau de la selection des messages MIDI:
- Messages de Note On
- Messages de Note Off
- Message de Pitch Bend : Utilise pour moduler la hauteur tonale d'une note en temps reel
- Message de Channel Pressure (Aftertouch) : Indique la pression appliquee a une seule note
- Message de Polyphonic Key Pressure : Pression individuelle sur chaque note
- Message de Control Change : Changements de parametres MIDI
- Message de System Common : Messages qui affectent l'ensemble du systeme MIDI
- Message de System Real-Time : Messages de synchronisation en temps reel
- Message de System Exclusive (SysEx) : Donnees specifiques au fabricant

Chaque fonction qui peut etre utilisee doit etre decommentee et declaree dans instrument.h
************************************************************************************************/

class MidiHandler {
  private:
    Instrument& _instrument;
    void processControlChange(byte controller, byte value);
  public:
    MidiHandler(Instrument &instrument);

    // Callbacks pour BLE MIDI - doivent etre publics pour etre appeles par la bibliotheque BLE
    void onNoteOn(byte channel, byte note, byte velocity);
    void onNoteOff(byte channel, byte note, byte velocity);
    void onControlChange(byte channel, byte controller, byte value);
    void onPitchBend(byte channel, int bend);
    void onAfterTouch(byte channel, byte pressure);
    void onPolyPressure(byte channel, byte note, byte pressure);
};

#endif // MIDIHANDLER_H
