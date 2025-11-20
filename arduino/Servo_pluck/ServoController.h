#ifndef SERVOCONTROLLER_H
#define SERVOCONTROLLER_H
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include "settings.h"

class ServoController {
private:
  Adafruit_PWMServoDriver pwm;
  int16_t currentPositions[NUM_SERVOS];  // Tableau pour stocker le dernier mouvement 0 => -20° , 1=>+20°
  void setServoAngle(uint8_t servoNum, uint16_t angle);
  void resetServosPosition();  // Utilise au demarrage pour deplacer les servos en position init

  // Variables pour l'initialisation non-bloquante
  enum InitState { INIT_IDLE, INIT_OPENING, INIT_WAIT_OPENING, INIT_CLOSING, INIT_WAIT_CLOSING, INIT_COMPLETE };
  InitState initState;
  uint8_t initServoIndex;
  unsigned long initLastTime;

public:
  ServoController();  // Initialise tous les servomoteurs et le tableau
  void update();  // A appeler dans loop() pour gerer l'initialisation non-bloquante
  bool isInitComplete();  // Retourne true quand l'initialisation est terminee
  void mute(uint8_t servoNum);  // Met le servo a l'angle d'initialisation contre la corde
  void pluck(uint8_t servoNum);  // Actionne le servo pour gratter la corde
};

#endif // SERVOCONTROLLER_H

