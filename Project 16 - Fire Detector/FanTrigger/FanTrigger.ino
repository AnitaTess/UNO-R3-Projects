// Flame sensor
const int flameDigitalPin = 2;

// Red warning LED
const int redLedPin = 13;

// Motor driver pins
const int motorEnablePin = 5; // PWM speed control
const int motorIn1 = 4;
const int motorIn2 = 3;

// Most flame sensors output LOW when flame is detected.
// If yours works backwards, change this to HIGH.
const int FLAME_DETECTED = LOW;

// Motor speed: 0-255
const int fanSpeed = 220;

void setup() {
  Serial.begin(9600);

  pinMode(flameDigitalPin, INPUT);

  pinMode(redLedPin, OUTPUT);

  pinMode(motorEnablePin, OUTPUT);
  pinMode(motorIn1, OUTPUT);
  pinMode(motorIn2, OUTPUT);

  stopFan();

  Serial.println("Flame sensor fan project started");
  Serial.println("Fan turns on when flame is detected.");
  Serial.println();
}

void loop() {
  int flameState = digitalRead(flameDigitalPin);

  if (flameState == FLAME_DETECTED) {
    Serial.println("FIRE detected! Fan ON");

    digitalWrite(redLedPin, HIGH);
    startFan();
  } else {
    Serial.println("No fire. Fan OFF");

    digitalWrite(redLedPin, LOW);
    stopFan();
  }

  delay(200);
}

void startFan() {
  // Motor direction
  digitalWrite(motorIn1, HIGH);
  digitalWrite(motorIn2, LOW);

  // Motor speed
  analogWrite(motorEnablePin, fanSpeed);
}

void stopFan() {
  analogWrite(motorEnablePin, 0);

  digitalWrite(motorIn1, LOW);
  digitalWrite(motorIn2, LOW);
}
