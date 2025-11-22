# ESP32 BLE MIDI - Version Test Interactive

**Contrôle total via Serial Monitor - AUCUN matériel requis**

---

## 🎯 Objectif

Version de test pour **valider la communication BLE MIDI** sans servos, boutons ou LED.
Tout se contrôle et se visualise dans le **Serial Monitor**.

---

## ✅ Fonctionnalités

### Réception MIDI
- ✅ Affiche tous les messages MIDI reçus (détaillés)
- ✅ Note On/Off avec note, vélocité, canal
- ✅ Control Change avec numéro et valeur
- ✅ SysEx avec affichage hexadécimal
- ✅ Réponse automatique à Identity Request

### Envoi MIDI (via commandes Serial)
- ✅ Envoyer Note On/Off
- ✅ Envoyer Control Change
- ✅ Envoyer SysEx Identity Reply
- ✅ Feedback MIDI configurable
- ✅ Heartbeat automatique

### Surveillance
- ✅ État de connexion en temps réel
- ✅ Statistiques complètes
- ✅ Compteurs de messages
- ✅ Durée de connexion

---

## 🚀 Installation

### 1. Upload
```
Fichier: ESP32_Test_Interactive.ino
Carte: ESP32 Dev Module
Compiler et uploader
```

### 2. Ouvrir Serial Monitor
```
Vitesse: 115200 bauds
Fin de ligne: Retour chariot (NL)
```

### 3. Connexion

Vous verrez:
```
╔════════════════════════════════════════════════════╗
║  ESP32 BLE MIDI - VERSION TEST INTERACTIVE        ║
║  Contrôle complet via Serial Monitor              ║
╚════════════════════════════════════════════════════╝
  Firmware: v1.0

[BLE] Initialisation...
[BLE] ✓ Service MIDI actif
[BLE] Nom: Lyre-Test
═══════════════════════════════════════════════════════

📱 Cherchez 'Lyre-Test' dans votre app MIDI BLE
```

---

## 📝 Commandes Serial Monitor

### Envoi de messages MIDI

| Commande | Description | Exemple |
|----------|-------------|---------|
| `n<note> <vel>` | Envoyer Note On | `n60 100` |
| `o<note>` | Envoyer Note Off | `o60` |
| `c<cc> <val>` | Envoyer Control Change | `c7 127` |
| `i` | Envoyer SysEx Identity Reply | `i` |

### Information

| Commande | Description |
|----------|-------------|
| `s` | Afficher statistiques |
| `h` | Afficher aide |

### Configuration

| Commande | Description |
|----------|-------------|
| `f` | Toggle Feedback MIDI ON/OFF |
| `t` | Toggle Auto Heartbeat ON/OFF |

---

## 💡 Exemples d'utilisation

### Test 1 : Connexion BLE

1. **Uploader le code**
2. **Ouvrir Serial Monitor**
3. **Connecter depuis app MIDI BLE**

Vous verrez:
```
╔═══════════════════════════════════╗
║   ✓✓✓ CONNEXION ETABLIE ✓✓✓      ║
╚═══════════════════════════════════╝

📤 Envoyé: Connection Status = CONNECTED
📤 Envoyé: SysEx Identity Reply
   F0 7E 7F 06 02 7D 00 01 00 01 01 00 00 00 F7
```

✅ **Connexion validée !**

---

### Test 2 : Recevoir une note MIDI

Envoyez **Note On 60, velocity 100** depuis votre app.

Vous verrez:
```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
📥 REÇU: Note On
   Note: 60 (0x3C)
   Velocity: 100
   Canal: 0
   → Servo jouerait cette note
📤 Envoyé: Note On 60, vel 100, ch 0
   ✓ Feedback envoyé
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

✅ **Réception validée !**
✅ **Feedback validé !**

---

### Test 3 : Envoyer une note depuis Serial

Tapez dans Serial Monitor:
```
n60 100
```

Vous verrez:
```
> n60 100
📤 Envoyé: Note On 60, vel 100, ch 0
```

**ET votre app MIDI devrait recevoir la note !**

✅ **Envoi validé !**

---

### Test 4 : SysEx Identity Request

**Depuis votre app**, envoyez:
```
F0 7E 7F 06 01 F7
```

Vous verrez:
```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
📥 REÇU: System / SysEx
   Data: F0 7E 00 06 01 F7
   → Identity Request détecté!
   → Envoi Identity Reply...
📤 Envoyé: SysEx Identity Reply
   F0 7E 7F 06 02 7D 00 01 00 01 01 00 00 00 F7
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

**ET votre app devrait recevoir l'Identity Reply !**

✅ **SysEx validé !**

---

### Test 5 : Statistiques

Tapez dans Serial Monitor:
```
s
```

Vous verrez:
```
╔════════════════════ STATISTIQUES ═════════════════════╗
║ Connecté:          OUI ✓
║ Durée connexion:   125 s (2 min)
║ ─────────────────────────────────────────────────────── ║
║ Messages reçus:    15
║ Messages envoyés:  18
║ ─────────────────────────────────────────────────────── ║
║ Note On reçus:     8
║ Note Off reçus:    7
║ CC reçus:          0
║ SysEx reçus:       1
║ ─────────────────────────────────────────────────────── ║
║ Feedback MIDI:     ACTIVÉ ✓
║ Auto Heartbeat:    ACTIVÉ ✓
╚════════════════════════════════════════════════════════╝
```

