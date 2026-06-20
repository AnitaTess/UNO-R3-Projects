#include <LiquidCrystal.h>

// LCD pins: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

// IR sensor pins
const int startSensorPin = 2;   // left sensor
const int endSensorPin = 13;    // right sensor

// Most IR obstacle sensors output LOW when object is detected.
// If yours works backwards, change this to HIGH.
const int DETECTED_STATE = LOW;

// Distance between sensors in metres
const float distanceMetres = 0.14;

bool measuring = false;
unsigned long startTime = 0;
unsigned long endTime = 0;

bool startWasTriggered = false;
bool endWasTriggered = false;

float lastSpeedKmh = 0.0;

void setup() {
  Serial.begin(9600);

  pinMode(startSensorPin, INPUT);
  pinMode(endSensorPin, INPUT);

  lcd.begin(16, 2);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Speed Trap");
  lcd.setCursor(0, 1);
  lcd.print("Ready");

  Serial.println("=== IR Speed Trap ===");
  Serial.println("Trigger left sensor first, then right sensor.");
  Serial.println("Distance: 14 cm");
  Serial.println();
}

void loop() {
  bool startTriggered = digitalRead(startSensorPin) == DETECTED_STATE;
  bool endTriggered = digitalRead(endSensorPin) == DETECTED_STATE;

  // Start measurement on NEW trigger of left sensor
  if (startTriggered && !startWasTriggered) {
    startMeasurement();
  }

  // End measurement on NEW trigger of right sensor
  if (measuring && endTriggered && !endWasTriggered) {
    finishMeasurement();
  }

  startWasTriggered = startTriggered;
  endWasTriggered = endTriggered;
}

void startMeasurement() {
  measuring = true;
  startTime = micros();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Measuring...");
  lcd.setCursor(0, 1);
  lcd.print("Go!");

  Serial.println("Start sensor triggered. Measuring...");
}

void finishMeasurement() {
  endTime = micros();
  measuring = false;

  unsigned long elapsedMicros = endTime - startTime;
  float elapsedSeconds = elapsedMicros / 1000000.0;

  float speedMetresPerSecond = distanceMetres / elapsedSeconds;
  float speedKmh = speedMetresPerSecond * 3.6;

  lastSpeedKmh = speedKmh;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Speed:");

  lcd.setCursor(0, 1);
  lcd.print(speedKmh, 2);
  lcd.print(" km/h");

  Serial.print("Time: ");
  Serial.print(elapsedSeconds, 4);
  Serial.println(" s");

  Serial.print("Speed: ");
  Serial.print(speedKmh, 2);
  Serial.println(" km/h");

  Serial.println();
}