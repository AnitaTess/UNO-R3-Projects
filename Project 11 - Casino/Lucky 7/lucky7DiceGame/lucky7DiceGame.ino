#include "LedControl.h"

// --------------------
// LED Dot Matrix
// DIN = 8, CLK = 10, CS = 9
// --------------------
LedControl lc = LedControl(8, 10, 9, 1);

// --------------------
// LEDs
// --------------------
const int greenLed = 6;
const int redLed = 5;

// --------------------
// Buttons
// --------------------
const int leftButton = 2;    // UNDER 7
const int selectButton = 3;  // EXACT 7
const int rightButton = 4;   // OVER 7

bool lastLeftState = HIGH;
bool lastSelectState = HIGH;
bool lastRightState = HIGH;

unsigned long lastButtonPress = 0;
const unsigned long buttonCooldown = 300;

// --------------------
// Game
// --------------------
int bank = 20;
int bet = 5;
const int winTarget = 50;

bool gameEnded = false;

// Bet types
// 0 = under 7
// 1 = exact 7
// 2 = over 7

// --------------------
// Dice faces
// --------------------

byte dice1[8] = {
  B00000000,
  B00000000,
  B00000000,
  B00011000,
  B00011000,
  B00000000,
  B00000000,
  B00000000
};

byte dice2[8] = {
  B00000000,
  B01100000,
  B01100000,
  B00000000,
  B00000000,
  B00000110,
  B00000110,
  B00000000
};

byte dice3[8] = {
  B00000000,
  B01100000,
  B01100000,
  B00011000,
  B00011000,
  B00000110,
  B00000110,
  B00000000
};

byte dice4[8] = {
  B00000000,
  B01100110,
  B01100110,
  B00000000,
  B00000000,
  B01100110,
  B01100110,
  B00000000
};

byte dice5[8] = {
  B00000000,
  B01100110,
  B01100110,
  B00011000,
  B00011000,
  B01100110,
  B01100110,
  B00000000
};

byte dice6[8] = {
  B01100110,
  B01100110,
  B00000000,
  B01100110,
  B01100110,
  B00000000,
  B01100110,
  B01100110
};

// Simple question mark / waiting icon
byte waitIcon[8] = {
  B00111100,
  B01100110,
  B00000110,
  B00001100,
  B00011000,
  B00000000,
  B00011000,
  B00011000
};

// Win smile
byte winIcon[8] = {
  B00000000,
  B00100100,
  B00100100,
  B00000000,
  B01000010,
  B00111100,
  B00000000,
  B00000000
};

// Lose X
byte loseIcon[8] = {
  B10000001,
  B01000010,
  B00100100,
  B00011000,
  B00011000,
  B00100100,
  B01000010,
  B10000001
};

void setup() {
  Serial.begin(9600);

  lc.shutdown(0, false);
  lc.setIntensity(0, 5);
  lc.clearDisplay(0);

  pinMode(greenLed, OUTPUT);
  pinMode(redLed, OUTPUT);

  pinMode(leftButton, INPUT_PULLUP);
  pinMode(selectButton, INPUT_PULLUP);
  pinMode(rightButton, INPUT_PULLUP);

  digitalWrite(greenLed, LOW);
  digitalWrite(redLed, LOW);

  randomSeed(analogRead(A3));

  showPattern(waitIcon);

  printInstructions();
}

void loop() {
  handleButtons();
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

      if (gameEnded) {
        resetGame();
      } else {
        playRound(0); // under 7
      }
    }

    if (lastSelectState == HIGH && currentSelect == LOW) {
      lastButtonPress = millis();

      if (gameEnded) {
        resetGame();
      } else {
        playRound(1); // exactly 7
      }
    }

    if (lastRightState == HIGH && currentRight == LOW) {
      lastButtonPress = millis();

      if (gameEnded) {
        resetGame();
      } else {
        playRound(2); // over 7
      }
    }
  }

  lastLeftState = currentLeft;
  lastSelectState = currentSelect;
  lastRightState = currentRight;
}

// --------------------
// Game logic
// --------------------

