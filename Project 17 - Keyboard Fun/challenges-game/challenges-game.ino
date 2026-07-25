#include <Wire.h>

// PCF8574 addresses
const byte PCF1 = 0x20;
const byte PCF2 = 0x21;

// Uno buttons D2-D13 = A-L
const int unoButtonPins[12] = {
  2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
};

// A-Z letters
const char letters[26] = {
  'A', 'B', 'C', 'D', 'E', 'F',
  'G', 'H', 'I', 'J', 'K', 'L',
  'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
  'U', 'V', 'W', 'X', 'Y', 'Z'
};

// EDIT THESE ARRAYS TO MATCH YOUR REAL BUTTON COLOURS
const char yellowLetters[] = {'A', 'F', 'K', 'P', 'U', 'Z'};
const char greenLetters[] = {'B', 'G', 'L', 'Q', 'V'};
const char blackLetters[] = {'C', 'H', 'M', 'R', 'W'};
const char blueLetters[] = {'D', 'I', 'N', 'S', 'X'};
const char redLetters[] = {'E', 'J', 'O', 'T', 'Y'};

// Random 5-letter words
const char* words[] = {
  "APPLE", "HOUSE", "PLANT", "WATER", "LIGHT",
  "MOUSE", "CHAIR", "BREAD", "GRAPE", "SNAKE",
  "BRICK", "CLOUD", "HEART", "TRAIN", "STONE",
  "SMILE", "RIVER", "FLAME", "MUSIC", "PAPER",
  "GREEN", "BLACK", "WHITE", "BRAIN", "WORLD"
};

const int wordCount = 25;
const unsigned long taskTimeLimit = 10000; // 10 seconds

// Button states
bool previousButtonState[26];
bool previousSpaceState = false;

unsigned long lastButtonPressTime[26];
unsigned long lastSpacePressTime = 0;
const unsigned long buttonCooldown = 180;

// Game states
enum GameState {
  WAITING_TO_START_GAME,
  WAITING_TO_START_TASK,
  TASK_RUNNING,
  GAME_OVER
};

GameState gameState = WAITING_TO_START_GAME;

// Task types
enum TaskType {
  SPELL_WORD,
  PRESS_POSITIONS,
  PRESS_COLOUR
};

TaskType currentTask;

String playerInput = "";
String expectedInput = "";
String taskDescription = "";

int score = 0;
unsigned long taskStartTime = 0;
int lastPrintedSecond = -1;

void setup() {
  Serial.begin(9600);
  Wire.begin();

  for (int i = 0; i < 12; i++) {
    pinMode(unoButtonPins[i], INPUT_PULLUP);
  }

  // PCF8574 pins HIGH = input mode
  Wire.beginTransmission(PCF1);
  Wire.write(0xFF);
  Wire.endTransmission();

  Wire.beginTransmission(PCF2);
  Wire.write(0xFF);
  Wire.endTransmission();

  for (int i = 0; i < 26; i++) {
    previousButtonState[i] = false;
    lastButtonPressTime[i] = 0;
  }

  randomSeed(analogRead(A0));

  Serial.println("=== A-Z Keyboard Challenge ===");
  Serial.println("Complete each task within 10 seconds.");
  Serial.println("SPACE starts and submits each task.");
  Serial.println("One failed task = game over.");
  Serial.println();
  Serial.println("Press SPACE to start the game.");
}

void loop() {
  readAllButtons();

  if (gameState == TASK_RUNNING) {
    checkTaskTimeout();
  }

  delay(15);
}

void readAllButtons() {
  checkUnoButtons();
  checkPCF1Buttons();
  checkPCF2Buttons();
}

void checkUnoButtons() {
  for (int i = 0; i < 12; i++) {
    bool pressed = digitalRead(unoButtonPins[i]) == LOW;
    handleLetterButton(i, pressed);
  }
}

