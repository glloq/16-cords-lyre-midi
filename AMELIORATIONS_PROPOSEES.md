# Améliorations proposées - Lyre MIDI ESP32 BLE

## 📋 Résumé de l'analyse

Code analysé : Version ESP32 BLE MIDI (`arduino/Servo_pluck_ESP32_BLE/`)

---

## 🔐 1. CONTRÔLE D'APPAIRAGE BLE

### Problèmes actuels
- ❌ N'importe qui peut se connecter (pas de sécurité)
- ❌ Pas d'indication visuelle de l'état de connexion
- ❌ Pas de contrôle manuel de l'appairage
- ❌ Connexion permanente active (consommation)
- ❌ Pas de gestion des connexions multiples

### Solutions proposées

#### A. Mode appairage avec bouton physique
```cpp
// Ajouter dans settings.h
#define PIN_PAIRING_BUTTON 0  // GPIO 0 (bouton BOOT sur ESP32)
#define PAIRING_TIMEOUT_MS 300000  // 5 minutes

// États d'appairage
enum PairingMode {
  PAIRING_DISABLED,    // Appairage désactivé
  PAIRING_ENABLED,     // Appairage actif
  PAIRING_CONNECTED    // Connecté
};
```

**Fonctionnement :**
- Appui court (< 1s) : Active appairage pour 5 minutes
- Appui long (> 3s) : Efface liste des appareils autorisés
- Timeout automatique après 5 minutes sans connexion

#### B. LED d'indication BLE
```cpp
#define PIN_BLE_LED 2  // GPIO 2 (LED intégrée ESP32)

// Patterns LED
// Éteinte : BLE désactivé
// Clignotement lent (1Hz) : Recherche de connexion
// Clignotement rapide (5Hz) : Appairage actif
// Fixe : Connecté
```

#### C. Sécurité de connexion
```cpp
// Liste blanche d'adresses MAC autorisées (stockée en NVS)
#define MAX_PAIRED_DEVICES 5

struct PairedDevice {
  uint8_t macAddress[6];
  char deviceName[32];
  uint32_t lastConnected;
};

// Authentification par code PIN (optionnel)
#define BLE_USE_PIN_AUTH true
#define BLE_PIN_CODE "1234"
```

#### D. Gestion de connexion améliorée
```cpp
// Limite de temps de connexion (économie batterie)
#define MAX_CONNECTION_TIME_MS 7200000  // 2 heures max

// Auto-sleep après déconnexion
#define AUTO_SLEEP_TIMEOUT_MS 60000  // 1 minute

// Statistiques de connexion
struct ConnectionStats {
  uint32_t totalConnections;
  uint32_t totalMessages;
  uint32_t connectionDuration;
  uint32_t lastDisconnectReason;
};
```

---

## 📥 2. RÉCEPTION DE MESSAGES MIDI

### Problèmes actuels
- ❌ Accepte tous les canaux MIDI (1-16)
- ❌ Pas de validation des données
- ❌ Pas de protection contre spam/surcharge
- ❌ Pas de statistiques
- ❌ Pas de gestion des erreurs

### Solutions proposées

#### A. Filtrage par canal MIDI
```cpp
// Ajouter dans settings.h
#define MIDI_CHANNEL 1  // Canal MIDI principal (1-16)
#define MIDI_OMNI_MODE false  // true = écoute tous canaux

// Fonction de validation
bool isValidMidiChannel(byte channel) {
  if (MIDI_OMNI_MODE) return true;
  return (channel == MIDI_CHANNEL - 1);  // MIDI channels are 0-indexed
}
```