void playRound(int betType) {
  if (bank < bet) {
    gameOver();
    return;
  }

  digitalWrite(greenLed, LOW);
  digitalWrite(redLed, LOW);

  Serial.println();
  Serial.println("--------------------");

  Serial.print("Bank before bet: £");
  Serial.println(bank);

  Serial.print("Bet: £");
  Serial.println(bet);

  Serial.print("Player chose: ");
  printBetType(betType);
  Serial.println();

  bank -= bet;

  int diceA = rollDiceWithAnimation();
  delay(700);

  int diceB = rollDiceWithAnimation();
  delay(700);

  int total = diceA + diceB;

  Serial.print("Dice 1: ");
  Serial.println(diceA);

  Serial.print("Dice 2: ");
  Serial.println(diceB);

  Serial.print("Total: ");
  Serial.println(total);

  bool won = false;
  int payout = 0;

  if (betType == 0 && total < 7) {
    won = true;
    payout = bet * 2;
  }
  else if (betType == 1 && total == 7) {
    won = true;
    payout = bet * 5;
  }
  else if (betType == 2 && total > 7) {
    won = true;
    payout = bet * 2;
  }

  if (won) {
    bank += payout;

    digitalWrite(greenLed, HIGH);
    digitalWrite(redLed, LOW);

    showPattern(winIcon);

    Serial.print("WIN! Payout: £");
    Serial.println(payout);
  } else {
    digitalWrite(redLed, HIGH);
    digitalWrite(greenLed, LOW);

    showPattern(loseIcon);

    Serial.println("LOSS! No payout.");
  }

  Serial.print("Bank now: £");
  Serial.println(bank);

  if (bank <= 0) {
    gameOver();
    return;
  }

  if (bank >= winTarget) {
    gameWon();
    return;
  }

  delay(2000);

  digitalWrite(greenLed, LOW);
  digitalWrite(redLed, LOW);
  showPattern(waitIcon);

  Serial.println();
  Serial.println("Next round:");
  Serial.println("LEFT = UNDER 7");
  Serial.println("MIDDLE = EXACT 7");
  Serial.println("RIGHT = OVER 7");
}

void gameOver() {
  gameEnded = true;
  bank = 0;

  digitalWrite(redLed, HIGH);
  digitalWrite(greenLed, LOW);

  showPattern(loseIcon);

  Serial.println();
  Serial.println("GAME OVER! You are bankrupt.");
  Serial.println("Press any button to restart.");
}

void gameWon() {
  gameEnded = true;

  digitalWrite(greenLed, HIGH);
  digitalWrite(redLed, LOW);

  showPattern(winIcon);

  Serial.println();
  Serial.println("YOU WON THE GAME!");
  Serial.print("Final bank: £");
  Serial.println(bank);
  Serial.println("Press any button to restart.");
}

void resetGame() {
  bank = 20;
  gameEnded = false;

  digitalWrite(greenLed, LOW);
  digitalWrite(redLed, LOW);

  showPattern(waitIcon);

  Serial.println();
  Serial.println("Game restarted!");
  printInstructions();
}

void printBetType(int betType) {
  if (betType == 0) {
    Serial.print("UNDER 7");
  }
  else if (betType == 1) {
    Serial.print("EXACT 7");
  }
  else if (betType == 2) {
    Serial.print("OVER 7");
  }
}

void printInstructions() {
  Serial.println();
  Serial.println("LUCKY 7 DICE GAME");
  Serial.println("-----------------");
  Serial.print("Starting bank: £");
  Serial.println(bank);
  Serial.print("Bet per round: £");
  Serial.println(bet);
  Serial.println();
  Serial.println("LEFT button   = UNDER 7");
  Serial.println("MIDDLE button = EXACT 7");
  Serial.println("RIGHT button  = OVER 7");
  Serial.println();
  Serial.println("UNDER/OVER correct = bet x2");
  Serial.println("EXACT 7 correct    = bet x5");
  Serial.println("Reach £50 to win.");
}

// --------------------
// Dice animation/display
// --------------------

int rollDiceWithAnimation() {
  int finalRoll = random(1, 7);

  for (int i = 0; i < 14; i++) {
    int fakeRoll = random(1, 7);
    showDice(fakeRoll);
    delay(70 + i * 12);
  }

  showDice(finalRoll);

  return finalRoll;
}

void showDice(int number) {
  if (number == 1) showPattern(dice1);
  else if (number == 2) showPattern(dice2);
  else if (number == 3) showPattern(dice3);
  else if (number == 4) showPattern(dice4);
  else if (number == 5) showPattern(dice5);
  else if (number == 6) showPattern(dice6);
}

void showPattern(byte pattern[8]) {
  lc.clearDisplay(0);

  for (int row = 0; row < 8; row++) {
    lc.setRow(0, row, pattern[row]);
  }
}