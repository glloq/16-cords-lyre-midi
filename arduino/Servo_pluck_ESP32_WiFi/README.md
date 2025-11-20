# Lyre MIDI 16 cordes - Version ESP32 WiFi

Version parallèle du projet de lyre MIDI adaptée pour ESP32 avec MIDI via WiFi (RTP-MIDI/AppleMIDI).

## Différences avec la version Arduino Leonardo

Cette version utilise :
- **ESP32** au lieu d'Arduino Leonardo
- **MIDI via WiFi** (RTP-MIDI/AppleMIDI) au lieu d'USB MIDI
- Connexion sans fil pour plus de flexibilité

## Matériel requis

- **ESP32** (ESP32-WROOM-32, ESP32-DevKit, etc.)
- **PCA9685** (contrôleur de servos I2C)
- **16 servomoteurs SG90**
- Pièces 3D (voir dossier STL du projet principal)
- Support en bois
- **Alimentation 5V 8A** (pour les servos)
- **Alimentation 5V USB** ou batterie pour l'ESP32

## Bibliothèques Arduino requises

Installez les bibliothèques suivantes via le gestionnaire de bibliothèques Arduino :

1. **ESP32** - Support de la carte (via le gestionnaire de cartes)
2. **Adafruit PWM Servo Driver Library** - Pour contrôler le PCA9685
3. **AppleMIDI Library** - Pour RTP-MIDI via WiFi
   - GitHub: https://github.com/lathoub/Arduino-AppleMIDI-Library

## Installation

### 1. Configuration de l'IDE Arduino pour ESP32

Si ce n'est pas déjà fait :
1. Ouvrir Arduino IDE
2. Aller dans **Fichier > Préférences**
3. Ajouter cette URL dans "URL de gestionnaire de cartes supplémentaires" :
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Aller dans **Outils > Type de carte > Gestionnaire de cartes**
5. Rechercher "ESP32" et installer "esp32 by Espressif Systems"

### 2. Configuration WiFi

Modifier le fichier `settings.h` :
```cpp
#define WIFI_SSID "VotreSSID"           // Votre nom de réseau WiFi
#define WIFI_PASSWORD "VotreMotDePasse" // Votre mot de passe WiFi
```

### 3. Connexions matérielles

#### ESP32 vers PCA9685 (I2C)
- **ESP32 GPIO 21 (SDA)** → PCA9685 SDA
- **ESP32 GPIO 22 (SCL)** → PCA9685 SCL
- **ESP32 GND** → PCA9685 GND
- **ESP32 3.3V** → PCA9685 VCC

#### Pin de contrôle OE (économie d'énergie)
- **ESP32 GPIO 5** → PCA9685 OE (Output Enable)

#### PCA9685 vers servos
- **PCA9685 V+** → Alimentation 5V 8A (+)
- **PCA9685 GND** → Alimentation 5V 8A (-) ET ESP32 GND (masse commune)
- Connecter les 16 servos aux sorties 0-15 du PCA9685

⚠️ **IMPORTANT** : Ne pas alimenter les servos via l'ESP32, utiliser une alimentation externe 5V 8A

## Configuration MIDI via WiFi

### macOS
1. Ouvrir **Audio MIDI Setup** (dans Applications/Utilitaires)
2. Aller dans **Fenêtre > Afficher la fenêtre MIDI**
3. Cliquer sur l'icône **MIDI réseau**
4. Rechercher la session **ESP32-Lyre-MIDI**
5. Cliquer sur **Connecter**

### Windows
1. Télécharger et installer **rtpMIDI** : https://www.tobias-erichsen.de/software/rtpmidi.html
2. Lancer rtpMIDI
3. Dans "Directory", rechercher **ESP32-Lyre-MIDI**
4. Cliquer sur **Connect**

### Linux
1. Installer **QmidiNet** : `sudo apt install qmidinet`
2. Lancer QmidiNet
3. Rechercher la session **ESP32-Lyre-MIDI**
4. Se connecter

## Utilisation

1. Téléverser le code sur l'ESP32
2. Ouvrir le moniteur série (115200 baud)
3. Vérifier que l'ESP32 se connecte au WiFi
4. Noter l'adresse IP affichée
5. Connecter via RTP-MIDI (voir ci-dessus)
6. Envoyer des notes MIDI depuis votre DAW ou contrôleur MIDI

## Calibration

Les angles d'initialisation des servos sont définis dans `settings.h` :
```cpp
const uint16_t initialAngles[NUM_SERVOS] = {85, 86, 96, 86, 88, 82, 95, 85, 94, 90, 108, 73, 110, 70, 105, 75};
```

Ajustez ces valeurs pour que chaque servo soit aligné contre sa corde au repos.

## Plage de notes MIDI

- **Note minimum** : G3 (MIDI 55)
- **Note maximum** : A5 (MIDI 81)
- **Notes supportées** : Gamme diatonique de 16 notes

Voir le mapping complet dans `settings.h` (tableau `MidiServoMapping`).

## Debug

Pour activer les messages de debug détaillés, modifier dans `settings.h` :
```cpp
#define DEBUG 1  // 1 = activé, 0 = désactivé
```

## Économie d'énergie

Le système désactive automatiquement les servos après 2 secondes d'inactivité pour économiser l'énergie et réduire la chaleur. Les servos se réactivent automatiquement lors de la réception d'une nouvelle note MIDI.

Vous pouvez ajuster ce délai dans `settings.h` :
```cpp
#define SERVO_AUTO_DISABLE_TIMEOUT_MS 2000  // en millisecondes
```

## Avantages de la version WiFi

- ✅ Sans fil - pas de câble USB nécessaire
- ✅ Portable avec une batterie
- ✅ Compatible avec plusieurs DAW simultanément
- ✅ Latence faible (RTP-MIDI)
- ✅ Portée WiFi étendue

## Dépannage

### L'ESP32 ne se connecte pas au WiFi
- Vérifier le SSID et le mot de passe dans `settings.h`
- S'assurer que le réseau WiFi est en 2.4GHz (ESP32 ne supporte pas le 5GHz)
- Vérifier la portée du signal WiFi

### PCA9685 non détecté
- Vérifier les connexions I2C (SDA/SCL)
- Vérifier l'adresse I2C du PCA9685 (par défaut 0x40)
- S'assurer que VCC et GND sont bien connectés

### Servos qui tremblent ou ne bougent pas
- Vérifier que l'alimentation 5V 8A est bien connectée
- S'assurer que la masse (GND) est commune entre ESP32, PCA9685 et alimentation
- Vérifier les angles dans `initialAngles[]`

### Pas de connexion MIDI
- Vérifier que l'ESP32 et l'ordinateur sont sur le même réseau WiFi
- Relancer le logiciel RTP-MIDI (rtpMIDI/Audio MIDI Setup)
- Vérifier le pare-feu (autoriser les connexions UDP sur les ports 5004-5005)

## Licence

Même licence que le projet principal.
