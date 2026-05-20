#include <Stepper.h>
#include "SR04.h"

// Ultrasonic sensor
#define TRIG_PIN 3
#define ECHO_PIN 4

SR04 sr04 = SR04(ECHO_PIN, TRIG_PIN);

// Stepper motor
const int stepsPerRevolution = 2048;
const int halfCircleSteps = stepsPerRevolution / 2; // 1024 steps = 180 degrees

const int rolePerMinute = 12;

// Stepper pins from your setup
Stepper myStepper(stepsPerRevolution, 8, 10, 9, 11);

// Radar settings
int currentStep = 0;
int direction = 1;

const int stepSize = 8;       // bigger = faster sweep, smaller = smoother
const int maxDistance = 35;   // cm, adjust for your table

void setup() {
  Serial.begin(9600);

  myStepper.setSpeed(rolePerMinute);

  delay(1000);

  Serial.println("Stepper radar ready");
}

void loop() {
  // Move stepper a small amount
  myStepper.step(direction * stepSize);
  currentStep += direction * stepSize;

  // Keep sweep inside 0-180 degrees
  if (currentStep >= halfCircleSteps) {
    currentStep = halfCircleSteps;
    direction = -1;
  }

  if (currentStep <= 0) {
    currentStep = 0;
    direction = 1;
  }

  // Convert current step position into angle
  int angle = map(currentStep, 0, halfCircleSteps, 0, 180);

  // Read distance
  long distance = sr04.Distance();

  // Clean weird readings
  if (distance <= 0 || distance > maxDistance) {
    distance = maxDistance + 1; // out of range
  }

  // Send data to Processing
  Serial.print(angle);
  Serial.print(",");
  Serial.println(distance);

  delay(50);
}
