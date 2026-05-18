#include "pitches.h"
#include "LedControl.h"

// --------------------
// Pins
// --------------------

const int buzzerPin = 2;

// Buttons from left to right
const int buttonPins[8] = {3, 4, 5, 6, 7, 8, 9, 10};

// Notes from left to right: C D E F G A B C
const int notes[8] = {
  NOTE_C5,
  NOTE_D5,
  NOTE_E5,
  NOTE_F5,
  NOTE_G5,
  NOTE_A5,
  NOTE_B5,
  NOTE_C6
};

// MAX7219 pins
// DIN -> A3
// CLK -> A4
// CS  -> A5
LedControl lc = LedControl(A3, A4, A5, 1);

// --------------------
// Letter patterns
// --------------------

byte letterC[8] = {
  B00111100,
  B01100110,
  B01100000,
  B01100000,
  B01100000,
  B01100110,
  B00111100,
  B00000000
};

byte letterD[8] = {
  B01111000,
  B01101100,
  B01100110,
  B01100110,
  B01100110,
  B01101100,
  B01111000,
  B00000000
};

byte letterE[8] = {
  B01111110,
  B01100000,
  B01100000,
  B01111100,
  B01100000,
  B01100000,
  B01111110,
  B00000000
};

byte letterF[8] = {
  B01111110,
  B01100000,
  B01100000,
  B01111100,
  B01100000,
  B01100000,
  B01100000,
  B00000000
};

byte letterG[8] = {
  B00111100,
  B01100110,
  B01100000,
  B01101110,
  B01100110,
  B01100110,
  B00111100,
  B00000000
};

byte letterA[8] = {
  B00011000,
  B00111100,
  B01100110,
  B01100110,
  B01111110,
  B01100110,
  B01100110,
  B00000000
};

byte letterB[8] = {
  B01111100,
  B01100110,
  B01100110,
  B01111100,
  B01100110,
  B01100110,
  B01111100,
  B00000000
};

byte blank[8] = {
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000
};

// --------------------
// Display helpers
// --------------------

void showPattern(byte pattern[8]) {
  lc.clearDisplay(0);

  for (int row = 0; row < 8; row++) {
    lc.setRow(0, row, pattern[row]);
  }
}

void showNoteLetter(int noteIndex) {
  if (noteIndex == 0 || noteIndex == 7) {
    showPattern(letterC);
  }
  else if (noteIndex == 1) {
    showPattern(letterD);
  }
  else if (noteIndex == 2) {
    showPattern(letterE);
  }
  else if (noteIndex == 3) {
    showPattern(letterF);
  }
  else if (noteIndex == 4) {
    showPattern(letterG);
  }
  else if (noteIndex == 5) {
    showPattern(letterA);
  }
  else if (noteIndex == 6) {
    showPattern(letterB);
  }
}

void setup() {
  Serial.begin(9600);

  pinMode(buzzerPin, OUTPUT);

  for (int i = 0; i < 8; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  lc.shutdown(0, false);
  lc.setIntensity(0, 8);
  lc.clearDisplay(0);

  Serial.println("Mini Piano Free Play Mode Ready");
}

void loop() {
  bool anyButtonPressed = false;

  for (int i = 0; i < 8; i++) {
    if (digitalRead(buttonPins[i]) == LOW) {
      anyButtonPressed = true;

      tone(buzzerPin, notes[i]);
      showNoteLetter(i);

      Serial.print("Playing note: ");

      if (i == 0 || i == 7) Serial.println("C");
      else if (i == 1) Serial.println("D");
      else if (i == 2) Serial.println("E");
      else if (i == 3) Serial.println("F");
      else if (i == 4) Serial.println("G");
      else if (i == 5) Serial.println("A");
      else if (i == 6) Serial.println("B");

      break; // only play one note at a time
    }
  }

  if (!anyButtonPressed) {
    noTone(buzzerPin);
    showPattern(blank);
  }
}