void checkPCF1Buttons() {
  byte data = readPCF(PCF1);

  // PCF1 P0-P7 = M-T = indexes 12-19
  for (int pin = 0; pin < 8; pin++) {
    bool pressed = bitRead(data, pin) == 0;
    handleLetterButton(12 + pin, pressed);
  }
}

void checkPCF2Buttons() {
  byte data = readPCF(PCF2);

  // PCF2 P0-P5 = U-Z = indexes 20-25
  for (int pin = 0; pin < 6; pin++) {
    bool pressed = bitRead(data, pin) == 0;
    handleLetterButton(20 + pin, pressed);
  }

  // PCF2 P6 = SPACE
  bool spacePressed = bitRead(data, 6) == 0;
  handleSpaceButton(spacePressed);
}

byte readPCF(byte address) {
  Wire.requestFrom(address, 1);

  if (Wire.available()) {
    return Wire.read();
  }

  return 0xFF;
}

void handleLetterButton(int index, bool pressed) {
  if (pressed && !previousButtonState[index]) {
    if (millis() - lastButtonPressTime[index] > buttonCooldown) {
      lastButtonPressTime[index] = millis();

      if (gameState == TASK_RUNNING) {
        char letter = letters[index];
        playerInput += letter;

        Serial.print("Input: ");
        Serial.println(playerInput);
      }
    }
  }

  previousButtonState[index] = pressed;
}

void handleSpaceButton(bool pressed) {
  if (pressed && !previousSpaceState) {
    if (millis() - lastSpacePressTime > buttonCooldown) {
      lastSpacePressTime = millis();
      handleSpaceAction();
    }
  }

  previousSpaceState = pressed;
}

void handleSpaceAction() {
  if (gameState == WAITING_TO_START_GAME) {
    score = 0;
    Serial.println();
    Serial.println("Game started!");
    prepareNextTask();
  }
  else if (gameState == WAITING_TO_START_TASK) {
    startTaskTimer();
  }
  else if (gameState == TASK_RUNNING) {
    submitTask();
  }
  else if (gameState == GAME_OVER) {
    Serial.println();
    Serial.println("Restarting game...");
    score = 0;
    prepareNextTask();
  }
}

void prepareNextTask() {
  playerInput = "";
  expectedInput = "";
  lastPrintedSecond = -1;

  int taskNumber = random(3);

  if (taskNumber == 0) {
    createSpellWordTask();
  }
  else if (taskNumber == 1) {
    createPositionTask();
  }
  else {
    createColourTask();
  }

  Serial.println();
  Serial.println("------------------------------");
  Serial.print("Score: ");
  Serial.println(score);
  Serial.println(taskDescription);
  Serial.println("Press SPACE to start the 10 second timer.");

  gameState = WAITING_TO_START_TASK;
}

void startTaskTimer() {
  playerInput = "";
  taskStartTime = millis();
  lastPrintedSecond = 10;

  Serial.println();
  Serial.println("Timer started!");
  Serial.println("You have 10 seconds.");
  Serial.println();

  gameState = TASK_RUNNING;
}

void submitTask() {
  unsigned long elapsed = millis() - taskStartTime;

  Serial.println();
  Serial.print("Submitted: ");
  Serial.println(playerInput);

  Serial.print("Expected: ");
  Serial.println(expectedInput);

  if (elapsed > taskTimeLimit) {
    gameOver("Time ran out before submit.");
    return;
  }

  bool correct = false;

  if (currentTask == SPELL_WORD) {
    correct = playerInput == expectedInput;
  }
  else {
    correct = sameLettersAnyOrder(playerInput, expectedInput);
  }

  if (correct) {
    score++;
    Serial.println("Correct! +1 point");
    prepareNextTask();
  }
  else {
    gameOver("Wrong answer.");
  }
}

