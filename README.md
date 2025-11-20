> [!NOTE]
> Projet fonctionnel et fini

# 16-cords-lyre-midi

Le projet utilise une lyre de 16 cordes avec 16 servomoteurs qui viennent actionner la corde voulue en fonction des messages midi.
  ![photo lyre](https://raw.githubusercontent.com/glloq/16-cords-lyre-midi/main/lyre.jpg?raw=true)

# liste materiel

- arduino leonardo
- tmc9685
- 16 servomoteurs
- les pieces à imprimer en 3D (dans le dossier stl)
- un support en bois pour poser l'intrument et cacher l'electronique
- une alimentation 5v 8A
- une prise d'alimentation 230v avec fusible
- un passe cloison usb

> [!TIP]
> Une version ESP32 avec Bluetooth BLE est disponible dans le dossier `arduino/Servo_pluck_ESP32_BLE/` !

# Versions disponibles

## Version Arduino Leonardo (USB MIDI)
- **Dossier**: `arduino/Servo_pluck/`
- **Communication**: MIDI via câble USB
- **Carte**: Arduino Leonardo
- **Avantages**: Simple, connexion stable, pas de configuration Bluetooth
- **Utilisation**: Idéale pour un setup fixe connecté à un ordinateur

## Version ESP32 (Bluetooth BLE MIDI)
- **Dossier**: `arduino/Servo_pluck_ESP32_BLE/`
- **Communication**: MIDI via Bluetooth Low Energy (sans fil)
- **Carte**: ESP32 (tous modèles compatibles)
- **Avantages**: Sans fil, connexion mobile (iOS/Android), portée ~10m
- **Utilisation**: Parfaite pour un setup mobile ou contrôle depuis smartphone/tablette
- **Documentation complète**: Voir `arduino/Servo_pluck_ESP32_BLE/README.md`
