#include "ServoController.h"
#include "settings.h"

ServoController::ServoController() {
  pwm = Adafruit_PWMServoDriver(); 
  if(pwm.begin()==false){
    Serial.println("DEBUG :pca i2c not found");
  }
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(SERVO_FREQUENCY);
  resetServosPosition();
}

void ServoController::setServoAngle(uint8_t servoNum, uint16_t angle) {
  // Adaptation de l'angle en plage de pulsations pour Adafruit_ServoDriver
  uint16_t pulsation = map(angle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE, SERVO_PULSE_MIN, SERVO_PULSE_MAX);
  int analog_value = int(float(pulsation) / 1000000 * SERVO_FREQUENCY * 4096);
  pwm.setPWM(servoNum, 0, analog_value);
}

void ServoController::resetServosPosition() {
  // Utilisé au démarrage pour déplacer les servos en position initiale de grattage
    bool isEven;
  for (uint8_t i = 0; i < NUM_SERVOS; ++i) {
    isEven = (i % 2 == 0);
    if (isEven) {
      setServoAngle(i, initialAngles[i] + PLUCK_ANGLE);  // Pour les servomoteurs pairs, inverse le sens de rotation à +20°
    } else {
      setServoAngle(i, initialAngles[i] - PLUCK_ANGLE);  // Pour les servomoteurs impairs, utilise -20°
    }
    //setServoAngle(i, initialAngles[i] - PLUCK_ANGLE);
	  delay(500); // délai pour laisser les servos se déplacer
    if (DEBUG) {
      Serial.print("DEBUG :reset ouverture servo");
      Serial.println(i);
    } 
  }
  // Puis en position initiale pour initialiser les positions du tableau à 0
  for (uint8_t i = 0; i < NUM_SERVOS; ++i) {
    setServoAngle(i, initialAngles[i]);
	  delay(100); //  délai pour laisser les servos se déplacer
    if (DEBUG) {
    Serial.print("DEBUG :reset fermeture servo");
    Serial.println(i);  } 
  }
  if (DEBUG) {
    Serial.println("DEBUG : servoController--init done");
  } 
}

void ServoController::mute(uint8_t servoNum) {
  setServoAngle(servoNum, initialAngles[servoNum]);
}

//gratte la corde
void ServoController::pluck(uint8_t servoNum) {
  // Actionne le servomoteur pour gratter la corde en fonction du dernier mouvement (position du pick)
  bool isEvenServo = (servoNum % 2 == 0);
  
  if (currentPositions[servoNum] == 1) {
    if (isEvenServo) {
      setServoAngle(servoNum, initialAngles[servoNum] + PLUCK_ANGLE);  // Pour les servomoteurs pairs, inverse le sens de rotation à +20°
    } else {
      setServoAngle(servoNum, initialAngles[servoNum] - PLUCK_ANGLE);  // Pour les servomoteurs impairs, utilise -20°
    }
   // setServoAngle(servoNum, initialAngles[servoNum] - PLUCK_ANGLE);  // Pick contre la corde à -20°
	  currentPositions[servoNum] = 0;
  } else {
if (isEvenServo) {
      setServoAngle(servoNum, initialAngles[servoNum] - PLUCK_ANGLE);  // Pour les servomoteurs pairs, inverse le sens de rotation à -20°
    } else {
      setServoAngle(servoNum, initialAngles[servoNum] + PLUCK_ANGLE);  // Pour les servomoteurs impairs, utilise +20°
    }	  currentPositions[servoNum] = 1;
  } 
  if (DEBUG) {
    Serial.print("DEBUG : pluck servo");
    Serial.println(servoNum);
  } 
}