#### B. Validation des messages
```cpp
// Validation de vélocité
#define VELOCITY_MIN 1    // Ignorer vélocité 0 (Note Off)
#define VELOCITY_MAX 127

// Validation de notes
bool isValidNote(byte note) {
  return (note >= MIDI_NOTE_MIN && note <= MIDI_NOTE_MAX);
}

// Callback amélioré
void onNoteOn(byte channel, byte note, byte velocity) {
  // Vérifier canal
  if (!isValidMidiChannel(channel)) {
    if (DEBUG) Serial.println("[MIDI] Canal ignoré");
    return;
  }

  // Vérifier vélocité
  if (velocity < VELOCITY_MIN || velocity > VELOCITY_MAX) {
    if (DEBUG) Serial.println("[MIDI] Vélocité invalide");
    midiStats.invalidMessages++;
    return;
  }

  // Vérifier note
  if (!isValidNote(note)) {
    if (DEBUG) Serial.println("[MIDI] Note hors plage");
    midiStats.outOfRangeNotes++;
    // Envoyer feedback MIDI d'erreur (voir section 3)
    return;
  }

  midiStats.validMessages++;
  midiHandler->onNoteOn(channel, note, velocity);
}
```

#### C. Protection anti-spam
```cpp
// Limite de notes par seconde (éviter surcharge)
#define MAX_NOTES_PER_SECOND 50

struct RateLimiter {
  uint32_t noteCount;
  unsigned long windowStart;
};

bool checkRateLimit() {
  unsigned long now = millis();

  // Nouvelle fenêtre d'une seconde
  if (now - rateLimiter.windowStart >= 1000) {
    rateLimiter.windowStart = now;
    rateLimiter.noteCount = 0;
  }

  // Vérifier limite
  if (rateLimiter.noteCount >= MAX_NOTES_PER_SECOND) {
    if (DEBUG) Serial.println("[MIDI] Rate limit dépassé!");
    midiStats.droppedMessages++;
    return false;
  }

  rateLimiter.noteCount++;
  return true;
}
```

#### D. Statistiques MIDI
```cpp
struct MidiStatistics {
  uint32_t validMessages;
  uint32_t invalidMessages;
  uint32_t outOfRangeNotes;
  uint32_t droppedMessages;
  uint32_t noteOnCount;
  uint32_t noteOffCount;
  uint32_t controlChangeCount;
  unsigned long lastMessageTime;
  uint32_t messagesPerSecond;
};

// Affichage périodique des stats
void printMidiStats() {
  Serial.println("========== MIDI Stats ==========");
  Serial.printf("Messages valides: %d\n", midiStats.validMessages);
  Serial.printf("Messages invalides: %d\n", midiStats.invalidMessages);
  Serial.printf("Notes hors plage: %d\n", midiStats.outOfRangeNotes);
  Serial.printf("Note On: %d | Note Off: %d\n",
                midiStats.noteOnCount, midiStats.noteOffCount);
  Serial.printf("Messages/s: %d\n", midiStats.messagesPerSecond);
  Serial.println("================================");
}
```

#### E. Gestion des erreurs
```cpp
// Handler d'erreur global
void onMidiError(byte errorCode, byte data1, byte data2) {
  switch(errorCode) {
    case ERROR_INVALID_CHANNEL:
      Serial.printf("[MIDI ERR] Canal invalide: %d\n", data1);
      break;
    case ERROR_INVALID_NOTE:
      Serial.printf("[MIDI ERR] Note invalide: %d\n", data1);
      break;
    case ERROR_SERVO_BUSY:
      Serial.printf("[MIDI ERR] Servo occupé: %d\n", data1);
      break;
    case ERROR_RATE_LIMIT:
      Serial.println("[MIDI ERR] Trop de messages!");
      break;
  }
  midiStats.invalidMessages++;
}
```

---

## 📤 3. ENVOI DE MESSAGES MIDI (NOUVEAU!)

### Problème actuel
- ❌ **AUCUN message MIDI n'est envoyé** (communication unidirectionnelle uniquement)
- ❌ Pas de feedback sur les notes jouées
- ❌ Pas de messages d'état

### Solutions proposées

#### A. Feedback MIDI pour notes jouées
```cpp
// Envoyer confirmation quand note effectivement jouée
void Instrument::noteOn(uint8_t midiNote, uint8_t velocity) {
  int16_t servo = getServo(midiNote);

  if (servo != -1) {
    servoController.pluck(servo);

    // NOUVEAU: Envoyer feedback MIDI
    sendMidiFeedback(MIDI_NOTE_ON, midiNote, velocity);

    if (DEBUG) {
      Serial.printf("[MIDI OUT] Note On: %d (vélocité: %d)\n",
                    midiNote, velocity);
    }
  } else {
    // Note non jouable - envoyer erreur
    sendMidiError(ERROR_NOTE_NOT_PLAYABLE, midiNote);
  }
}

void Instrument::noteOff(uint8_t midiNote) {
  int16_t servo = getServo(midiNote);

  if (servo != -1) {
    servoController.mute(servo);

    // NOUVEAU: Envoyer feedback MIDI
    sendMidiFeedback(MIDI_NOTE_OFF, midiNote, 0);
  }
}
```

