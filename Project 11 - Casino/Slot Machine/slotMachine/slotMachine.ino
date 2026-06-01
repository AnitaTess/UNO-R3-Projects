#include <LiquidCrystal.h>

// LCD pins
LiquidCrystal lcd(5, 6, 7, 11, 12, 13);

// --------------------
// 74HC595 display pins
// --------------------
int data = 8;
int latch = 9;
int clock = 10;

int digitPins[4] = {2, 3, 4, A5};

const int DIGIT_ON = LOW;
const int DIGIT_OFF = HIGH;

unsigned char table[] = {
  0x3F, // 0
  0x06, // 1
  0x5B, // 2
  0x4F, // 3
  0x66, // 4
  0x6D, // 5
  0x7D, // 6
  0x07, // 7
  0x7F, // 8
  0x6F, // 9
  0x00  // blank
};

// --------------------
// Button + LEDs
// --------------------
const int buttonPin = A0;

const int redLed = A1;
const int greenLed = A2;

// --------------------
// Game values
// --------------------
int bank = 5;
int spinCost = 1;

int slots[3] = {0, 0, 0};
bool slotStopped[3] = {false, false, false};
int stoppedCount = 0;

unsigned long lastSpinUpdate = 0;
const unsigned long spinSpeed = 120;

bool waitingToStart = true;
bool spinning = false;
bool showingResult = false;
bool gameOver = false;
bool gameWon = false;

unsigned long resultStartedAt = 0;

// Button debounce
bool lastButtonState = HIGH;
unsigned long lastButtonPress = 0;
const unsigned long buttonCooldown = 250;

// Slot types
// 0 = 7
// 1 = star
// 2 = cherry
// 3 = bell

// --------------------
// Custom LCD characters
// --------------------

// Star
byte starChar[8] = {
  B00100,
  B10101,
  B01110,
  B11111,
  B01110,
  B10101,
  B00100,
  B00000
};

// Cherry
byte cherryChar[8] = {
  B00010,
  B00100,
  B01110,
  B10101,
  B10101,
  B01110,
  B00000,
  B00000
};

// Bell
byte bellChar[8] = {
  B00100,
  B01110,
  B01110,
  B11111,
  B11111,
  B00100,
  B01110,
  B00000
};

void delayWithDisplay(unsigned long ms) {
  unsigned long start = millis();

  while (millis() - start < ms) {
    refreshMoneyDisplay();
  }
}

void setup() {
  lcd.begin(16, 2);

  lcd.createChar(0, starChar);
  lcd.createChar(1, cherryChar);
  lcd.createChar(2, bellChar);

  pinMode(data, OUTPUT);
  pinMode(latch, OUTPUT);
  pinMode(clock, OUTPUT);

  for (int i = 0; i < 4; i++) {
    pinMode(digitPins[i], OUTPUT);
    digitalWrite(digitPins[i], DIGIT_OFF);
  }

  pinMode(buttonPin, INPUT_PULLUP);

  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);

  digitalWrite(redLed, LOW);
  digitalWrite(greenLed, LOW);

  randomSeed(analogRead(A4));

  showStartScreen();
}

void loop() {
  refreshMoneyDisplay();

  handleButton();

  if (spinning) {
    updateRollingSlots();
  }

  if (showingResult && millis() - resultStartedAt > 2200) {
    showingResult = false;
    digitalWrite(redLed, LOW);
    digitalWrite(greenLed, LOW);

    if (bank <= 0) {
      showGameOver();
    }
    else if (bank >= 500) {
      showGameWon();
    }
    else {
      showStartScreen();
    }
  }
}

// --------------------
// Main game logic
// --------------------

void showStartScreen() {
  waitingToStart = true;
  spinning = false;
  gameOver = false;
  gameWon = false;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Bank:");
  lcd.print(bank);
  lcd.print(" Cost:");
  lcd.print(spinCost);

  lcd.setCursor(0, 1);
  lcd.print("Press to spin");
}

void startSpin() {
  if (bank < spinCost) {
    bank = 0;
    showGameOver();
    return;
  }

  bank -= spinCost;

  waitingToStart = false;
  spinning = true;
  showingResult = false;

  stoppedCount = 0;

  for (int i = 0; i < 3; i++) {
    slotStopped[i] = false;
    slots[i] = random(0, 4);
  }

  digitalWrite(redLed, LOW);
  digitalWrite(greenLed, LOW);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Stop each slot");

  drawSlots();
}

void stopNextSlot() {
  if (!spinning) return;

  if (stoppedCount < 3) {
    slotStopped[stoppedCount] = true;
    stoppedCount++;
    drawSlots();
  }

 if (stoppedCount >= 3) {
  spinning = false;

  lcd.setCursor(0, 0);
  lcd.print("Final result!  ");

  delayWithDisplay(2000); // show stopped slots for 2 seconds

  evaluateSpin();
}
}

void updateRollingSlots() {
  if (millis() - lastSpinUpdate < spinSpeed) {
    return;
  }

  lastSpinUpdate = millis();

  for (int i = 0; i < 3; i++) {
    if (!slotStopped[i]) {
      slots[i] = random(0, 4);
    }
  }

  drawSlots();
}

