# ESP32 Lyre BLE - Version Professionnelle 🎵

Version optimisée pour utilisation professionnelle avec feedback MIDI et réponse SysEx.

---

## 🆕 Fonctionnalités professionnelles

### ✅ Appairage automatique permanent
- **Aucun bouton requis** - Toujours prêt à se connecter
- **Reconnexion automatique** après déconnexion
- **Multi-plateformes** - iOS, Android, Windows, macOS

### ✅ Communication bidirectionnelle complète

**Messages envoyés par l'ESP32 :**

| Message | Code | Fonction |
|---------|------|----------|
| **Note On** | 0x90 | Confirmation note jouée |
| **Note Off** | 0x80 | Confirmation note arrêtée |
| **CC 102** | 0xB0 | État connexion (127=connecté, 0=déconnecté) |
| **CC 103** | 0xB0 | Heartbeat toutes les 30s (appareil vivant) |
| **SysEx Identity Reply** | 0xF0 7E... | Identification automatique |

### ✅ Réponse SysEx Identity Request

Quand l'interface envoie : `F0 7E 7F 06 01 F7` (Identity Request)

L'ESP32 répond automatiquement :
```
F0 7E 7F 06 02    // Identity Reply header
   7D             // Manufacturer ID (Educational)
   00 01          // Family (0x0100)
   00 01          // Model (0x0100)
   01 00 00 00    // Version 1.0.0.0
F7                // End SysEx
```

### ✅ Optimisations de performance

- **MTU maximisé** (517 bytes) pour paquets MIDI plus gros
- **Interval de connexion minimal** (7.5ms) pour latence ultra-faible
- **Délai loop = 1ms** pour réactivité maximale
- **Traitement prioritaire** des messages MIDI

---

## 📊 Surveillance de l'état

### Messages de connexion

**À la connexion :**
```
1. CC 102 = 127   (Connexion établie)
2. SysEx Identity Reply (Identification automatique)
```

**À la déconnexion :**
```
1. CC 102 = 0     (Déconnexion)
```

**Pendant la session :**
```
Toutes les 30s: CC 103 = 127 (Heartbeat/Alive)
```

### Détection de l'état depuis votre interface

**Pseudo-code de votre application :**
```javascript
// À l'ouverture de connexion
function onMIDIConnect() {
  connectionTimeout = setTimeout(() => {
    console.error("Pas de réponse de l'appareil");
  }, 5000);
}

// À réception d'un message MIDI
function onMIDIMessage(status, data1, data2) {
  // Control Change reçu
  if (status === 0xB0) {
    // CC 102 = Connection Status
    if (data1 === 102) {
      clearTimeout(connectionTimeout);
      if (data2 === 127) {
        console.log("✓ ESP32 connecté et prêt");
        deviceConnected = true;
      } else if (data2 === 0) {
        console.log("✗ ESP32 déconnecté");
        deviceConnected = false;
      }
    }

    // CC 103 = Heartbeat
    if (data1 === 103 && data2 === 127) {
      console.log("♥ ESP32 alive");
      lastHeartbeat = Date.now();
    }
  }

  // SysEx Identity Reply
  if (status === 0xF0) {
    if (isSysExIdentityReply(data)) {
      console.log("✓ Appareil identifié: ESP32 Lyre");
      deviceIdentified = true;
    }
  }
}

// Vérifier le heartbeat
setInterval(() => {
  if (Date.now() - lastHeartbeat > 60000) {
    console.warn("⚠ Pas de heartbeat depuis 60s");
  }
}, 10000);
```

---

## 🎹 Feedback MIDI des notes

Quand **activé** (`MIDI_SEND_FEEDBACK = true`) :

**Vous envoyez** : Note On 60, velocity 100
**Vous recevez** : Note On 60, velocity 100 (confirmation)

**Avantages :**
- ✅ Confirmation que la note a été **réellement jouée**
- ✅ Synchronisation interface ↔ hardware
- ✅ Détection notes non jouables (pas de feedback = note ignorée)

**Pour désactiver** (mode unidirectionnel) :
```cpp
// Dans settings.h
#define MIDI_SEND_FEEDBACK false
```

---

## 🚀 Installation

### 1. Bibliothèques

**Une seule requise :**
```
Arduino IDE → Gérer les bibliothèques
Chercher: "Adafruit PWM Servo Driver"
Installer
```

### 2. Upload

```
Ouvrir: ESP32_Lyre_BLE_Pro.ino
Carte: ESP32 Dev Module
Compiler et uploader
```

### 3. Connexion

```
Moniteur Série (115200 bauds):
========================================
  ESP32 BLE MIDI Lyre - Pro Version
========================================
Firmware: ESP32-Lyre v1.0

[I2C] Pins par défaut (SDA=21, SCL=22)
[BLE] Initialisation...
[BLE] ✓ Service MIDI actif
[BLE] Nom: Lyre-MIDI-ESP32
[BLE] Appairage permanent activé
========================================

En attente de connexion...
```

### 4. Test depuis votre app

**Envoyer Identity Request :**
```
F0 7E 7F 06 01 F7
```

**Recevoir :**
```
F0 7E 7F 06 02 7D 00 01 00 01 01 00 00 00 F7
```

✅ **Si vous recevez la réponse = tout fonctionne !**

---

## 🔧 Configuration

### Dans `settings.h`

