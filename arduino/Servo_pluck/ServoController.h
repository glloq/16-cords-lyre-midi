#ifndef SERVOCONTROLLER_H
#define SERVOCONTROLLER_H
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include "settings.h"

class ServoController {
private:
  Adafruit_PWMServoDriver pwm;
  int16_t currentPositions[NUM_SERVOS];  // Tableau pour stocker le dernier mouvement 0 => -20° , 1=>+20° (pour 20° de jeu de chaque coté)
  void setServoAngle(uint8_t servoNum, uint16_t angle);
  void resetServosPosition();// utilisé au demarrage pour deplacer les servos en position init-angle grattage puis en position init pour initiliser les position du tableau a 0

public:
  ServoController(); //initialise toutles servomoteurs et le tableau
  void mute(uint8_t servoNum); // met le servo a l'angle d'initilisation contre la corde 
  void pluck(uint8_t servoNum);// actionne le servo pour gratter la corde et alterne le gratage en utilisant le tableau currentPositions pour le sens de grattage 
};

#endif // SERVOCONTROLLER_H

