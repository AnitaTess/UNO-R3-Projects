#include <Servo.h>

Servo goalkeeperServo;

const int servoPin = 4;
const int joystickXPin = A0;

int servoAngle = 90; // starting position, middle

void setup() {
  goalkeeperServo.attach(servoPin);
  goalkeeperServo.write(servoAngle);

  Serial.begin(9600);
}

void loop() {
  int joystickX = analogRead(joystickXPin);

  // Print joystick value so you can see what it is doing
  Serial.print("Joystick X: ");
  Serial.println(joystickX);

  // Joystick centre is usually around 512
  // Left = lower values, right = higher values
  if (joystickX < 400) {
    servoAngle -= 7; // move left
  }
  else if (joystickX > 600) {
    servoAngle += 7; // move right
  }

  servoAngle = constrain(servoAngle, 0, 180);

  goalkeeperServo.write(servoAngle);

  delay(10);
}
