# Lyre MIDI 16 notes - ESP32 BLE ENHANCED 🚀

**Version améliorée** avec contrôle d'appairage, feedback MIDI, et fonctionnalités avancées.

## 🆕 Nouveautés par rapport à la version standard

### 🔐 Contrôle d'appairage BLE
- ✅ **Bouton physique** pour activer/désactiver l'appairage (GPIO 0 - bouton BOOT)
- ✅ **LED d'état** indiquant la connexion BLE (GPIO 2 - LED intégrée)
- ✅ **Timeout automatique** d'appairage (5 minutes)
- ✅ **Reset liste appareils** avec appui long (3 secondes)

### 📥 Réception MIDI améliorée
- ✅ **Filtrage par canal MIDI** (configurable 1-16 ou mode omni)
- ✅ **Validation complète** des messages (canal, note, vélocité)
- ✅ **Protection anti-spam** (limite 50 notes/seconde)
- ✅ **Statistiques en temps réel** (messages reçus, erreurs, taux)
- ✅ **Gestion d'erreurs** avec codes spécifiques

### 📤 Envoi de messages MIDI (NOUVEAU!)
- ✅ **Feedback MIDI** : Confirmation Note On/Off quand note jouée
- ✅ **Messages d'erreur** : Notification si note non jouable
- ✅ **Control Change** : Codes d'erreur (CC 127/126)
- ✅ **All Notes Off** : Support CC 123

### ⚙️ Stabilité et debug
- ✅ **Watchdog** : Redémarrage automatique en cas de blocage (30s)
- ✅ **Commandes série** : Interface debug complète
- ✅ **Logs détaillés** : Suivi de toutes les opérations
- ✅ **Gestion mémoire** optimisée

---

## 🎛️ Utilisation du bouton d'appairage

### Bouton BOOT (GPIO 0)

**Appui court (< 1 seconde):**
- Active/désactive le mode appairage
- LED clignote rapidement quand appairage actif
- Timeout automatique après 5 minutes

**Appui long (> 3 secondes):**
- Efface la liste des appareils appairés
- Confirmation par 5 clignotements rapides de la LED

### États LED (GPIO 2)

| État LED | Signification |
|----------|---------------|
| **Éteinte** | Appairage désactivé, pas de connexion |
| **Clignotement lent** (1 Hz) | Recherche de connexion |
| **Clignotement rapide** (5 Hz) | Mode appairage actif |
| **Fixe** | Connecté à un appareil |

---

## 🎹 Configuration MIDI

### Filtrage par canal

Par défaut, l'instrument écoute **uniquement le canal 1**. Pour changer :

```cpp
// Dans settings.h
#define MIDI_CHANNEL 1        // Canal 1-16
#define MIDI_OMNI_MODE false  // true = tous canaux, false = canal unique
```

### Feedback MIDI

Quand activé, l'instrument **renvoie** les messages MIDI :
- **Note On** confirmée quand servo effectivement activé
- **Note Off** confirmée quand servo revient en position repos
- **Erreurs** envoyées si note non jouable (CC 127/126)

```cpp
// Dans settings.h
#define MIDI_SEND_FEEDBACK true  // true = feedback activé
```

**Exemple de feedback :**
```
Réception:  Note On  channel=1, note=60 (C4), velocity=100
 ↓
Vérifications : canal OK, note OK, vélocité OK
 ↓
Servo joue la note
 ↓
Envoi:      Note On  channel=1, note=60, velocity=100  (confirmation)
```

### Protection anti-spam

Limite le nombre de notes par seconde pour éviter la surcharge :

```cpp
// Dans settings.h
#define MAX_NOTES_PER_SECOND 50         // Max 50 notes/s
#define ENABLE_RATE_LIMITING true       // Activer protection
```

Si la limite est dépassée :
- Message dropped (ignoré)
- Erreur envoyée via CC 127/126
- Compteur dans les statistiques

---

## 🖥️ Commandes série (Debug)

Ouvrir le **Moniteur série** (115200 bauds) et taper :

| Commande | Action |
|----------|--------|
| `s` | Afficher statistiques MIDI |
| `r` | Reset statistiques MIDI |
| `i` | Informations système |
| `p` | Toggle appairage BLE |
| `h` | Afficher aide |

