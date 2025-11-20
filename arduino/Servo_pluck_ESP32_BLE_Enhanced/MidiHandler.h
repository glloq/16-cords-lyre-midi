#ifndef MIDIHANDLER_H
#define MIDIHANDLER_H

#include "instrument.h"
#include "settings.h"
#include <BLEMIDI_Transport.h>

/***********************************************************************************************
----------------------------    MIDI Handler BLE Enhanced   ------------------------------------
************************************************************************************************
Version améliorée avec:
- Validation des messages MIDI
- Filtrage par canal
- Protection anti-spam
- Statistiques en temps réel
- Feedback MIDI (envoi de confirmations)
- Gestion d'erreurs
************************************************************************************************/

// Structure pour statistiques MIDI
struct MidiStatistics {
  uint32_t validMessages;
  uint32_t invalidMessages;
  uint32_t outOfRangeNotes;
  uint32_t droppedMessages;
  uint32_t noteOnCount;
  uint32_t noteOffCount;
  uint32_t controlChangeCount;
  uint32_t errorCount;
  unsigned long lastMessageTime;
  uint32_t messagesPerSecond;
};

// Structure pour rate limiting
struct RateLimiter {
  uint32_t noteCount;
  unsigned long windowStart;
};

class MidiHandler {
  private:
    Instrument& _instrument;
    MidiStatistics _stats;
    RateLimiter _rateLimiter;

    // Méthodes privées
    void processControlChange(byte controller, byte value);
    bool isValidMidiChannel(byte channel);
    bool isValidNote(byte note);
    bool isValidVelocity(byte velocity);
    bool checkRateLimit();
    void sendMidiFeedback(byte messageType, byte note, byte velocity);
    void sendMidiError(byte errorCode, byte data);
    void updateStats(bool valid);

  public:
    MidiHandler(Instrument &instrument);

    // Callbacks BLE MIDI
    void onNoteOn(byte channel, byte note, byte velocity);
    void onNoteOff(byte channel, byte note, byte velocity);
    void onControlChange(byte channel, byte controller, byte value);
    void onPitchBend(byte channel, int bend);
    void onAfterTouch(byte channel, byte pressure);
    void onPolyPressure(byte channel, byte note, byte pressure);

    // Gestion statistiques
    void printStatistics();
    void resetStatistics();
    MidiStatistics getStatistics() { return _stats; }

    // Permet l'accès au MIDI pour envoyer des messages
    void setMidiInterface(void* midiPtr);  // Pointeur vers l'interface MIDI

  private:
    void* _midiInterface;  // Pointeur vers BLEMIDI
};

#endif // MIDIHANDLER_H
