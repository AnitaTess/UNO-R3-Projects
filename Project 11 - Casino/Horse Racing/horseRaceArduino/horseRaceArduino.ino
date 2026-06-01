// Horse Racing Casino Game
// Arduino controls buttons, LEDs, game logic
// Processing draws the horse race screen

// --------------------
// LED horse pins left to right
// --------------------
const int horseLedPins[5] = {
  13, // Blue
  12, // Yellow
  11, // Green
  10, // White
  9   // Red
};

// --------------------
// Buttons
// --------------------
const int leftButton = 2;
const int selectButton = 3;
const int rightButton = 4;

bool lastLeftState = HIGH;
bool lastSelectState = HIGH;
bool lastRightState = HIGH;

unsigned long lastButtonPress = 0;
const unsigned long buttonCooldown = 250;

// --------------------
// Game state
// --------------------
enum GameState {
  SELECT_BET,
  SELECT_HORSE,
  RACING,
  RESULT,
  GAME_OVER,
  GAME_WON
};

GameState state = SELECT_BET;

int bank = 20;
int betAmount = 5;
int selectedHorse = 0;

const int winTarget = 500;
const int finishLine = 100;

int horsePositions[5] = {0, 0, 0, 0, 0};
bool horseFinished[5] = {false, false, false, false, false};
int finishOrder[5] = {-1, -1, -1, -1, -1};
int finishedCount = 0;

int winnerHorse = -1;
int playerPlace = 0;
int payout = 0;
int profitLoss = 0;

unsigned long lastRaceUpdate = 0;
const unsigned long raceSpeed = 160;

void setup() {
  Serial.begin(9600);

  for (int i = 0; i < 5; i++) {
    pinMode(horseLedPins[i], OUTPUT);
    digitalWrite(horseLedPins[i], LOW);
  }

  pinMode(leftButton, INPUT_PULLUP);
  pinMode(selectButton, INPUT_PULLUP);
  pinMode(rightButton, INPUT_PULLUP);

  randomSeed(analogRead(A0));

  sendSelectBet();
}

void loop() {
  handleButtons();

  if (state == RACING) {
    updateRace();
  }
}

// --------------------
// Buttons
// --------------------

void handleButtons() {
  bool currentLeft = digitalRead(leftButton);
  bool currentSelect = digitalRead(selectButton);
  bool currentRight = digitalRead(rightButton);

  if (millis() - lastButtonPress > buttonCooldown) {

    if (lastLeftState == HIGH && currentLeft == LOW) {
      lastButtonPress = millis();
      handleLeft();
    }

    if (lastRightState == HIGH && currentRight == LOW) {
      lastButtonPress = millis();
      handleRight();
    }

    if (lastSelectState == HIGH && currentSelect == LOW) {
      lastButtonPress = millis();
      handleSelect();
    }
  }

  lastLeftState = currentLeft;
  lastSelectState = currentSelect;
  lastRightState = currentRight;
}

void handleLeft() {
  if (state == SELECT_BET) {
    changeBet(-1);
    sendSelectBet();
  }
  else if (state == SELECT_HORSE) {
    selectedHorse--;
    if (selectedHorse < 0) selectedHorse = 4;

    showSelectedHorseLed();
    sendSelectHorse();
  }
}

void handleRight() {
  if (state == SELECT_BET) {
    changeBet(1);
    sendSelectBet();
  }
  else if (state == SELECT_HORSE) {
    selectedHorse++;
    if (selectedHorse > 4) selectedHorse = 0;

    showSelectedHorseLed();
    sendSelectHorse();
  }
}

void handleSelect() {
  if (state == SELECT_BET) {
    state = SELECT_HORSE;
    selectedHorse = 0;
    showSelectedHorseLed();
    sendSelectHorse();
  }
  else if (state == SELECT_HORSE) {
    startRace();
  }
  else if (state == RESULT) {
    if (bank <= 0) {
      state = GAME_OVER;
      sendGameOver();
    }
    else if (bank >= winTarget) {
      state = GAME_WON;
      sendGameWon();
    }
    else {
      state = SELECT_BET;
      adjustBetIfTooHigh();
      sendSelectBet();
    }
  }
  else if (state == GAME_OVER || state == GAME_WON) {
    resetGame();
  }
}

// --------------------
// Bet selector
// --------------------

int betOptions[] = {5, 10, 20, 30, 40, 50, 100, 200, 300, 400};
const int betOptionCount = sizeof(betOptions) / sizeof(betOptions[0]);

int getMaxBet() {
  // Starting bank is 20, so this keeps starting max at 10.
  // Later, as bank grows, higher options unlock.
  if (bank < 30) return 10;

  int rounded = (bank / 10) * 10;
  return rounded;
}