void checkTaskTimeout() {
  unsigned long elapsed = millis() - taskStartTime;

  int secondsLeft = 10 - (elapsed / 1000);

  if (secondsLeft != lastPrintedSecond && secondsLeft >= 0) {
    lastPrintedSecond = secondsLeft;

    Serial.print("Time left: ");
    Serial.print(secondsLeft);
    Serial.println("s");
  }

  if (elapsed > taskTimeLimit) {
    gameOver("Time is up.");
  }
}

void gameOver(String reason) {
  gameState = GAME_OVER;

  Serial.println();
  Serial.println("=== GAME OVER ===");
  Serial.println(reason);

  Serial.print("Final score: ");
  Serial.println(score);

  Serial.println("Press SPACE to restart.");
}

void createSpellWordTask() {
  currentTask = SPELL_WORD;

  int index = random(wordCount);
  expectedInput = String(words[index]);

  taskDescription = "Task: Spell this word: ";
  taskDescription += expectedInput;
}

void createPositionTask() {
  currentTask = PRESS_POSITIONS;

  int pos1 = random(1, 27);
  int pos2 = random(1, 27);

  while (pos2 == pos1) {
    pos2 = random(1, 27);
  }

  char letter1 = 'A' + (pos1 - 1);
  char letter2 = 'A' + (pos2 - 1);

  expectedInput = "";
  expectedInput += letter1;
  expectedInput += letter2;

  taskDescription = "Task: Press the ";
  taskDescription += String(pos1);
  taskDescription += ordinalSuffix(pos1);
  taskDescription += " and ";
  taskDescription += String(pos2);
  taskDescription += ordinalSuffix(pos2);
  taskDescription += " letters of the alphabet.";
}

void createColourTask() {
  currentTask = PRESS_COLOUR;

  int colourIndex = random(5);

  if (colourIndex == 0) {
    expectedInput = lettersArrayToString(yellowLetters, sizeof(yellowLetters));
    taskDescription = "Task: Press all YELLOW letters.";
  }
  else if (colourIndex == 1) {
    expectedInput = lettersArrayToString(redLetters, sizeof(redLetters));
    taskDescription = "Task: Press all RED letters.";
  }
  else if (colourIndex == 2) {
    expectedInput = lettersArrayToString(greenLetters, sizeof(greenLetters));
    taskDescription = "Task: Press all GREEN letters.";
  }
  else if (colourIndex == 3) {
    expectedInput = lettersArrayToString(blueLetters, sizeof(blueLetters));
    taskDescription = "Task: Press all BLUE letters.";
  }
  else {
    expectedInput = lettersArrayToString(blackLetters, sizeof(blackLetters));
    taskDescription = "Task: Press all BLACK letters.";
  }
}

String lettersArrayToString(const char arr[], int length) {
  String result = "";

  for (int i = 0; i < length; i++) {
    result += arr[i];
  }

  return result;
}

bool sameLettersAnyOrder(String input, String expected) {
  if (input.length() != expected.length()) {
    return false;
  }

  int inputCounts[26];
  int expectedCounts[26];

  for (int i = 0; i < 26; i++) {
    inputCounts[i] = 0;
    expectedCounts[i] = 0;
  }

  for (int i = 0; i < input.length(); i++) {
    char c = input.charAt(i);

    if (c < 'A' || c > 'Z') {
      return false;
    }

    inputCounts[c - 'A']++;
  }

  for (int i = 0; i < expected.length(); i++) {
    char c = expected.charAt(i);

    if (c < 'A' || c > 'Z') {
      return false;
    }

    expectedCounts[c - 'A']++;
  }

  for (int i = 0; i < 26; i++) {
    if (inputCounts[i] != expectedCounts[i]) {
      return false;
    }
  }

  return true;
}

String ordinalSuffix(int number) {
  if (number == 11 || number == 12 || number == 13) {
    return "th";
  }

  int lastDigit = number % 10;

  if (lastDigit == 1) {
    return "st";
  }
  else if (lastDigit == 2) {
    return "nd";
  }
  else if (lastDigit == 3) {
    return "rd";
  }

  return "th";
}
