#include "pitches.h"
#include "LedControl.h"

// --------------------
// Pins
// --------------------

const int buzzerPin = 2;

const int greenLedPin = 12;
const int redLedPin = 13;

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
// Game melodies
// These numbers refer to button/note indexes:
// 0 = C, 1 = D, 2 = E, 3 = F, 4 = G, 5 = A, 6 = B, 7 = high C
// --------------------

const int level1Melody[] = {0, 1, 2};                 // C D E
const int level2Melody[] = {0, 2, 4, 2, 0};           // C E G E C
const int level3Melody[] = {0, 1, 2, 4, 5, 4, 2, 0};  // C D E G A G E C

int currentLevel = 1;
int playerPosition = 0;

bool waitingForPlayer = false;

// --------------------
// Matrix icons
// --------------------

byte number1[8] = {
  B00011000,
  B00111000,
  B00011000,
  B00011000,
  B00011000,
  B00011000,
  B01111110,
  B00000000
};

byte number2[8] = {
  B00111100,
  B01100110,
  B00000110,
  B00001100,
  B00110000,
  B01100000,
  B01111110,
  B00000000
};

byte number3[8] = {
  B00111100,
  B01100110,
  B00000110,
  B00011100,
  B00000110,
  B01100110,
  B00111100,
  B00000000
};

byte heart[8] = {
  B00000000,
  B01100110,
  B11111111,
  B11111111,
  B11111111,
  B01111110,
  B00111100,
  B00011000
};

byte winIcon[8] = {
  B10000001,
  B01000010,
  B00100100,
  B00011000,
  B00011000,
  B00100100,
  B01000010,
  B10000001
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

void showLevelNumber() {
  if (currentLevel == 1) {
    showPattern(number1);
  }
  else if (currentLevel == 2) {
    showPattern(number2);
  }
  else if (currentLevel == 3) {
    showPattern(number3);
  }
}

void showHeart() {
  showPattern(heart);
}

void showWinIcon() {
  showPattern(winIcon);
}

// --------------------
// Melody helpers
// --------------------

int getLevelLength() {
  if (currentLevel == 1) return 3;
  if (currentLevel == 2) return 5;
  return 8;
}

int getExpectedNoteIndex(int position) {
  if (currentLevel == 1) {
    return level1Melody[position];
  }
  else if (currentLevel == 2) {
    return level2Melody[position];
  }
  else {
    return level3Melody[position];
  }
}

void playNoteByIndex(int noteIndex) {
  tone(buzzerPin, notes[noteIndex], 350);
  delay(400);
  noTone(buzzerPin);
}

void playCurrentLevelMelody() {
  showLevelNumber();

  delay(700);

  int melodyLength = getLevelLength();

  for (int i = 0; i < melodyLength; i++) {
    int noteIndex = getExpectedNoteIndex(i);
    playNoteByIndex(noteIndex);
    delay(200);
  }

  playerPosition = 0;
  waitingForPlayer = true;

  Serial.print("Your turn! Level ");
  Serial.println(currentLevel);
}

// --------------------
// Feedback
// --------------------

void correctFeedback() {
  digitalWrite(greenLedPin, HIGH);
  delay(500);
  digitalWrite(greenLedPin, LOW);
}

void wrongFeedback() {
  digitalWrite(redLedPin, HIGH);

  // Error sound
  tone(buzzerPin, NOTE_C3, 300);
  delay(350);
  noTone(buzzerPin);

  digitalWrite(redLedPin, LOW);
}

void levelWon() {
  waitingForPlayer = false;

  digitalWrite(greenLedPin, HIGH);
  showHeart();

  tone(buzzerPin, NOTE_C5, 150);
  delay(180);
  tone(buzzerPin, NOTE_E5, 150);
  delay(180);
  tone(buzzerPin, NOTE_G5, 150);
  delay(180);
  tone(buzzerPin, NOTE_C6, 400);
  delay(500);
  noTone(buzzerPin);

  delay(2500);

  digitalWrite(greenLedPin, LOW);

  currentLevel++;

  if (currentLevel > 3) {
    gameWon();
  } else {
    showLevelNumber();
    delay(1000);
    playCurrentLevelMelody();
  }
}

void gameWon() {
  waitingForPlayer = false;

  showHeart();

  for (int i = 0; i < 3; i++) {
    digitalWrite(greenLedPin, HIGH);
    tone(buzzerPin, NOTE_C6, 150);
    delay(200);
    digitalWrite(greenLedPin, LOW);

    tone(buzzerPin, NOTE_G5, 150);
    delay(200);
    noTone(buzzerPin);
  }

  delay(1000);

  // Restart game
  currentLevel = 1;
  showLevelNumber();
  delay(1500);
  playCurrentLevelMelody();
}

// --------------------
// Button handling
// --------------------

int getPressedButton() {
  for (int i = 0; i < 8; i++) {
    if (digitalRead(buttonPins[i]) == LOW) {
      delay(30); // debounce

      if (digitalRead(buttonPins[i]) == LOW) {
        // Wait until button is released
        while (digitalRead(buttonPins[i]) == LOW) {
          delay(10);
        }

        return i;
      }
    }
  }

  return -1;
}

void handlePlayerInput(int pressedNote) {
  int expectedNote = getExpectedNoteIndex(playerPosition);

  Serial.print("Pressed note index: ");
  Serial.print(pressedNote);
  Serial.print(" | Expected: ");
  Serial.println(expectedNote);

  playNoteByIndex(pressedNote);

  if (pressedNote == expectedNote) {
    playerPosition++;

    if (playerPosition >= getLevelLength()) {
      levelWon();
    }
  }
  else {
    wrongFeedback();

    Serial.println("Wrong note! Restarting this level...");

    playerPosition = 0;
    waitingForPlayer = false;

    delay(1000);
    playCurrentLevelMelody();
  }
}

// --------------------
// Setup and loop
// --------------------

void setup() {
  Serial.begin(9600);

  pinMode(buzzerPin, OUTPUT);

  pinMode(greenLedPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);

  digitalWrite(greenLedPin, LOW);
  digitalWrite(redLedPin, LOW);

  for (int i = 0; i < 8; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  lc.shutdown(0, false);
  lc.setIntensity(0, 8);
  lc.clearDisplay(0);

  Serial.println("Mini Piano Memory Game");

  showLevelNumber();
  delay(1000);

  playCurrentLevelMelody();
}

void loop() {
  if (waitingForPlayer) {
    int pressedButton = getPressedButton();

    if (pressedButton != -1) {
      handlePlayerInput(pressedButton);
    }
  }
}
