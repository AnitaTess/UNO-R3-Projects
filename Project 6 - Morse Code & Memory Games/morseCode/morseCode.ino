// Morse Code Number Game
// Enter 4-digit code using Morse buttons
// Dot = white, Dash = blue, Accept = green, Clear = red

// --------------------
// 74HC595 display pins
// --------------------
int data = 8;
int latch = 9;
int clock = 10;

// Digit select pins: left to right
int digitPins[4] = {2, 11, 12, 13};

const int DIGIT_ON = LOW;
const int DIGIT_OFF = HIGH;

// Corrected segment table from your wiring
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
// Button pins
// --------------------
const int dotButton = 3;      // white button
const int dashButton = 4;     // blue button
const int enterButton = 6;    // green button
const int clearButton = 7;    // red button

// --------------------
// LED pins
// --------------------
const int dotLed = A0;        // white LED
const int dashLed = A1;       // blue LED
const int enterLed = A3;      // green LED
const int clearLed = A4;      // red LED

// --------------------
// Morse digit codes
// --------------------
String morseDigits[10] = {
  "-----", // 0
  ".----", // 1
  "..---", // 2
  "...--", // 3
  "....-", // 4
  ".....", // 5
  "-....", // 6
  "--...", // 7
  "---..", // 8
  "----."  // 9
};

// --------------------
// Game state
// --------------------
String currentMorse = "";

int enteredDigits[4] = {10, 10, 10, 10}; // 10 = blank
int enteredCount = 0;

int targetDigits[4];

bool gameComplete = false;

// --------------------
// Display helpers
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
  for (int i = 0; i < 4; i++) {
    allDigitsOff();

    int digit = enteredDigits[i];

    if (digit >= 0 && digit <= 9) {
      sendSegments(table[digit]);
    } else {
      sendSegments(table[10]); // blank
    }

    digitalWrite(digitPins[i], DIGIT_ON);
    delay(2);
  }

  allDigitsOff();
}

void gameDelay(unsigned long ms) {
  unsigned long startTime = millis();

  while (millis() - startTime < ms) {
    refreshDisplay();
  }
}

// --------------------
// LED helpers
// --------------------

void blinkLed(int ledPin, int duration) {
  digitalWrite(ledPin, HIGH);
  gameDelay(duration);
  digitalWrite(ledPin, LOW);
}

void blinkAllLeds(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(dotLed, HIGH);
    digitalWrite(dashLed, HIGH);
    digitalWrite(enterLed, HIGH);
    digitalWrite(clearLed, HIGH);

    gameDelay(200);

    digitalWrite(dotLed, LOW);
    digitalWrite(dashLed, LOW);
    digitalWrite(enterLed, LOW);
    digitalWrite(clearLed, LOW);

    gameDelay(200);
  }
}

// --------------------
// Button helpers
// --------------------

bool buttonPressed(int pin) {
  if (digitalRead(pin) == LOW) {
    gameDelay(25);

    if (digitalRead(pin) == LOW) {
      while (digitalRead(pin) == LOW) {
        refreshDisplay();
      }

      return true;
    }
  }

  return false;
}

// --------------------
// Morse helpers
// --------------------

int morseToDigit(String code) {
  for (int i = 0; i < 10; i++) {
    if (code == morseDigits[i]) {
      return i;
    }
  }

  return -1; // invalid Morse sequence
}

void printTargetCode() {
  Serial.print("Target code: ");

  for (int i = 0; i < 4; i++) {
    Serial.print(targetDigits[i]);
  }

  Serial.println();
  Serial.println("Enter the code using Morse:");
  Serial.println("Dot = white, Dash = blue, Enter = green, Clear = red");
}

void generateTargetCode() {
  for (int i = 0; i < 4; i++) {
    targetDigits[i] = random(0, 10);
  }

  printTargetCode();
}

void resetEntryOnly() {
  currentMorse = "";
  enteredCount = 0;
  gameComplete = false;

  for (int i = 0; i < 4; i++) {
    enteredDigits[i] = 10; // blank
  }
}

void newGame() {
  resetEntryOnly();
  generateTargetCode();
}

void clearCurrentGame() {
  Serial.println("Cleared. Restarting current attempt.");

  resetEntryOnly();
  blinkLed(clearLed, 300);

  printTargetCode();
}

void checkFullCode() {
  bool correct = true;

  for (int i = 0; i < 4; i++) {
    if (enteredDigits[i] != targetDigits[i]) {
      correct = false;
    }
  }

  if (correct) {
    Serial.println("Correct code! You win!");
    blinkAllLeds(4);

    Serial.println("New code generated.");
    gameDelay(1000);
    newGame();
  } else {
    Serial.println("Wrong code! Try again.");
    blinkLed(clearLed, 700);

    resetEntryOnly();
    printTargetCode();
  }
}

void acceptMorseDigit() {
  if (currentMorse.length() == 0) {
    Serial.println("No Morse entered yet.");
    blinkLed(clearLed, 200);
    return;
  }

  int digit = morseToDigit(currentMorse);

  if (digit == -1) {
    Serial.print("Invalid Morse: ");
    Serial.println(currentMorse);

    currentMorse = "";
    blinkLed(clearLed, 500);
    return;
  }

  if (enteredCount < 4) {
    enteredDigits[enteredCount] = digit;
    enteredCount++;

    Serial.print("Accepted digit: ");
    Serial.println(digit);

    blinkLed(enterLed, 250);
  }

  currentMorse = "";

  if (enteredCount >= 4) {
    checkFullCode();
  }
}

// --------------------
// Setup and loop
// --------------------

void setup() {
  Serial.begin(9600);

  pinMode(data, OUTPUT);
  pinMode(latch, OUTPUT);
  pinMode(clock, OUTPUT);

  for (int i = 0; i < 4; i++) {
    pinMode(digitPins[i], OUTPUT);
    digitalWrite(digitPins[i], DIGIT_OFF);
  }

  pinMode(dotButton, INPUT_PULLUP);
  pinMode(dashButton, INPUT_PULLUP);
  pinMode(enterButton, INPUT_PULLUP);
  pinMode(clearButton, INPUT_PULLUP);

  pinMode(dotLed, OUTPUT);
  pinMode(dashLed, OUTPUT);
  pinMode(enterLed, OUTPUT);
  pinMode(clearLed, OUTPUT);

  randomSeed(analogRead(A5));

  Serial.println("Morse Code Display Game");
  Serial.println("-----------------------");

  newGame();
}

void loop() {
  refreshDisplay();

  if (buttonPressed(dotButton)) {
    if (currentMorse.length() < 5) {
      currentMorse += ".";
      Serial.print("Current Morse: ");
      Serial.println(currentMorse);
      blinkLed(dotLed, 120);
    }
  }

  if (buttonPressed(dashButton)) {
    if (currentMorse.length() < 5) {
      currentMorse += "-";
      Serial.print("Current Morse: ");
      Serial.println(currentMorse);
      blinkLed(dashLed, 220);
    }
  }

  if (buttonPressed(enterButton)) {
    acceptMorseDigit();
  }

  if (buttonPressed(clearButton)) {
    clearCurrentGame();
  }
}