# Lyre MIDI ESP32 - Version BLE Natif

## ✅ **Solution sans bibliothèque externe**

Cette version utilise **uniquement le BLE natif ESP32** avec le protocole MIDI implémenté directement dans le code.

### Avantages
- ✅ **Aucune bibliothèque externe** BLE MIDI requise
- ✅ **Plus stable** et prévisible
- ✅ **Fonctionne sur tous les ESP32** (WROOM-32D testé)
- ✅ **Compatible** avec toutes les apps MIDI BLE

---

## 📦 Installation

### Bibliothèques requises

**Une seule bibliothèque externe :**
```
Adafruit PWM Servo Driver Library
```

**BLE natif ESP32** est déjà inclus dans le core ESP32, rien à installer !

### Étapes

1. **Ouvrir** `Servo_pluck_ESP32_BLE_Natif.ino`
2. **Installer** uniquement "Adafruit PWM Servo Driver Library"
3. **Compiler et téléverser**

---

## 🎹 Utilisation

### Connexion

1. **Uploader** le code sur l'ESP32
2. **Ouvrir** le Moniteur Série (115200 bauds)
3. **Vérifier** le message:
   ```
   [BLE] ✓ Service MIDI démarré
   [BLE] Nom: Lyre-MIDI-ESP32
   [BLE] En attente de connexion...
   ```

4. **Connecter** depuis votre app MIDI :
   - **iOS**: "MIDI BLE Connect" → chercher "Lyre-MIDI-ESP32"
   - **Android**: "MIDI BLE Connect" → chercher "Lyre-MIDI-ESP32"
   - **macOS**: Configuration Audio MIDI → Bluetooth → "Lyre-MIDI-ESP32"
   - **Windows**: "MIDIberry" → "Lyre-MIDI-ESP32"

5. **Jouer** des notes MIDI (55-81, canal quelconque)

### LED d'état (GPIO 2)

| État | Signification |
|------|---------------|
| **Clignotement** (0.5 Hz) | En attente de connexion |
| **Fixe** | Connecté |

---

## 🎛️ Configuration

### Dans `settings.h`

```cpp
// Nom Bluetooth
#define BLE_DEVICE_NAME "Lyre-MIDI-ESP32"

// Debug (afficher messages MIDI reçus)
#define DEBUG 0  // 0=OFF, 1=ON

// Plage de notes supportées
#define MIDI_NOTE_MIN 55  // G3
#define MIDI_NOTE_MAX 81  // A5

// Pins
#define PIN_SERVO_OE 5    // Contrôle servos
// I2C par défaut: SDA=21, SCL=22
```

### Mode debug

Pour voir les messages MIDI reçus :
```cpp
#define DEBUG 1
```

Sortie :
```
[MIDI IN] Note On: 60 (vel: 100) canal: 1
[MIDI IN] Note Off: 60 canal: 1
```

---

## 📊 Messages MIDI supportés

| Message | Code | Supporté | Action |
|---------|------|----------|--------|
| **Note On** | 0x90 | ✅ | Joue la note (servo) |
| **Note Off** | 0x80 | ✅ | Arrête la note (repos) |
| **Control Change 123** | 0xB0 | ✅ | All Notes Off |
| Pitch Bend | 0xE0 | ❌ | Non implémenté |
| Aftertouch | 0xD0/0xA0 | ❌ | Non implémenté |

### Notes

- **Tous les canaux MIDI** sont acceptés (pas de filtrage)
- **Velocity 0** est interprété comme Note Off
- **Notes hors plage** (< 55 ou > 81) sont ignorées

---

## 🔧 Dépannage

### L'appareil n'apparaît pas dans l'app MIDI

1. **Vérifier le Moniteur Série** :
   ```
   [BLE] ✓ Service MIDI démarré
   ```
   → Si ce message apparaît, le BLE fonctionne

2. **Redémarrer l'app MIDI** sur le téléphone/tablette

3. **Vider le cache Bluetooth** :
   - iOS : Désactiver/réactiver Bluetooth
   - Android : Paramètres → Apps → Bluetooth → Vider cache

4. **Essayer depuis un autre appareil** pour isoler le problème

### Servos ne bougent pas

1. **Vérifier l'alimentation** 5V 8A des servos
2. **Ouvrir Moniteur Série** en mode DEBUG=1
3. **Envoyer une note MIDI** et vérifier :
   ```
   [MIDI IN] Note On: 60 (vel: 100) canal: 1
   ```
4. **Vérifier connexion I2C** du PCA9685 :
   - SDA = GPIO 21
   - SCL = GPIO 22

### Déconnexions fréquentes

- **Trop de distance** → Rapprocher appareil (< 5m)
- **Interférences WiFi** → Désactiver WiFi sur ESP32
- **Alimentation insuffisante** → Vérifier alim servos

---

## ⚙️ Différences avec les autres versions

| Caractéristique | BLE Natif | Standard | Enhanced |
|-----------------|-----------|----------|----------|
| **Bibliothèque BLE** | Aucune (natif) | ESP32-BLE-MIDI | ESP32-BLE-MIDI |
| **Stabilité** | ✅ Excellente | ⚠️ Dépend lib | ⚠️ Dépend lib |
| **Installation** | ✅ Simple | ⚠️ Bibliothèque externe | ⚠️ Bibliothèque externe |
| **Filtrage canal** | ❌ Tous canaux | ❌ | ✅ Configurable |
| **Feedback MIDI** | ❌ | ❌ | ✅ |
| **Bouton appairage** | ❌ | ❌ | ✅ |
| **Statistiques** | ❌ | ❌ | ✅ |

**Recommandation :** Utilisez cette version **BLE Natif** si vous avez des problèmes avec les bibliothèques externes.

---

## 🚀 Améliorations futures possibles

Pour ajouter des fonctionnalités, voir les autres versions :
- **Enhanced** : Bouton appairage, LED, feedback MIDI, statistiques
- **Standard** : Version de base avec bibliothèque

Ou adaptez ce code en ajoutant :
- Filtrage par canal MIDI
- Envoi de feedback MIDI
- Statistiques
- Bouton de contrôle

---

## 📝 Notes techniques

### Protocole BLE MIDI

Ce code implémente le protocole Apple BLE MIDI :
- **Service UUID** : `03b80e5a-ede8-4b33-a751-6ce34ec4c700`
- **Characteristic UUID** : `7772e5db-3868-4112-a1a9-f2669d106bf3`

Format des messages :
```
[header] [timestamp] [status] [data1] [data2]
```

### UUIDs standard

Ces UUIDs sont les standards Apple pour BLE MIDI, reconnus par toutes les apps compatibles.

---

**Version** : 3.0 BLE Natif
**Date** : 2025-11-20
**Testé sur** : ESP32 WROOM-32D

Simple, stable, fonctionnel ! 🎵