### Exemple de statistiques

```
========== MIDI STATISTICS ==========
Messages valides:    1543
Messages invalides:  12
Notes hors plage:    5
Messages dropped:    0
Erreurs envoyées:    17
-------------------------------------
Note On:             1234
Note Off:            1234
Control Change:      75
-------------------------------------
Messages/seconde:    23
Dernier message:     125 ms
=====================================
```

### Exemple d'informations système

```
========== SYSTEM INFO ==========
Firmware:        2.0 (2025-11-20)
Device Name:     Lyre-MIDI-ESP32
MIDI Channel:    1
Omni Mode:       OFF
Feedback MIDI:   ON
Rate Limiting:   ON (50 notes/s max)
---------------------------------
BLE Connected:   YES
Pairing State:   CONNECTED
Free Heap:       245632 bytes
Uptime:          125436 ms
=================================
```

---

## 📊 Messages MIDI envoyés

### Feedback de notes

| Message | Condition | Description |
|---------|-----------|-------------|
| **Note On** | Note jouée avec succès | Confirmation avec même note et vélocité |
| **Note Off** | Servo retour repos | Confirmation note arrêtée |

### Messages d'erreur (Control Change)

L'instrument envoie des **Control Change** pour signaler les erreurs :

- **CC 127** = Type d'erreur
- **CC 126** = Données d'erreur (note, vélocité, etc.)

**Codes d'erreur (CC 127) :**

| Code | Nom | Description |
|------|-----|-------------|
| 1 | `ERROR_NOTE_NOT_PLAYABLE` | Note hors de la plage supportée |
| 2 | `ERROR_SERVO_TIMEOUT` | Servo n'a pas répondu |
| 3 | `ERROR_RATE_LIMIT` | Trop de notes par seconde |
| 4 | `ERROR_INVALID_CHANNEL` | Canal MIDI incorrect |
| 5 | `ERROR_INVALID_VELOCITY` | Vélocité hors plage |

**Exemple :**
```
Réception:  Note On  note=100 (E7) - hors plage (55-81)
 ↓
Envoi:      CC 127 = 1  (ERROR_NOTE_NOT_PLAYABLE)
            CC 126 = 100 (note concernée)
```

### All Notes Off (CC 123)

Supporte le standard MIDI **All Notes Off** :
- Reçoit CC 123 → Arrête tous les servos
- Remet tous les servos en position de repos

---

## 🔧 Configuration avancée

### Modifier les pins

```cpp
// Dans settings.h

// Pin bouton appairage (défaut: GPIO 0 = bouton BOOT)
#define PIN_PAIRING_BUTTON 0

// Pin LED d'état (défaut: GPIO 2 = LED intégrée)
#define PIN_BLE_LED 2

// Pin contrôle servos
#define PIN_SERVO_OE 5
```

### Modifier les timeouts

```cpp
// Timeout appairage (défaut: 5 minutes)
#define PAIRING_TIMEOUT_MS 300000

// Durée appui long pour reset (défaut: 3 secondes)
#define PAIRING_LONG_PRESS_MS 3000

// Timeout watchdog (défaut: 30 secondes)
#define WATCHDOG_TIMEOUT_SEC 30

// Timeout auto-disable servos (défaut: 2 secondes)
#define SERVO_AUTO_DISABLE_TIMEOUT_MS 2000
```

### Activer/désactiver fonctionnalités

```cpp
// Mode debug (logs détaillés)
#define DEBUG 0  // 0=OFF, 1=ON

// Feedback MIDI
#define MIDI_SEND_FEEDBACK true

// Active Sensing (messages périodiques)
#define MIDI_SEND_ACTIVE_SENSING false

// Rate limiting
#define ENABLE_RATE_LIMITING true

// Watchdog
#define ENABLE_WATCHDOG true
```

---

## 🐛 Dépannage

### Problème : Aucun son même si connecté

1. Vérifier le canal MIDI dans votre DAW/app
2. Taper `i` dans le moniteur série → vérifier `MIDI Channel`
3. Activer mode omni : `#define MIDI_OMNI_MODE true`

### Problème : Messages ignorés (dropped)

