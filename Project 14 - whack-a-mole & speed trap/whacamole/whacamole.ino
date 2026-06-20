const int sensorPins[5] = {2, 3, 4, 5, 6};
const int ledPins[5]    = {9, 10, 11, 12, 13};

const int totalMoles = 5;
const int maxMisses = 5;

// Most IR obstacle sensors output LOW when an object is detected.
// If yours behaves backwards, change this to HIGH.
const int DETECTED_STATE = LOW;

bool moleActive[5] = {false, false, false, false, false};
bool sensorWasTriggered[5] = {false, false, false, false, false};

unsigned long moleStartTime[5];
unsigned long moleDuration[5];

int score = 0;
int misses = 0;

bool gameStarted = false;
bool gameOver = false;

unsigned long gameStartTime = 0;
unsigned long lastSpawnTime = 0;

unsigned long spawnInterval = 1800;
unsigned long minSpawnInterval = 850;

unsigned long visibleTime = 2200;
unsigned long minVisibleTime = 1000;

int maxActiveMoles = 1;

void setup() {
  Serial.begin(9600);

  for (int i = 0; i < totalMoles; i++) {
    pinMode(sensorPins[i], INPUT);
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }

  randomSeed(analogRead(A0));

  Serial.println("=== IR Whack-a-Mole ===");
  Serial.println("Whack the lit-up mole by triggering the matching IR sensor.");
  Serial.println("Timer starts when you whack the first mole.");
  Serial.println("5 misses = game over.");
  Serial.println();

  resetGame();
  spawnMole();
}

void loop() {
  if (gameOver) {
    checkRestart();
    return;
  }

  checkSensors();
  checkMisses();
  updateDifficulty();
  spawnMolesIfNeeded();
}

void resetGame() {
  score = 0;
  misses = 0;

  gameStarted = false;
  gameOver = false;

  spawnInterval = 1800;
  visibleTime = 2200;
  maxActiveMoles = 1;

  lastSpawnTime = millis();

  for (int i = 0; i < totalMoles; i++) {
    moleActive[i] = false;
    sensorWasTriggered[i] = false;
    digitalWrite(ledPins[i], LOW);
  }

  Serial.println("New game ready.");
  Serial.println("Whack the first mole to start the timer!");
  Serial.println();
}

void spawnMolesIfNeeded() {
  unsigned long now = millis();

  if (now - lastSpawnTime >= spawnInterval) {
    if (countActiveMoles() < maxActiveMoles) {
      spawnMole();
      lastSpawnTime = now;
    }
  }
}

void spawnMole() {
  if (countActiveMoles() >= maxActiveMoles) {
    return;
  }

  int available[5];
  int availableCount = 0;

  for (int i = 0; i < totalMoles; i++) {
    if (!moleActive[i]) {
      available[availableCount] = i;
      availableCount++;
    }
  }

  if (availableCount == 0) {
    return;
  }

  int chosenIndex = available[random(availableCount)];

  moleActive[chosenIndex] = true;
  moleStartTime[chosenIndex] = millis();
  moleDuration[chosenIndex] = visibleTime;

  digitalWrite(ledPins[chosenIndex], HIGH);
}

void checkSensors() {
  for (int i = 0; i < totalMoles; i++) {
    bool currentlyTriggered = digitalRead(sensorPins[i]) == DETECTED_STATE;

    // Count only when the sensor changes from clear to triggered.
    // This prevents one long hand movement from counting multiple times.
    if (moleActive[i] && currentlyTriggered && !sensorWasTriggered[i]) {
      whackMole(i);
    }

    sensorWasTriggered[i] = currentlyTriggered;
  }
}

void whackMole(int index) {
  moleActive[index] = false;
  digitalWrite(ledPins[index], LOW);

  if (!gameStarted) {
    gameStarted = true;
    gameStartTime = millis();
    lastSpawnTime = millis();

    Serial.println("Game started!");
  }

  score++;

  Serial.print("Whacked mole ");
  Serial.print(index + 1);
  Serial.print(" | Score: ");
  Serial.print(score);
  Serial.print(" | Misses: ");
  Serial.print(misses);
  Serial.print(" | Active moles: ");
  Serial.println(countActiveMoles());
}

void checkMisses() {
  unsigned long now = millis();

  for (int i = 0; i < totalMoles; i++) {
    if (moleActive[i] && now - moleStartTime[i] >= moleDuration[i]) {
      moleActive[i] = false;
      digitalWrite(ledPins[i], LOW);

      misses++;

      Serial.print("Missed mole ");
      Serial.print(i + 1);
      Serial.print(" | Score: ");
      Serial.print(score);
      Serial.print(" | Misses: ");
      Serial.println(misses);

      if (misses >= maxMisses) {
        endGame();
        return;
      }
    }
  }
}

void updateDifficulty() {
  if (!gameStarted) {
    return;
  }

  // Increase difficulty only up to score 40.
  // After that, keep the game at a stable maximum difficulty.
  int difficultyScore = score;

  if (difficultyScore > 40) {
    difficultyScore = 40;
  }

  // More moles appear as score increases.
  if (difficultyScore >= 8) {
    maxActiveMoles = 2;
  } else {
    maxActiveMoles = 1;
  }

  if (difficultyScore >= 20) {
    maxActiveMoles = 3;
  }

  // Calculate speed using capped difficultyScore, not actual score.
  long calculatedVisibleTime = 2200 - (difficultyScore * 35);
  long calculatedSpawnInterval = 1800 - (difficultyScore * 25);

  if (calculatedVisibleTime < minVisibleTime) {
    calculatedVisibleTime = minVisibleTime;
  }

  if (calculatedSpawnInterval < minSpawnInterval) {
    calculatedSpawnInterval = minSpawnInterval;
  }

  visibleTime = calculatedVisibleTime;
  spawnInterval = calculatedSpawnInterval;
}

int countActiveMoles() {
  int count = 0;

  for (int i = 0; i < totalMoles; i++) {
    if (moleActive[i]) {
      count++;
    }
  }

  return count;
}

void endGame() {
  gameOver = true;

  for (int i = 0; i < totalMoles; i++) {
    moleActive[i] = false;
    sensorWasTriggered[i] = false;
    digitalWrite(ledPins[i], LOW);
  }

  unsigned long gameTime = 0;

  if (gameStarted) {
    gameTime = millis() - gameStartTime;
  }

  Serial.println();
  Serial.println("=== GAME OVER ===");

  Serial.print("Final score: ");
  Serial.println(score);

  Serial.print("Misses: ");
  Serial.println(misses);

  Serial.print("Game time: ");
  Serial.print(gameTime / 1000.0);
  Serial.println(" seconds");

  Serial.println();
  Serial.println("Trigger any sensor to restart.");
  Serial.println();
}

void checkRestart() {
  for (int i = 0; i < totalMoles; i++) {
    bool currentlyTriggered = digitalRead(sensorPins[i]) == DETECTED_STATE;

    if (currentlyTriggered && !sensorWasTriggered[i]) {
      delay(300);
      resetGame();
      spawnMole();
      return;
    }

    sensorWasTriggered[i] = currentlyTriggered;
  }
}
