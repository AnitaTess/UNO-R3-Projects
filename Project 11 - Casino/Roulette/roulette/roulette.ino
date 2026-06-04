#include <Servo.h>

// --------------------
// Servo
// --------------------
Servo rouletteServo;
const int servoPin = 7;

int currentAngle = 90;

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
// Buttons
// --------------------
const int leftButton = A0;    // down / red
const int selectButton = A1;  // select
const int rightButton = A2;   // up / black

bool lastLeftState = HIGH;
bool lastSelectState = HIGH;
bool lastRightState = HIGH;

unsigned long lastButtonPress = 0;
const unsigned long buttonCooldown = 250;

// --------------------
// Game
// --------------------
int bank = 5;
int spinCost = 1;
const int winAmount = 500;

int selectedNumber = 1;
int selectedColour = 2; // 1 = red, 2 = black

int landedNumber = 0;
int landedColour = 0;

enum GameState {
  PICK_NUMBER,
  PICK_COLOUR,
  READY_TO_SPIN,
  SPINNING,
  RESULT,
  GAME_OVER,
  GAME_WON
};

GameState state = PICK_NUMBER;

unsigned long resultStartedAt = 0;

void setup() {
  Serial.begin(9600);

  rouletteServo.attach(servoPin);
  rouletteServo.write(90);

  pinMode(data, OUTPUT);
  pinMode(latch, OUTPUT);
  pinMode(clock, OUTPUT);

  for (int i = 0; i < 4; i++) {
    pinMode(digitPins[i], OUTPUT);
    digitalWrite(digitPins[i], DIGIT_OFF);
  }

  pinMode(leftButton, INPUT_PULLUP);
  pinMode(selectButton, INPUT_PULLUP);
  pinMode(rightButton, INPUT_PULLUP);

  randomSeed(analogRead(A3));

  Serial.println("Servo Roulette Game");
  Serial.println("-------------------");
  Serial.println("Money is shown on the 4-digit display.");
  Serial.println("Left/Right = choose number");
  Serial.println("Select = confirm");
  Serial.println("Then Left = RED, Right = BLACK");
  Serial.println("Select = spin");
  Serial.println();

  showPickNumber();
}

void loop() {
  refreshMoneyDisplay();
  handleButtons();

  if (state == RESULT && millis() - resultStartedAt > 3000) {
    if (bank <= 0) {
      showGameOver();
    }
    else if (bank >= winAmount) {
      showGameWon();
    }
    else {
      showPickNumber();
    }
  }
}

// --------------------
// Button handling
// --------------------

void handleButtons() {
  bool currentLeft = digitalRead(leftButton);
  bool currentSelect = digitalRead(selectButton);
  bool currentRight = digitalRead(rightButton);

  if (millis() - lastButtonPress > buttonCooldown) {
    if (lastLeftState == HIGH && currentLeft == LOW) {
      lastButtonPress = millis();
      handleLeftPress();
    }

    if (lastRightState == HIGH && currentRight == LOW) {
      lastButtonPress = millis();
      handleRightPress();
    }

    if (lastSelectState == HIGH && currentSelect == LOW) {
      lastButtonPress = millis();
      handleSelectPress();
    }
  }

  lastLeftState = currentLeft;
  lastSelectState = currentSelect;
  lastRightState = currentRight;
}

void handleLeftPress() {
  if (state == PICK_NUMBER) {
    selectedNumber--;
    if (selectedNumber < 1) selectedNumber = 9;
    showPickNumber();
  }
  else if (state == PICK_COLOUR) {
    selectedColour = 1; // red
    showPickColour();
  }
}

void handleRightPress() {
  if (state == PICK_NUMBER) {
    selectedNumber++;
    if (selectedNumber > 9) selectedNumber = 1;
    showPickNumber();
  }
  else if (state == PICK_COLOUR) {
    selectedColour = 2; // black
    showPickColour();
  }
}

void handleSelectPress() {
  if (state == GAME_OVER || state == GAME_WON) {
    resetGame();
    return;
  }

  if (state == PICK_NUMBER) {
    state = PICK_COLOUR;
    showPickColour();
  }
  else if (state == PICK_COLOUR) {
    state = READY_TO_SPIN;
    showReadyToSpin();
  }
  else if (state == READY_TO_SPIN) {
    startSpin();
  }
}

// --------------------
// Game flow
// --------------------

void showPickNumber() {
  state = PICK_NUMBER;

  Serial.println();
  Serial.print("Pick number: ");
  Serial.println(selectedNumber);
  Serial.print("Bank: ");
  Serial.println(bank);
  Serial.println("Left/Right to change. Select to confirm.");
}

void showPickColour() {
  state = PICK_COLOUR;

  Serial.println();
  Serial.print("Selected number: ");
  Serial.println(selectedNumber);
  Serial.println("Choose colour:");
  Serial.println("Left = RED");
  Serial.println("Right = BLACK");

  Serial.print("Current colour: ");
  printColourName(selectedColour);
  Serial.println();
}

void showReadyToSpin() {
  state = READY_TO_SPIN;

  Serial.println();
  Serial.print("Bet selected: ");
  Serial.print(selectedNumber);
  Serial.print(" ");
  printColourName(selectedColour);
  Serial.println();

  Serial.print("Spin cost: ");
  Serial.println(spinCost);

  Serial.print("Bank: ");
  Serial.println(bank);

  Serial.println("Press SELECT to spin.");
}