```cpp
// Nom Bluetooth (visible dans scan)
#define BLE_DEVICE_NAME "Lyre-MIDI-ESP32"

// Feedback MIDI (confirmations)
#define MIDI_SEND_FEEDBACK true  // true/false

// Debug (messages Serial Monitor)
#define DEBUG 1  // 0=OFF, 1=ON

// Plage de notes
#define MIDI_NOTE_MIN 55  // G3
#define MIDI_NOTE_MAX 81  // A5
```

---

## 📡 Protocole de communication

### Initialisation de connexion (séquence automatique)

```
1. App se connecte en BLE
2. ESP32 détecte connexion
3. ESP32 envoie: CC 102 = 127 (Connecté)
4. ESP32 envoie: SysEx Identity Reply
5. App reçoit et confirme l'identité
6. Communication établie ✓
```

### Pendant la session

```
App → ESP32:  Note On 60, vel 100
ESP32 joue la note
ESP32 → App:  Note On 60, vel 100 (confirmation)

App → ESP32:  Note Off 60
ESP32 arrête la note
ESP32 → App:  Note Off 60 (confirmation)

Toutes les 30s:
ESP32 → App:  CC 103 = 127 (Heartbeat)
```

### Déconnexion

```
1. Connexion perdue
2. ESP32 redémarre advertising automatiquement
3. Prêt pour nouvelle connexion
```

---

## 🎯 Cas d'usage

### Mode performance live
```cpp
#define MIDI_SEND_FEEDBACK false  // Pas de feedback (latence min)
#define DEBUG 0                    // Pas de Serial (perf max)
```

### Mode développement/debug
```cpp
#define MIDI_SEND_FEEDBACK true   // Feedback complet
#define DEBUG 1                    // Logs détaillés
```

### Mode interface graphique
```cpp
#define MIDI_SEND_FEEDBACK true   // Synchronisation UI
#define DEBUG 0                    // Pas de logs
```

---

## 📊 Performances

| Métrique | Valeur |
|----------|--------|
| **Latence BLE** | ~7.5 ms (connexion rapide activée) |
| **Latence traitement MIDI** | < 1 ms |
| **Latence servo** | ~100-200 ms (mécanique) |
| **Latence totale** | ~110-210 ms |
| **Débit max** | ~50 notes/seconde |
| **MTU** | 517 bytes (optimisé) |

---

## 🐛 Dépannage

### L'app ne reçoit pas de feedback

```cpp
// Vérifier dans settings.h:
#define MIDI_SEND_FEEDBACK true  // Doit être true

// Vérifier que votre app écoute les messages entrants
```

### Pas de réponse Identity Request

```
1. Vérifier le format exact: F0 7E 7F 06 01 F7
2. Activer DEBUG=1 et voir si "[SYSEX] Identity Request reçu"
3. Vérifier que votre app écoute les SysEx
```

### Heartbeat ne fonctionne pas

```
- Normal si connexion < 30 secondes
- Premier heartbeat arrive à t=30s
- Puis toutes les 30s
```

### LED ne clignote pas

**Version Pro n'utilise PAS de LED physique**
- État de connexion = via MIDI (CC 102)
- Pas de matériel supplémentaire requis

---

## 🔄 Différences avec autres versions

| Fonctionnalité | Standard | Enhanced | **Pro** |
|----------------|----------|----------|---------|
| SysEx Identity | ❌ | ❌ | ✅ |
| Feedback MIDI | ❌ | ✅ | ✅ Optimisé |
| CC Connection Status | ❌ | ❌ | ✅ |
| Heartbeat | ❌ | ❌ | ✅ |
| Bouton physique | ❌ | ✅ | ❌ |
| LED physique | ❌ | ✅ | ❌ |
| MTU optimisé | ❌ | ❌ | ✅ 517 bytes |
| Interval mini | ❌ | ❌ | ✅ 7.5ms |
| Appairage auto | ✅ | ❌ | ✅ Permanent |

**👉 Version Pro = Meilleur choix pour interfaces logicielles professionnelles**

---

## 📝 Exemples d'intégration

### Python (mido)
```python
import mido

def on_message(msg):
    if msg.type == 'control_change':
        if msg.control == 102:
            if msg.value == 127:
                print("✓ ESP32 connecté")
            elif msg.value == 0:
                print("✗ ESP32 déconnecté")
        elif msg.control == 103:
            print("♥ Heartbeat")

    elif msg.type == 'note_on':
        print(f"✓ Note {msg.note} confirmée")

port = mido.open_input('Lyre-MIDI-ESP32')
for msg in port:
    on_message(msg)
```

### JavaScript (Web MIDI API)
```javascript
navigator.requestMIDIAccess({sysex: true}).then(access => {
  for (let input of access.inputs.values()) {
    if (input.name.includes('Lyre-MIDI')) {
      input.onmidimessage = (msg) => {
        const [status, data1, data2] = msg.data;

        // Connection Status
        if (status === 0xB0 && data1 === 102) {
          console.log(data2 === 127 ? "✓ Connecté" : "✗ Déconnecté");
        }

        // Heartbeat
        if (status === 0xB0 && data1 === 103) {
          console.log("♥ Alive");
        }

        // Note feedback
        if (status === 0x90) {
          console.log(`✓ Note ${data1} jouée (vel ${data2})`);
        }
      };
    }
  }
});
```

---

**Version** : 1.0 Professional
**Compatible** : Toutes apps MIDI BLE
**Testé** : ESP32 WROOM-32D
**Sans matériel supplémentaire requis** ✅

🎵 **Prêt pour intégration professionnelle !**
