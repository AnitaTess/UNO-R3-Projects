#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

// RFID pins
#define RST_PIN 9
#define SS_PIN 10

MFRC522 mfrc522(SS_PIN, RST_PIN);

// Servo
Servo doorServo;
const int servoPin = 6;

const int closedAngle = 180;
const int openAngle = 90;

// LEDs
const int greenLedPin = 2;
const int redLedPin = 3;

// Replace this with your real card/fob UID
byte allowedUID[] = {0x25, 0x3A, 0xF9, 0x03};
byte allowedUIDSize = 4;

void setup() {
  Serial.begin(9600);

  SPI.begin();
  mfrc522.PCD_Init();

  doorServo.attach(servoPin);
  doorServo.write(closedAngle);

  pinMode(greenLedPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);

  digitalWrite(greenLedPin, LOW);
  digitalWrite(redLedPin, LOW);

  Serial.println("RFID Door System Ready");
  Serial.println("Scan your card or fob...");
}

void loop() {
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.print("Scanned UID:");

  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(" 0x");

    if (mfrc522.uid.uidByte[i] < 0x10) {
      Serial.print("0");
    }

    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }

  Serial.println();

  if (isAllowedCard()) {
    accessGranted();
  } else {
    accessDenied();
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  delay(1000);
}

bool isAllowedCard() {
  if (mfrc522.uid.size != allowedUIDSize) {
    return false;
  }

  for (byte i = 0; i < allowedUIDSize; i++) {
    if (mfrc522.uid.uidByte[i] != allowedUID[i]) {
      return false;
    }
  }

  return true;
}

void accessGranted() {
  Serial.println("ACCESS GRANTED");

  digitalWrite(redLedPin, LOW);
  digitalWrite(greenLedPin, HIGH);

  moveServoSlowly(openAngle);

  delay(3000); // door stays open for 3 seconds

  moveServoSlowly(closedAngle);

  digitalWrite(greenLedPin, LOW);

  Serial.println("Door closed");
}

void accessDenied() {
  Serial.println("ACCESS DENIED");

  digitalWrite(greenLedPin, LOW);
  digitalWrite(redLedPin, HIGH);

  delay(2000);

  digitalWrite(redLedPin, LOW);
}

void moveServoSlowly(int targetAngle) {
  int currentAngle = doorServo.read();

  if (targetAngle > currentAngle) {
    for (int pos = currentAngle; pos <= targetAngle; pos++) {
      doorServo.write(pos);
      delay(20);
    }
  } 
  else {
    for (int pos = currentAngle; pos >= targetAngle; pos--) {
      doorServo.write(pos);
      delay(20);
    }
  }
}