void changeBet(int direction) {
  int maxBet = getMaxBet();

  int currentIndex = 0;

  for (int i = 0; i < betOptionCount; i++) {
    if (betOptions[i] == betAmount) {
      currentIndex = i;
      break;
    }
  }

  while (true) {
    currentIndex += direction;

    if (currentIndex < 0) currentIndex = betOptionCount - 1;
    if (currentIndex >= betOptionCount) currentIndex = 0;

    int candidate = betOptions[currentIndex];

    if (candidate <= bank && candidate <= maxBet) {
      betAmount = candidate;
      break;
    }
  }
}

void adjustBetIfTooHigh() {
  int maxBet = getMaxBet();

  if (betAmount > bank || betAmount > maxBet) {
    betAmount = 5;
  }
}

// --------------------
// Race logic
// --------------------

void startRace() {
  if (bank < betAmount) {
    state = GAME_OVER;
    bank = 0;
    sendGameOver();
    return;
  }

  bank -= betAmount;

  for (int i = 0; i < 5; i++) {
    horsePositions[i] = 0;
    horseFinished[i] = false;
    finishOrder[i] = -1;
    digitalWrite(horseLedPins[i], LOW);
  }

  finishedCount = 0;
  winnerHorse = -1;
  playerPlace = 0;
  payout = 0;
  profitLoss = 0;

  state = RACING;
  lastRaceUpdate = millis();

  sendRaceUpdate();
}

void updateRace() {
  if (millis() - lastRaceUpdate < raceSpeed) {
    return;
  }

  lastRaceUpdate = millis();

  for (int i = 0; i < 5; i++) {
    if (!horseFinished[i]) {
      horsePositions[i] += random(2, 9);

      if (horsePositions[i] >= finishLine) {
        horsePositions[i] = finishLine;
        horseFinished[i] = true;

        finishOrder[finishedCount] = i;
        finishedCount++;
      }
    }
  }

  sendRaceUpdate();

  if (finishedCount >= 5) {
    finishRace();
  }
}

void finishRace() {
  winnerHorse = finishOrder[0];

  for (int i = 0; i < 5; i++) {
    if (finishOrder[i] == selectedHorse) {
      playerPlace = i + 1;
      break;
    }
  }

  if (playerPlace == 1) {
    payout = betAmount * 4;
  }
  else if (playerPlace == 2) {
    payout = betAmount * 2;
  }
  else if (playerPlace == 3) {
    payout = betAmount;
  }
  else {
    payout = 0;
  }

  bank += payout;
  profitLoss = payout - betAmount;

  showWinnerLed();

  state = RESULT;
  sendResult();
}

// --------------------
// LEDs
// --------------------

void allHorseLedsOff() {
  for (int i = 0; i < 5; i++) {
    digitalWrite(horseLedPins[i], LOW);
  }
}

void showSelectedHorseLed() {
  allHorseLedsOff();
  digitalWrite(horseLedPins[selectedHorse], HIGH);
}

void showWinnerLed() {
  allHorseLedsOff();
  digitalWrite(horseLedPins[winnerHorse], HIGH);
}

// --------------------
// Serial messages to Processing
// --------------------

void sendSelectBet() {
  state = SELECT_BET;

  Serial.print("BET,");
  Serial.print(bank);
  Serial.print(",");
  Serial.println(betAmount);
}

void sendSelectHorse() {
  Serial.print("HORSE,");
  Serial.print(bank);
  Serial.print(",");
  Serial.print(betAmount);
  Serial.print(",");
  Serial.println(selectedHorse);
}

void sendRaceUpdate() {
  Serial.print("RACE,");
  Serial.print(bank);
  Serial.print(",");
  Serial.print(betAmount);
  Serial.print(",");
  Serial.print(selectedHorse);

  for (int i = 0; i < 5; i++) {
    Serial.print(",");
    Serial.print(horsePositions[i]);
  }

  Serial.println();
}

void sendResult() {
  Serial.print("RESULT,");
  Serial.print(bank);
  Serial.print(",");
  Serial.print(betAmount);
  Serial.print(",");
  Serial.print(selectedHorse);
  Serial.print(",");
  Serial.print(winnerHorse);
  Serial.print(",");
  Serial.print(playerPlace);
  Serial.print(",");
  Serial.print(payout);
  Serial.print(",");
  Serial.print(profitLoss);

  for (int i = 0; i < 5; i++) {
    Serial.print(",");
    Serial.print(finishOrder[i]);
  }

  Serial.println();
}

void sendGameOver() {
  allHorseLedsOff();

  Serial.print("GAMEOVER,");
  Serial.println(bank);
}

void sendGameWon() {
  allHorseLedsOff();

  Serial.print("GAMEWON,");
  Serial.println(bank);
}

void resetGame() {
  bank = 20;
  betAmount = 5;
  selectedHorse = 0;

  allHorseLedsOff();

  state = SELECT_BET;
  sendSelectBet();
}
