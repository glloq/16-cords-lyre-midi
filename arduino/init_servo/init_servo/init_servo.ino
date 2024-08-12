#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

const int SERVO_FREQUENCY = 50;  // Valeur frequence servo
const int SERVO_MIN = 500;  // Valeur PWM minimale
const int SERVO_MAX = 2500;  // Valeur PWM maximale

int currentServo = 0;       // Servomoteur actuel
int servoPosition[16];      // Positions pour chaque servomoteur

void setup() {
  Serial.begin(9600);
  pwm.begin();
  pwm.setPWMFreq(SERVO_FREQUENCY);  // Fréquence PWM
  
  // Initialisation des positions pour chaque servomoteur à 90°

  for (int i = 0; i < 16; i++) {
    servoPosition[i] = 90;
  }
}

void loop() {
  if (Serial.available() > 0) {
    char command = Serial.read();
    if (command == 'P') {
      moveCurrentServo(15);  // Déplace le servomoteur actuel de +15°
      delay(500);
      moveCurrentServo(0);  // Déplace le servomoteur actuel a 0°
      delay(500);
      moveCurrentServo(-15);  // Déplace le servomoteur actuel de -15°
      delay(500);
      moveCurrentServo(0);  // Déplace le servomoteur actuel a 0°

    } else if (command == 'N') {
      currentServo++;        // Passe au servomoteur suivant
      if (currentServo >= 16) {
        currentServo = 0;    // Revenir au début si nous atteignons 16
      }
    } else if (command >= '0' && command <= '180') {
      int newAngle = command ;
      if (newAngle >= 0 && newAngle <= 180) {
        servoPosition[currentServo] = newAngle;
        moveCurrentServo(0);  // Déplace le servomoteur à la position spécifiée
      }
    }
    
    // Si tous les 16 servomoteurs sont initialisés, affichez le tableau des positions
    if (currentServo == 15) {
      printServoPositions();
    }
  }
}

void moveCurrentServo(int angle) {
  int pulse = map(servoPosition[currentServo] + angle, 0, 180, SERVO_MIN, SERVO_MAX);
  pwm.setPWM(currentServo, 0, pulse);
}

void printServoPositions() {
  Serial.println("Tableau des positions pour les servomoteurs : [");
  for (int i = 0; i < 16; i++) {
    Serial.print(servoPosition[i]);
    if(i!=16){
      Serial.print(",");
    }
  }
  Serial.println("]");
}
