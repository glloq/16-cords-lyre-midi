# Lyre MIDI 16 notes - Version ESP32 avec Bluetooth BLE

Cette version utilise un **ESP32** et le **Bluetooth Low Energy (BLE) MIDI** pour contrôler la lyre sans fil.

## Différences avec la version Arduino Leonardo

| Caractéristique | Arduino Leonardo | ESP32 BLE |
|----------------|------------------|-----------|
| Communication MIDI | USB (MIDIUSB) | Bluetooth Low Energy |
| Bibliothèque MIDI | MIDIUSB.h | BLEMIDI (ESP32-BLE-MIDI) |
| Connexion | Câble USB | Sans fil Bluetooth |
| Pins I2C | SDA/SCL standards | GPIO 21 (SDA), GPIO 22 (SCL) |
| Nom appareil | - | "Lyre-MIDI-ESP32" |

## Matériel requis

- **ESP32** (Dev Kit, NodeMCU-32S, ou compatible)
- **PCA9685** - Contrôleur PWM I2C 16 canaux
- **16 servomoteurs SG90**
- **Alimentation 5V 8A** pour les servos
- Support en bois et pièces 3D (voir dossier STL du projet principal)
- Alimentation 230V avec fusible

## Câblage ESP32

### Connexions I2C (ESP32 → PCA9685)
- **GPIO 21** (SDA) → SDA du PCA9685
- **GPIO 22** (SCL) → SCL du PCA9685
- **GND** → GND du PCA9685
- **3.3V** ou **5V** → VCC du PCA9685 (vérifier la compatibilité de votre module)