#### B. Messages Control Change pour état système
```cpp
// Envoyer l'état du système via Control Change
void sendSystemStatus() {
  // CC 102 : État batterie (0-127, si alimenté par batterie)
  byte batteryLevel = getBatteryLevel();
  MIDI.sendControlChange(102, batteryLevel, MIDI_CHANNEL);

  // CC 103 : Température servos (0-127)
  byte temperature = getServoTemperature();
  MIDI.sendControlChange(103, temperature, MIDI_CHANNEL);

  // CC 104 : Nombre de servos actifs (0-16)
  byte activeServos = getActiveServoCount();
  MIDI.sendControlChange(104, activeServos, MIDI_CHANNEL);

  // CC 105 : Erreurs détectées (0 = OK, >0 = erreur)
  byte errorStatus = getErrorStatus();
  MIDI.sendControlChange(105, errorStatus, MIDI_CHANNEL);
}

// Envoyer périodiquement (toutes les 10 secondes si connecté)
void loop() {
  static unsigned long lastStatusSend = 0;

  if (isConnected && millis() - lastStatusSend > 10000) {
    sendSystemStatus();
    lastStatusSend = millis();
  }
}
```

#### C. Messages System Exclusive (SysEx) pour configuration
```cpp
// Format SysEx personnalisé:
// F0 7D [CMD] [DATA...] F7
// 7D = Manufacturer ID (Educational/Development use)

// Commandes SysEx
#define SYSEX_GET_CONFIG     0x01  // Lire configuration
#define SYSEX_SET_SERVO_ANGLE 0x02  // Définir angle servo
#define SYSEX_GET_SERVO_ANGLE 0x03  // Lire angle servo
#define SYSEX_RESET_SYSTEM   0x04  // Reset système
#define SYSEX_GET_VERSION    0x05  // Version firmware

// Handler SysEx
void onSystemExclusive(byte* data, unsigned int length) {
  if (length < 3) return;  // Trop court
  if (data[0] != 0xF0 || data[1] != 0x7D) return;  // Pas notre format

  byte command = data[2];

  switch(command) {
    case SYSEX_GET_CONFIG:
      sendConfigurationSysEx();
      break;

    case SYSEX_SET_SERVO_ANGLE:
      if (length >= 6) {
        byte servoNum = data[3];
        uint16_t angle = (data[4] << 7) | data[5];
        setServoAngle(servoNum, angle);
        sendAckSysEx();
      }
      break;

    case SYSEX_GET_SERVO_ANGLE:
      if (length >= 4) {
        byte servoNum = data[3];
        sendServoAngleSysEx(servoNum);
      }
      break;

    case SYSEX_RESET_SYSTEM:
      sendAckSysEx();
      delay(100);
      ESP.restart();
      break;

    case SYSEX_GET_VERSION:
      sendVersionSysEx();
      break;
  }
}

// Envoyer configuration complète
void sendConfigurationSysEx() {
  byte sysex[128];
  int idx = 0;

  sysex[idx++] = 0xF0;  // Start SysEx
  sysex[idx++] = 0x7D;  // Manufacturer ID
  sysex[idx++] = SYSEX_GET_CONFIG;

  // Ajouter configuration
  sysex[idx++] = NUM_SERVOS;
  sysex[idx++] = PLUCK_ANGLE;
  sysex[idx++] = MIDI_CHANNEL;

  // Angles de tous les servos (2 bytes par servo)
  for (int i = 0; i < NUM_SERVOS; i++) {
    uint16_t angle = initialAngles[i];
    sysex[idx++] = (angle >> 7) & 0x7F;  // MSB
    sysex[idx++] = angle & 0x7F;         // LSB
  }

  sysex[idx++] = 0xF7;  // End SysEx

  MIDI.sendSysEx(idx, sysex, true);
}
```

