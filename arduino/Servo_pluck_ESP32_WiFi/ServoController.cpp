#include "ServoController.h"
#include "settings.h"

ServoController::ServoController() {
  // Configurer le pin OE
  pinMode(PIN_SERVO_OE, OUTPUT);
  enableServos();  // Activer les servos

  pwm = Adafruit_PWMServoDriver();
  if(pwm.begin() == false){
    Serial.println("ERREUR CRITIQUE: PCA9685 non detecte sur le bus I2C!");
    Serial.println("Verifiez les connexions et l'adresse I2C.");
    while(1) {  // Bloquer l'execution
      delay(1000);
    }
  }
  pwm.setOscillatorFrequency(PCA9685_OSCILLATOR_FREQ);
  pwm.setPWMFreq(SERVO_FREQUENCY);

  // Initialiser le bitfield currentPositions (tous les bits a 0)
  currentPositions = 0;

  // Demarrer l'initialisation non-bloquante
  initState = INIT_OPENING;
  initServoIndex = 0;
  initLastTime = millis();

  // Initialiser le timeout de desactivation
  lastActivityTime = millis();
  servosEnabled = true;
}

void ServoController::setServoAngle(uint8_t servoNum, uint16_t angle) {
  if (servoNum >= NUM_SERVOS) {
    if (DEBUG) {
      Serial.print("[SERVO] ERREUR: index servo invalide: ");
      Serial.println(servoNum);
    }
    return;
  }

  // Adaptation de l'angle en plage de pulsations pour Adafruit_ServoDriver
  uint16_t pulsation = map(angle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE, SERVO_PULSE_MIN, SERVO_PULSE_MAX);
  int analog_value = int(float(pulsation) / 1000000 * SERVO_FREQUENCY * 4096);
  pwm.setPWM(servoNum, 0, analog_value);
}

void ServoController::update() {
  // Machine a etats pour l'initialisation non-bloquante
  unsigned long currentTime = millis();

  switch (initState) {
    case INIT_OPENING:
      // Deplacer le servo actuel en position d'ouverture
      if (initServoIndex % 2 == 0) {
        setServoAngle(initServoIndex, initialAngles[initServoIndex] + PLUCK_ANGLE);
      } else {
        setServoAngle(initServoIndex, initialAngles[initServoIndex] - PLUCK_ANGLE);
      }
      if (DEBUG) {
        Serial.print("[SERVO] Initialisation servo #");
        Serial.print(initServoIndex);
        Serial.println(" - position ouverture");
      }
      initLastTime = currentTime;
      initState = INIT_WAIT_OPENING;
      break;

    case INIT_WAIT_OPENING:
      // Attendre que le servo se deplace
      if (currentTime - initLastTime >= SERVO_INIT_DELAY_MS) {
        initServoIndex++;
        if (initServoIndex >= NUM_SERVOS) {
          // Tous les servos sont en position d'ouverture, passer a la fermeture
          initServoIndex = 0;
          initState = INIT_CLOSING;
        } else {
          initState = INIT_OPENING;
        }
      }
      break;

    case INIT_CLOSING:
      // Remettre le servo en position de repos
      setServoAngle(initServoIndex, initialAngles[initServoIndex]);
      if (DEBUG) {
        Serial.print("[SERVO] Servo #");
        Serial.print(initServoIndex);
        Serial.println(" - position repos");
      }
      initLastTime = currentTime;
      initState = INIT_WAIT_CLOSING;
      break;

    case INIT_WAIT_CLOSING:
      // Attendre que le servo se deplace
      if (currentTime - initLastTime >= SERVO_RESET_DELAY_MS) {
        initServoIndex++;
        if (initServoIndex >= NUM_SERVOS) {
          // Initialisation complete - desactiver les servos pour economiser l'energie
          disableServos();
          initState = INIT_COMPLETE;
          if (DEBUG) {
            Serial.println("[SERVO] Tous les servos sont initialises");
            Serial.println("[SERVO] Servos desactives (economie d'energie)");
          }
        } else {
          initState = INIT_CLOSING;
        }
      }
      break;

    case INIT_IDLE:
    case INIT_COMPLETE:
      // Gerer la desactivation automatique apres timeout
      if (servosEnabled && (currentTime - lastActivityTime >= SERVO_AUTO_DISABLE_TIMEOUT_MS)) {
        disableServos();  // servosEnabled est deja mis a false dans disableServos()
        if (DEBUG) {
          Serial.println("[SERVO] Timeout - servos desactives");
        }
      }
      break;
  }
}

bool ServoController::isInitComplete() {
  return (initState == INIT_COMPLETE);
}

void ServoController::resetServosPosition() {
  // Methode legacy - plus utilisee avec le systeme non-bloquant
}

void ServoController::mute(uint8_t servoNum) {
  if (servoNum >= NUM_SERVOS) {
    if (DEBUG) {
      Serial.print("[SERVO] ERREUR: index servo invalide: ");
      Serial.println(servoNum);
    }
    return;
  }

  enableServos();  // Activer avant de bouger (reinitialise aussi le timeout)
  setServoAngle(servoNum, initialAngles[servoNum]);
}

void ServoController::enableServos() {
  if (!servosEnabled) {
    digitalWrite(PIN_SERVO_OE, LOW);  // OE actif bas
    servosEnabled = true;
    if (DEBUG) {
      Serial.println("[SERVO] Servos actives");
    }
  }
  lastActivityTime = millis();  // Reinitialiser le timeout
}

void ServoController::disableServos() {
  digitalWrite(PIN_SERVO_OE, HIGH);  // OE inactif haut
  servosEnabled = false;
}

//gratte la corde
void ServoController::pluck(uint8_t servoNum) {
  if (servoNum >= NUM_SERVOS) {
    if (DEBUG) {
      Serial.print("[SERVO] ERREUR: index servo invalide: ");
      Serial.println(servoNum);
    }
    return;
  }

  // Activer les servos avant de jouer
  enableServos();

  // Calcul simplifie de la direction de grattage
  // Position alterne entre 0 et 1, servos pairs/impairs ont des sens opposes
  uint8_t position = (currentPositions >> servoNum) & 1;  // Extraire le bit correspondant
  int8_t direction = (position == 1) ? 1 : -1;
  if ((servoNum % 2) != 0) {
    direction = -direction;  // Inverser pour les servos impairs
  }

  setServoAngle(servoNum, initialAngles[servoNum] + (direction * PLUCK_ANGLE));

  // Toggle la position (0 <-> 1) avec XOR
  currentPositions ^= (1 << servoNum);
  if (DEBUG) {
    Serial.print("[SERVO] Pluck servo #");
    Serial.print(servoNum);
    Serial.print(" - position: ");
    Serial.println((currentPositions >> servoNum) & 1);
  }
}