---

### Test 6 : Désactiver Feedback

Tapez:
```
f
```

Résultat:
```
🔄 Feedback MIDI: DÉSACTIVÉ
```

Maintenant, les notes reçues **ne seront plus renvoyées**.

Pour réactiver, tapez à nouveau `f`.

---

## 🎯 Scénarios de test

### ✅ Test complet de validation

1. **Upload et connexion**
   ```
   - Upload code
   - Ouvrir Serial Monitor (115200)
   - Connecter app MIDI BLE à "Lyre-Test"
   - Vérifier message "CONNEXION ETABLIE"
   ```

2. **Test réception**
   ```
   - Envoyer Note On 60, vel 100 depuis app
   - Vérifier affichage dans Serial Monitor
   - Vérifier feedback reçu dans app
   ```

3. **Test envoi**
   ```
   - Taper: n60 100
   - Vérifier réception dans app
   ```

4. **Test SysEx**
   ```
   - Envoyer Identity Request depuis app
   - Vérifier Identity Reply reçu dans app
   - Vérifier affichage dans Serial Monitor
   ```

5. **Test statistiques**
   ```
   - Taper: s
   - Vérifier compteurs cohérents
   ```

6. **Test heartbeat**
   ```
   - Attendre 30 secondes
   - Vérifier "💓 Envoyé: Heartbeat" dans Serial
   - Vérifier réception CC 103 = 127 dans app
   ```

✅ **Si tous les tests passent = communication BLE MIDI 100% fonctionnelle !**

---

## 📊 Format des messages affichés

### Message reçu (exemple)
```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
📥 REÇU: Note On
   Note: 60 (0x3C)
   Velocity: 100
   Canal: 0
   → Servo jouerait cette note
📤 Envoyé: Note On 60, vel 100, ch 0
   ✓ Feedback envoyé
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

### Message envoyé (exemple)
```
> n60 100
📤 Envoyé: Note On 60, vel 100, ch 0
```

---

## ⚙️ Configuration

### Modifier le nom Bluetooth

Ligne 20:
```cpp
#define BLE_DEVICE_NAME "Lyre-Test"
```

### Désactiver feedback par défaut

Ligne 43:
```cpp
bool feedbackEnabled = false;  // false = pas de feedback
```

### Désactiver heartbeat par défaut

Ligne 44:
```cpp
bool autoHeartbeat = false;  // false = pas de heartbeat
```

---

## 🔧 Dépannage

### Pas de message à la connexion

```
- Vérifier baudrate Serial Monitor = 115200
- Vérifier "Fin de ligne" = NL ou Both NL & CR
- Appuyer sur bouton RST de l'ESP32
```

### Commandes ne fonctionnent pas

```
- Vérifier format exact (ex: n60 100, pas n 60 100)
- Vérifier connexion BLE établie
- Voir si message "❌ Impossible d'envoyer: pas connecté"
```

### App ne reçoit pas les messages

```
- Vérifier que l'app écoute les messages entrants
- Taper 's' pour vérifier que messages sont bien envoyés
- Vérifier compteur "Messages envoyés"
```

---

## 📈 Comparaison versions

| Fonctionnalité | Test Interactive | Pro | Standard |
|----------------|------------------|-----|----------|
| **Contrôle Serial** | ✅ Complet | ❌ | ❌ |
| **Affichage détaillé** | ✅ | ❌ | ❌ |
| **Envoi manuel MIDI** | ✅ | ❌ | ❌ |
| **Statistiques** | ✅ | ❌ | ❌ |
| **Servos requis** | ❌ | ✅ | ✅ |
| **SysEx Identity** | ✅ | ✅ | ❌ |
| **Feedback MIDI** | ✅ | ✅ | ❌ |
| **Heartbeat** | ✅ | ✅ | ❌ |

**👉 Version Test Interactive = Parfaite pour développement et validation**

---

## 🎓 Utilisation pédagogique

Cette version est **idéale pour apprendre** le protocole MIDI :

1. **Voir exactement** ce qui est reçu/envoyé
2. **Comprendre** le format des messages
3. **Tester** différents scénarios
4. **Valider** votre implémentation
5. **Débugger** sans matériel

---

## ✨ Prochaines étapes

Une fois la communication validée avec cette version :

1. **Version Pro** : Ajouter les servos (ESP32_Lyre_BLE_Pro)
2. **Personnaliser** : Adapter mapping MIDI ↔ servos
3. **Optimiser** : Ajuster latence et réactivité
4. **Produire** : Déployer en production

---

**Version** : 1.0 Test Interactive
**Matériel requis** : ESP32 uniquement (aucun accessoire)
**Idéal pour** : Développement, validation, apprentissage

🎵 **Testez et validez votre communication BLE MIDI !**