#### D. Active Sensing (vérification connexion)
```cpp
// Envoyer Active Sensing toutes les 300ms (standard MIDI)
#define ACTIVE_SENSING_INTERVAL_MS 300

void loop() {
  static unsigned long lastActiveSensing = 0;

  if (isConnected && millis() - lastActiveSensing > ACTIVE_SENSING_INTERVAL_MS) {
    MIDI.sendRealTime(midi::ActiveSensing);
    lastActiveSensing = millis();
  }
}
```

#### E. Messages d'erreur personnalisés
```cpp
// Utiliser Program Change pour signaler erreurs
// (alternative à Control Change)
void sendMidiError(byte errorType, byte errorData) {
  // PC 100-110 réservés pour erreurs
  byte errorPC = 100 + errorType;
  MIDI.sendProgramChange(errorPC, MIDI_CHANNEL);

  // Ajouter détails via CC
  MIDI.sendControlChange(127, errorData, MIDI_CHANNEL);

  if (DEBUG) {
    Serial.printf("[MIDI OUT] Erreur envoyée: Type=%d, Data=%d\n",
                  errorType, errorData);
  }
}

// Types d'erreur
#define ERROR_NOTE_NOT_PLAYABLE 1
#define ERROR_SERVO_TIMEOUT     2
#define ERROR_OVERLOAD          3
#define ERROR_PCA9685_FAIL      4
```

---

## ⚙️ 4. AUTRES AMÉLIORATIONS

### A. Sauvegarde en mémoire non-volatile (NVS)
```cpp
#include <Preferences.h>

Preferences preferences;

// Sauvegarder configuration
void saveConfiguration() {
  preferences.begin("lyre-config", false);

  // Sauvegarder angles servos
  for (int i = 0; i < NUM_SERVOS; i++) {
    char key[16];
    sprintf(key, "angle_%d", i);
    preferences.putUShort(key, initialAngles[i]);
  }

  // Sauvegarder paramètres MIDI
  preferences.putUChar("midi_channel", MIDI_CHANNEL);
  preferences.putBool("omni_mode", MIDI_OMNI_MODE);

  // Sauvegarder appareils appairés
  preferences.putUChar("paired_count", pairedDeviceCount);

  preferences.end();
  Serial.println("[NVS] Configuration sauvegardée");
}

// Charger configuration
void loadConfiguration() {
  preferences.begin("lyre-config", true);

  // Charger si existe
  if (preferences.isKey("angle_0")) {
    for (int i = 0; i < NUM_SERVOS; i++) {
      char key[16];
      sprintf(key, "angle_%d", i);
      initialAngles[i] = preferences.getUShort(key, 90);
    }
    Serial.println("[NVS] Configuration chargée");
  }

  preferences.end();
}
```

### B. Watchdog (redémarrage auto en cas de blocage)
```cpp
#include <esp_task_wdt.h>

#define WDT_TIMEOUT 30  // 30 secondes

void setup() {
  // Configurer watchdog
  esp_task_wdt_init(WDT_TIMEOUT, true);
  esp_task_wdt_add(NULL);

  Serial.println("[WDT] Watchdog activé (30s)");
}

void loop() {
  // Réinitialiser watchdog à chaque itération
  esp_task_wdt_reset();

  // ... reste du code
}
```

### C. Gestion de la batterie (optionnel)
```cpp
#define PIN_BATTERY_ADC 34  // GPIO 34 (ADC1_CH6)
#define BATTERY_MIN_VOLTAGE 3.0
#define BATTERY_MAX_VOLTAGE 4.2

byte getBatteryLevel() {
  int adcValue = analogRead(PIN_BATTERY_ADC);
  float voltage = (adcValue / 4095.0) * 3.3 * 2;  // Diviseur de tension

  // Convertir en pourcentage (0-127 pour MIDI)
  float percentage = (voltage - BATTERY_MIN_VOLTAGE) /
                     (BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE);
  percentage = constrain(percentage, 0.0, 1.0);

  return (byte)(percentage * 127);
}

// Alerte batterie faible
void checkBattery() {
  byte level = getBatteryLevel();

  if (level < 20) {  // Moins de ~15%
    Serial.println("[BATTERY] Batterie faible!");
    // Envoyer CC d'alerte
    MIDI.sendControlChange(102, level, MIDI_CHANNEL);

    // Clignoter LED
    blinkLED(10, 100);
  }
}
```