void evaluateSpin() {
  int payout = calculatePayout();

  bank += payout;

  lcd.clear();

  if (payout > 0) {
    digitalWrite(greenLed, HIGH);
    digitalWrite(redLed, LOW);

    lcd.setCursor(0, 0);
    lcd.print("WIN +");
    lcd.print(payout);
  } else {
    digitalWrite(redLed, HIGH);
    digitalWrite(greenLed, LOW);

    lcd.setCursor(0, 0);
    lcd.print("No win!");
  }

  lcd.setCursor(0, 1);
  lcd.print("Bank:");
  lcd.print(bank);
  lcd.print(" Next:");
  lcd.print(spinCost + 1);

  spinCost++;

  showingResult = true;
  resultStartedAt = millis();
}

int calculatePayout() {
  int a = slots[0];
  int b = slots[1];
  int c = slots[2];

  // Three matching
  if (a == b && b == c) {
    return payoutForThree(a);
  }

  // Two matching
  if (a == b) {
    return payoutForTwo(a);
  }

  if (a == c) {
    return payoutForTwo(a);
  }

  if (b == c) {
    return payoutForTwo(b);
  }

  return 0;
}

int payoutForThree(int symbol) {
  if (symbol == 3) return 25;   // bells
  if (symbol == 2) return 50;   // cherries
  if (symbol == 1) return 100;  // stars
  if (symbol == 0) return 250;  // sevens

  return 0;
}

int payoutForTwo(int symbol) {
  if (symbol == 3) return 5;    // bells
  if (symbol == 2) return 10;   // cherries
  if (symbol == 1) return 20;   // stars
  if (symbol == 0) return 35;   // sevens

  return 0;
}

// --------------------
// LCD slot drawing
// --------------------

void drawSlots() {
  lcd.setCursor(0, 1);
  lcd.print("   [");

  drawSlotSymbol(slots[0]);

  lcd.print("] [");

  drawSlotSymbol(slots[1]);

  lcd.print("] [");

  drawSlotSymbol(slots[2]);

  lcd.print("]  ");
}

void drawSlotSymbol(int symbol) {
  if (symbol == 0) {
    lcd.print("7");
  }
  else if (symbol == 1) {
    lcd.write(byte(0)); // star
  }
  else if (symbol == 2) {
    lcd.write(byte(1)); // cherry
  }
  else if (symbol == 3) {
    lcd.write(byte(2)); // bell
  }
}

// --------------------
// Button handling
// --------------------

void handleButton() {
  bool currentButtonState = digitalRead(buttonPin);

  if (
    lastButtonState == HIGH &&
    currentButtonState == LOW &&
    millis() - lastButtonPress > buttonCooldown
  ) {
    lastButtonPress = millis();

    if (gameOver || gameWon) {
      resetGame();
    }
    else if (waitingToStart && !showingResult) {
      startSpin();
    }
    else if (spinning) {
      stopNextSlot();
    }
  }

  lastButtonState = currentButtonState;
}

// --------------------
// Game over / win
// --------------------

void showGameOver() {
  gameOver = true;
  waitingToStart = false;
  spinning = false;
  showingResult = false;

  digitalWrite(redLed, HIGH);
  digitalWrite(greenLed, LOW);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("GAME OVER");
  lcd.setCursor(0, 1);
  lcd.print("Press restart");
}

void showGameWon() {
  gameWon = true;
  waitingToStart = false;
  spinning = false;
  showingResult = false;

  digitalWrite(greenLed, HIGH);
  digitalWrite(redLed, LOW);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("JACKPOT WIN!");
  lcd.setCursor(0, 1);
  lcd.print("Bank:");
  lcd.print(bank);
}

void resetGame() {
  bank = 5;
  spinCost = 1;

  digitalWrite(redLed, LOW);
  digitalWrite(greenLed, LOW);

  showStartScreen();
}

// --------------------
// 4-digit money display
// --------------------

void refreshMoneyDisplay() {
  int displayValue = bank;

  if (displayValue < 0) {
    displayValue = 0;
  }

  if (displayValue > 9999) {
    displayValue = 9999;
  }

  int digits[4];

  digits[0] = (displayValue / 1000) % 10;
  digits[1] = (displayValue / 100) % 10;
  digits[2] = (displayValue / 10) % 10;
  digits[3] = displayValue % 10;

  for (int i = 0; i < 4; i++) {
    allDigitsOff();

    byte pattern;

    // Blank leading zeroes
    if (i == 0 && digits[0] == 0) {
      pattern = table[10];
    }
    else if (i == 1 && digits[0] == 0 && digits[1] == 0) {
      pattern = table[10];
    }
    else if (i == 2 && digits[0] == 0 && digits[1] == 0 && digits[2] == 0) {
      pattern = table[10];
    }
    else {
      pattern = table[digits[i]];
    }

    sendSegments(pattern);
    digitalWrite(digitPins[i], DIGIT_ON);
    delay(2);
  }

  allDigitsOff();
}

void sendSegments(byte value) {
  digitalWrite(latch, LOW);
  shiftOut(data, clock, MSBFIRST, value);
  digitalWrite(latch, HIGH);
}

void allDigitsOff() {
  for (int i = 0; i < 4; i++) {
    digitalWrite(digitPins[i], DIGIT_OFF);
  }
}
