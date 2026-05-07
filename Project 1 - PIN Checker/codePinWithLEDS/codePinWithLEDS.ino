//www.elegoo.com
//2016.12.9

#include <Keypad.h>

const byte ROWS = 4;
const byte COLS = 4;

char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

const int greenLedPin = 11;
const int redLedPin = 12;

const String correctCode = "1608";
String enteredCode = "";

void setup() {
  Serial.begin(9600);

  pinMode(greenLedPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);

  digitalWrite(greenLedPin, LOW);
  digitalWrite(redLedPin, LOW);

  Serial.println("Enter 4-digit code, then press #");
}

void loop() {
  char customKey = customKeypad.getKey();

  if (customKey) {
    Serial.print("Pressed: ");
    Serial.println(customKey);

    if (customKey >= '0' && customKey <= '9') {
      // Only allow up to 4 digits
      if (enteredCode.length() < 4) {
        enteredCode += customKey;
        Serial.print("Current code: ");
        Serial.println(enteredCode);
      }
    }

    else if (customKey == '#') {
      Serial.print("Checking code: ");
      Serial.println(enteredCode);

      if (enteredCode == correctCode) {
        Serial.println("Correct code!");

        digitalWrite(greenLedPin, HIGH);
        delay(2000);
        digitalWrite(greenLedPin, LOW);
      } 
      else {
        Serial.println("Wrong code!");

        digitalWrite(redLedPin, HIGH);
        delay(2000);
        digitalWrite(redLedPin, LOW);
      }

      // Clear code after checking
      enteredCode = "";
      Serial.println("Enter code again, then press #");
    }

    else if (customKey == '*') {
      // Use * as clear/reset
      enteredCode = "";
      Serial.println("Code cleared");
    }
  }
}