### D. Mode deep sleep (économie d'énergie)
```cpp
#include <esp_sleep.h>

void enterDeepSleep() {
  Serial.println("[POWER] Entrée en mode deep sleep...");

  // Désactiver servos
  servoController.disableServos();

  // Configurer réveil par bouton
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0);  // Réveil sur bouton BOOT

  // Sleep
  esp_deep_sleep_start();
}

// Auto-sleep après timeout sans connexion
void checkAutoSleep() {
  static unsigned long lastActivity = millis();

  if (!isConnected) {
    if (millis() - lastActivity > AUTO_SLEEP_TIMEOUT_MS) {
      enterDeepSleep();
    }
  } else {
    lastActivity = millis();
  }
}
```

---

## 📊 RÉCAPITULATIF DES AMÉLIORATIONS

| Catégorie | Fonctionnalité | Priorité | Complexité |
|-----------|----------------|----------|------------|
| **Appairage BLE** | Bouton appairage | 🔴 Haute | Faible |
| | LED d'état | 🔴 Haute | Faible |
| | Timeout appairage | 🟡 Moyenne | Faible |
| | Liste blanche MAC | 🟡 Moyenne | Moyenne |
| | Code PIN | 🟢 Basse | Moyenne |
| **Réception MIDI** | Filtrage canal | 🔴 Haute | Faible |
| | Validation messages | 🔴 Haute | Faible |
| | Anti-spam | 🟡 Moyenne | Moyenne |
| | Statistiques | 🟡 Moyenne | Faible |
| **Envoi MIDI** | Feedback notes | 🔴 Haute | Moyenne |
| | Control Change état | 🟡 Moyenne | Moyenne |
| | SysEx config | 🟢 Basse | Haute |
| | Active Sensing | 🟡 Moyenne | Faible |
| **Autres** | Sauvegarde NVS | 🟡 Moyenne | Moyenne |
| | Watchdog | 🔴 Haute | Faible |
| | Gestion batterie | 🟢 Basse | Moyenne |
| | Deep sleep | 🟢 Basse | Moyenne |

**Légende priorité :**
- 🔴 Haute : Amélioration critique
- 🟡 Moyenne : Amélioration recommandée
- 🟢 Basse : Amélioration optionnelle

---

## 🎯 RECOMMANDATIONS

### Version minimale (essentiels uniquement)
1. ✅ Bouton appairage
2. ✅ LED d'état BLE
3. ✅ Filtrage canal MIDI
4. ✅ Validation messages
5. ✅ Feedback MIDI (Note On/Off)
6. ✅ Watchdog

**Temps implémentation estimé :** 2-3 heures

### Version complète (toutes améliorations)
- Toutes les fonctionnalités listées ci-dessus
- Configuration via SysEx
- Statistiques détaillées
- Gestion batterie
- Mode deep sleep

**Temps implémentation estimé :** 6-8 heures

---

## 📝 PROCHAINES ÉTAPES

1. **Choisir** la version à implémenter (minimale/complète/personnalisée)
2. **Valider** les pins GPIO utilisées (bouton, LED, batterie)
3. **Implémenter** les modifications
4. **Tester** chaque fonctionnalité
5. **Documenter** les nouveaux paramètres dans README.md

---

## ❓ QUESTIONS À RÉSOUDRE

1. **Pins GPIO :** Quels pins utiliser pour bouton/LED ? (par défaut : GPIO 0 et 2)
2. **Batterie :** Système alimenté par batterie ou secteur uniquement ?
3. **Sécurité :** Besoin de code PIN ou liste blanche suffisante ?
4. **Canal MIDI :** Quel canal par défaut ? (actuellement aucun filtrage)
5. **Feedback MIDI :** Renvoyer toutes les notes ou seulement les erreurs ?

---

*Document créé le 2025-11-20*
*Analyse du projet 16-cords-lyre-midi - Version ESP32 BLE*