1. Taper `s` → vérifier `Messages dropped`
2. Si élevé → augmenter limite : `#define MAX_NOTES_PER_SECOND 100`
3. Ou désactiver : `#define ENABLE_RATE_LIMITING false`

### Problème : LED ne s'allume pas

1. Vérifier GPIO 2 utilisé pour autre chose ?
2. Changer pin : `#define PIN_BLE_LED 4` (ou autre)
3. Certains ESP32 ont la LED sur GPIO 5 ou 22

### Problème : Redémarrages intempestifs

1. Watchdog trop court → augmenter timeout
2. Ou désactiver : `#define ENABLE_WATCHDOG false`
3. Vérifier alimentation servos (8A minimum)

### Problème : Pas de feedback MIDI

1. Vérifier : `#define MIDI_SEND_FEEDBACK true`
2. Dans votre DAW, activer "MIDI Thru" ou "Local Control"
3. Certaines apps filtrent les messages retour

---

## 📈 Performances

### Latence MIDI

- **BLE MIDI** : ~10-20 ms (typique)
- **Validation + Rate limit** : < 1 ms
- **Servo response** : ~100-200 ms (mécanique)

**Latence totale estimée** : 110-220 ms

### Consommation

- **ESP32 seul** : ~80 mA (Bluetooth actif)
- **16 servos actifs** : ~2000-5000 mA (selon charge)
- **Auto-disable** : Servos désactivés après 2s inactivité

### Capacité

- **Max notes/seconde** : 50 (configurable)
- **Polyphonie** : 16 notes simultanées max
- **Portée Bluetooth** : ~10 mètres en intérieur

---

## 🔄 Différences avec version standard

| Fonctionnalité | Standard | Enhanced |
|----------------|----------|----------|
| Bouton appairage | ❌ | ✅ GPIO 0 |
| LED d'état | ❌ | ✅ GPIO 2 |
| Filtrage canal MIDI | ❌ | ✅ Configurable |
| Validation messages | ❌ | ✅ Complète |
| Protection spam | ❌ | ✅ 50 notes/s |
| Feedback MIDI | ❌ | ✅ Note On/Off |
| Messages erreur | ❌ | ✅ CC 127/126 |
| Statistiques | ❌ | ✅ Temps réel |
| Watchdog | ❌ | ✅ 30 secondes |
| Commandes série | ❌ | ✅ h/s/r/i/p |
| All Notes Off | ❌ | ✅ CC 123 |

---

## 📦 Installation

1. **Copier** le dossier `Servo_pluck_ESP32_BLE_Enhanced/`
2. **Ouvrir** le fichier `.ino` dans Arduino IDE
3. **Installer** les bibliothèques requises :
   - Adafruit PWM Servo Driver Library
   - ESP32-BLE-MIDI (lathoub)
4. **Sélectionner** la carte ESP32
5. **Compiler** et téléverser

## 🎯 Utilisation rapide

1. **Téléverser** le code sur ESP32
2. **Appuyer** sur bouton BOOT (GPIO 0) → LED clignote rapidement
3. **Connecter** depuis app MIDI BLE (chercher "Lyre-MIDI-ESP32")
4. **Jouer** des notes MIDI sur canal 1 (55-81)
5. **Observer** feedback dans moniteur série (115200 bauds)

---

## 📚 Documentation complète

- **Analyse détaillée** : Voir `AMELIORATIONS_PROPOSEES.md` à la racine
- **Version standard** : Voir `../Servo_pluck_ESP32_BLE/README.md`
- **Matériel** : Voir README principal du projet

---

## 🔮 Améliorations futures possibles

- [ ] Sauvegarde NVS des appareils appairés
- [ ] Code PIN pour sécuriser appairage
- [ ] Configuration via SysEx
- [ ] Active Sensing périodique
- [ ] Gestion batterie avec ADC
- [ ] Mode deep sleep
- [ ] Interface web de configuration
- [ ] OTA (update firmware par Bluetooth)

---

**Version** : 2.0 Enhanced
**Date** : 2025-11-20
**Auteur** : Adaptation ESP32 BLE Enhanced

Pour toute question, vérifiez d'abord le moniteur série et tapez `h` pour l'aide ! 🎵
