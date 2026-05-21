// Simon-style LED memory game
// 5 buttons + 5 LEDs + 4-digit 7-segment display via 74HC595

// --------------------
// 74HC595 display pins
// --------------------
int data = 8;
int latch = 9;
int clock = 10;

// Digit select pins: left to right
int digitPins[4] = {2, 11, 12, 13};

// If your display does not light correctly, swap these:
const int DIGIT_ON = LOW;
const int DIGIT_OFF = HIGH;

// Your corrected segment table
// Your mapping:
// bit 0 = A
// bit 1 = B
// bit 2 = G
// bit 3 = D
// bit 4 = E
// bit 5 = F
// bit 6 = C
// bit 7 = decimal point
unsigned char table[] = {
  0x7B, // 0
  0x42, // 1
  0x1F, // 2
  0x4F, // 3
  0x66, // 4
  0x6D, // 5
  0x7D, // 6
  0x43, // 7
  0x7F, // 8
  0x6F, // 9
  0x00  // blank
};

// --------------------
// Game pins
// --------------------

// Buttons left to right: 7, 6, 5, 4, 3
const int buttonPins[5] = {3, 4, 5, 6, 7};

// LEDs left to right: A0, A1, A2, A3, A4
const int ledPins[5] = {A0, A1, A2, A3, A4};

// --------------------
// Game settings
// --------------------

const int maxSequenceLength = 50;
int sequence[maxSequenceLength];

int sequenceLength = 0;
int score = 0;

int flashTime = 450;
int pauseTime = 200;

// --------------------
// Display functions
// --------------------

void allDigitsOff() {
  for (int i = 0; i < 4; i++) {
    digitalWrite(digitPins[i], DIGIT_OFF);
  }
}

void sendSegments(byte value) {
  digitalWrite(latch, LOW);
  shiftOut(data, clock, MSBFIRST, value);
  digitalWrite(latch, HIGH);
}

void refreshDisplay() {
  int digits[4];

  digits[0] = (score / 1000) % 10;
  digits[1] = (score / 100) % 10;
  digits[2] = (score / 10) % 10;
  digits[3] = score % 10;

  for (int i = 0; i < 4; i++) {
    allDigitsOff();
    sendSegments(table[digits[i]]);
    digitalWrite(digitPins[i], DIGIT_ON);
    delay(2);
  }

  allDigitsOff();
}

// Use this instead of normal delay so the display stays alive
void gameDelay(unsigned long ms) {
  unsigned long startTime = millis();

  while (millis() - startTime < ms) {
    refreshDisplay();
  }
}

// --------------------
// LED helpers
// --------------------

void allLedsOff() {
  for (int i = 0; i < 5; i++) {
    digitalWrite(ledPins[i], LOW);
  }
}

void flashLed(int ledIndex, int duration) {
  digitalWrite(ledPins[ledIndex], HIGH);
  gameDelay(duration);
  digitalWrite(ledPins[ledIndex], LOW);
  gameDelay(pauseTime);
}

void flashAllLeds(int times) {
  for (int t = 0; t < times; t++) {
    for (int i = 0; i < 5; i++) {
      digitalWrite(ledPins[i], HIGH);
    }

    gameDelay(200);

    for (int i = 0; i < 5; i++) {
      digitalWrite(ledPins[i], LOW);
    }

    gameDelay(200);
  }
}

// --------------------
// Button helper
// --------------------

int waitForButtonPress() {
  while (true) {
    refreshDisplay();

    for (int i = 0; i < 5; i++) {
      if (digitalRead(buttonPins[i]) == LOW) {
        gameDelay(30); // debounce

        if (digitalRead(buttonPins[i]) == LOW) {
          while (digitalRead(buttonPins[i]) == LOW) {
            refreshDisplay();
          }

          return i;
        }
      }
    }
  }
}

// --------------------
// Game logic
// --------------------

void addRandomLedToSequence() {
  if (sequenceLength < maxSequenceLength) {
    sequence[sequenceLength] = random(0, 5);
    sequenceLength++;
  }
}

void playSequence() {
  gameDelay(600);

  for (int i = 0; i < sequenceLength; i++) {
    flashLed(sequence[i], flashTime);
  }
}

bool playerRepeatsSequenceCorrectly() {
  for (int i = 0; i < sequenceLength; i++) {
    int pressedButton = waitForButtonPress();

    // Show the LED they pressed as feedback
    flashLed(pressedButton, 150);

    if (pressedButton != sequence[i]) {
      return false;
    }
  }

  return true;
}

void resetGame() {
  sequenceLength = 0;
  score = 0;
  flashTime = 450;

  allLedsOff();
  flashAllLeds(2);

  gameDelay(800);

  addRandomLedToSequence();
}

// --------------------
// Setup and loop
// --------------------

void setup() {
  pinMode(data, OUTPUT);
  pinMode(latch, OUTPUT);
  pinMode(clock, OUTPUT);

  for (int i = 0; i < 4; i++) {
    pinMode(digitPins[i], OUTPUT);
    digitalWrite(digitPins[i], DIGIT_OFF);
  }

  for (int i = 0; i < 5; i++) {
    pinMode(ledPins[i], OUTPUT);
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  allLedsOff();

  randomSeed(analogRead(A5));

  score = 0;
  gameDelay(1000);

  resetGame();
}

void loop() {
  refreshDisplay();

  playSequence();

  bool correct = playerRepeatsSequenceCorrectly();

  if (correct) {
    score++;

    flashAllLeds(1);

    addRandomLedToSequence();

    if (flashTime > 180) {
      flashTime -= 10;
    }

    gameDelay(700);
  } 
  else {
    flashAllLeds(4);
    resetGame();
  }
}