### Pin de contrôle OE
- **GPIO 5** → Pin OE du PCA9685 (économie d'énergie)

### Alimentation
- **V+** du PCA9685 → **+5V** de l'alimentation externe (8A)
- **GND** du PCA9685 → **GND** commun (ESP32 + alimentation)

> **⚠️ IMPORTANT**: Les servomoteurs doivent être alimentés par une source externe de 5V 8A minimum. Ne PAS alimenter les servos via l'ESP32 !

## Bibliothèques requises

Installez ces bibliothèques via le gestionnaire de bibliothèques Arduino IDE :

1. **Adafruit PWM Servo Driver Library**
   - Auteur: Adafruit
   - Pour contrôler le PCA9685

2. **ESP32-BLE-MIDI** (ou BLE-MIDI)
   - Auteur: lathoub
   - URL: https://github.com/lathoub/Arduino-BLE-MIDI
   - Pour la communication MIDI via Bluetooth

3. **Support ESP32**
   - Dans Arduino IDE: Fichier → Préférences
   - Ajouter l'URL: `https://dl.espressif.com/dl/package_esp32_index.json`
   - Outils → Type de carte → Gestionnaire de carte → Installer "ESP32"

## Installation et utilisation

### 1. Préparer Arduino IDE
```
1. Installer les bibliothèques requises
2. Sélectionner la carte: Outils → Type de carte → ESP32 Dev Module
3. Sélectionner le port COM approprié
```

### 2. Configuration (optionnel)
Modifier `settings.h` si nécessaire :
- `BLE_DEVICE_NAME` : Nom Bluetooth de l'appareil
- `PIN_SERVO_OE` : Pin pour contrôle OE du PCA9685
- `I2C_SDA` / `I2C_SCL` : Pins I2C personnalisées (décommenter si besoin)
- `initialAngles[]` : Angles de repos pour chaque servo
- `MidiServoMapping[]` : Mapping notes MIDI → numéros de cordes

### 3. Téléverser le code
1. Brancher l'ESP32 via USB
2. Compiler et téléverser le sketch
3. Ouvrir le Moniteur série (115200 bauds) pour voir les messages de debug

### 4. Connexion Bluetooth MIDI

#### Sur iOS (iPhone/iPad)
1. Télécharger une application MIDI BLE (ex: "MIDI BLE Connect", "midimittr")
2. Chercher "Lyre-MIDI-ESP32" dans les appareils Bluetooth
3. Se connecter
4. Utiliser une application DAW ou synthétiseur MIDI

#### Sur Android
1. Télécharger "MIDI BLE Connect" ou équivalent
2. Activer le Bluetooth
3. Chercher et connecter "Lyre-MIDI-ESP32"
4. Utiliser avec une application de musique compatible MIDI

#### Sur Windows 10/11
1. Activer le Bluetooth
2. Utiliser un logiciel comme "MIDIberry" ou "Bluetooth MIDI Connector"
3. Connecter à "Lyre-MIDI-ESP32"
4. Utiliser avec votre DAW préférée

#### Sur macOS
1. Ouvrir "Configuration MIDI Audio" (dans /Applications/Utilitaires)
2. Fenêtre → Afficher la fenêtre MIDI Bluetooth
3. Connecter "Lyre-MIDI-ESP32"
4. Utiliser avec GarageBand, Logic Pro, etc.

## Configuration MIDI

### Notes supportées
- **Plage**: G3 (MIDI 55) à A5 (MIDI 81)
- **16 notes** jouables selon le mapping dans `settings.h`

### Messages MIDI supportés
- ✅ **Note On** (0x90)
- ✅ **Note Off** (0x80)
- ✅ **Control Change** (0xB0) - partiellement
- ⚠️ Pitch Bend, Aftertouch (désactivés par défaut, décommenter dans le code)

## Debug et dépannage

### Moniteur série
Ouvrir le moniteur série à **115200 bauds** pour voir :
- État de l'initialisation
- Messages de connexion/déconnexion BLE
- Détection du PCA9685
- Messages MIDI reçus (si DEBUG=1 dans settings.h)

### Problèmes courants

**"PCA9685 non détecté"**
- Vérifier les connexions I2C (SDA/SCL)
- Vérifier l'alimentation du PCA9685
- Essayer de changer l'adresse I2C (par défaut 0x40)

**"Pas de connexion Bluetooth"**
- Vérifier que le Bluetooth est activé sur l'appareil
- Redémarrer l'ESP32
- Vérifier que le nom BLE apparaît dans les paramètres Bluetooth

**"Servos ne bougent pas"**
- Vérifier l'alimentation externe 5V 8A
- Vérifier le pin OE (doit être LOW pour activer)
- Ouvrir le moniteur série pour voir les erreurs

**"Servos tremblent"**
- Alimentation insuffisante → utiliser une alimentation plus puissante
- Câbles trop longs → raccourcir les câbles d'alimentation

## Mode Debug

Pour activer les messages de debug détaillés :
```cpp
// Dans settings.h
#define DEBUG 1  // Changer 0 en 1
```

Cela affichera :
- Détails de l'initialisation des servos
- Messages MIDI reçus
- Positions des servos
- Erreurs détaillées

## Économie d'énergie

Le système désactive automatiquement les servos après **2 secondes** d'inactivité (configurable via `SERVO_AUTO_DISABLE_TIMEOUT_MS`). Les servos sont réactivés automatiquement lors de la prochaine note jouée.

## Structure des fichiers

```
Servo_pluck_ESP32_BLE/
├── Servo_pluck_ESP32_BLE.ino  # Fichier principal
├── settings.h                  # Configuration (pins, angles, mapping MIDI)
├── MidiHandler.h/cpp          # Gestion messages MIDI BLE
├── Instrument.h/cpp           # Logique instrument
├── ServoController.h/cpp      # Contrôle des servos via PCA9685
└── README.md                  # Cette documentation
```

## Amélirations possibles

- [ ] Ajouter un mode WiFi MIDI (rtpMIDI)
- [ ] Implémenter le Pitch Bend pour des effets
- [ ] Ajouter des LEDs pour indiquer l'état BLE
- [ ] Interface web de configuration
- [ ] Mode apprentissage des angles via Bluetooth

## Licence

Même licence que le projet principal.

## Auteur

Version ESP32 BLE adaptée du projet original Arduino Leonardo.

---

Pour toute question ou problème, vérifier d'abord le moniteur série et les messages de debug !