void startSpin() {
  if (bank < spinCost) {
    bank = 0;
    showGameOver();
    return;
  }

  bank -= spinCost;

  state = SPINNING;

  Serial.println();
  Serial.println("Spinning...");

  // random stop only at full 10-degree positions
  int stopIndex = random(0, 19); // 0-18
  int stopAngle = stopIndex * 10;

  // Do a dramatic sweep first
  int sweeps = random(2, 4);

  for (int s = 0; s < sweeps; s++) {
    moveServoSlowly(180, 8);
    moveServoSlowly(0, 8);
  }

  moveServoSlowly(stopAngle, 12);

  currentAngle = stopAngle;

  decodeAngle(stopAngle);

  Serial.print("Stopped at angle: ");
  Serial.println(stopAngle);

  Serial.print("Landed on: ");
  Serial.print(landedNumber);
  Serial.print(" ");
  printColourName(landedColour);
  Serial.println();

  evaluateResult();
}

void evaluateResult() {
  int payout = 0;

  if (landedNumber == 0) {
  int zeroOutcome = random(0, 2); // 0 or 1

  if (zeroOutcome == 0) {
    payout = 500;
    Serial.println("0 GREEN = lucky jackpot +500!");
  } 
  else {
    bank = 0;
    Serial.println("0 GREEN = unlucky bankrupt!");
    
    state = RESULT;
    resultStartedAt = millis();
    return;
  }
}
  else {
    bool exactNumber = landedNumber == selectedNumber;
    bool sameColour = landedColour == selectedColour;
    bool sameParity = (landedNumber % 2) == (selectedNumber % 2);

    if (exactNumber && sameColour) {
      payout = 250;
    }
    else if (exactNumber && !sameColour) {
      payout = 50;
    }
    else if (sameColour && sameParity) {
      payout = 20;
    }
    else if (sameColour && !sameParity) {
      payout = 10;
    }
    else if (!sameColour && sameParity) {
      payout = 5;
    }
    else {
      payout = 0;
    }
  }

  bank += payout;

  Serial.print("Payout: +");
  Serial.println(payout);

  Serial.print("Bank: ");
  Serial.println(bank);

  spinCost++;

  Serial.print("Next spin cost: ");
  Serial.println(spinCost);

  state = RESULT;
  resultStartedAt = millis();
}

void showGameOver() {
  state = GAME_OVER;
  bank = 0;

  Serial.println();
  Serial.println("GAME OVER");
  Serial.println("Bank reached 0.");
  Serial.println("Press SELECT to restart.");
}

void showGameWon() {
  state = GAME_WON;

  Serial.println();
  Serial.println("YOU WIN!");
  Serial.print("Final bank: ");
  Serial.println(bank);
  Serial.println("Press SELECT to restart.");
}

void resetGame() {
  bank = 5;
  spinCost = 1;
  selectedNumber = 1;
  selectedColour = 2;

  rouletteServo.write(90);
  currentAngle = 90;

  Serial.println();
  Serial.println("Game reset.");

  showPickNumber();
}

// --------------------
// Angle decoding
// --------------------
void decodeAngle(int angle) {
  if (angle == 90) {
    landedNumber = 0;
    landedColour = 0; // green
    return;
  }

  if (angle < 90) {
    // LEFT side:
    // 80 = 1 black
    // 70 = 2 red
    // 60 = 3 black
    // ...
    // 0 = 9 black
    landedNumber = (90 - angle) / 10;

    if (landedNumber % 2 == 1) {
      landedColour = 2; // black
    } else {
      landedColour = 1; // red
    }
  }
  else {
    // RIGHT side:
    // 100 = 1 red
    // 110 = 2 black
    // 120 = 3 red
    // ...
    // 180 = 9 red
    landedNumber = (angle - 90) / 10;

    if (landedNumber % 2 == 1) {
      landedColour = 1; // red
    } else {
      landedColour = 2; // black
    }
  }
}

void printColourName(int colour) {
  if (colour == 0) Serial.print("GREEN");
  else if (colour == 1) Serial.print("RED");
  else if (colour == 2) Serial.print("BLACK");
}

// --------------------
// Servo movement
// --------------------

void moveServoSlowly(int targetAngle, int stepDelay) {
  int startAngle = currentAngle;

  if (targetAngle > startAngle) {
    for (int pos = startAngle; pos <= targetAngle; pos++) {
      rouletteServo.write(pos);
      currentAngle = pos;
      refreshMoneyDisplay();
      delay(stepDelay);
    }
  }
  else {
    for (int pos = startAngle; pos >= targetAngle; pos--) {
      rouletteServo.write(pos);
      currentAngle = pos;
      refreshMoneyDisplay();
      delay(stepDelay);
    }
  }
}

// --------------------
// 4-digit money display
// --------------------

void refreshMoneyDisplay() {
  int value;

  if (state == PICK_NUMBER || state == PICK_COLOUR || state == READY_TO_SPIN) {
    value = selectedNumber;   // show chosen number while picking
  } else {
    value = bank;             // show money during/after spin
  }

  if (value < 0) value = 0;
  if (value > 9999) value = 9999;

  int digits[4];

  digits[0] = (value / 1000) % 10;
  digits[1] = (value / 100) % 10;
  digits[2] = (value / 10) % 10;
  digits[3] = value % 10;

  for (int i = 0; i < 4; i++) {
    allDigitsOff();

    byte pattern;

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