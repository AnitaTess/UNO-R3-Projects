#include <LiquidCrystal.h>

// LCD pins from your starter code
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

// Button
const int buttonPin = 5;

// Game settings
const int playerCol = 1;
int obstacleCol = 15;

bool isJumping = false;
unsigned long jumpStartTime = 0;
const unsigned long jumpDuration = 500;

unsigned long lastMoveTime = 0;
int moveDelay = 350;

int score = 0;
bool gameOver = false;

// Custom character: little dinosaur/player
byte dino[8] = {
  B00111,
  B00101,
  B00111,
  B10110,
  B11111,
  B00110,
  B01010,
  B10010
};

// Custom character: cactus/obstacle
byte cactus[8] = {
  B00100,
  B10100,
  B10101,
  B10101,
  B11101,
  B00100,
  B00100,
  B00100
};

void setup() {
  lcd.begin(16, 2);

  pinMode(buttonPin, INPUT_PULLUP);

  lcd.createChar(0, dino);
  lcd.createChar(1, cactus);

  showStartScreen();

  // Wait for button press to start
  while (digitalRead(buttonPin) == HIGH) {
    delay(10);
  }

  startGame();
}

void loop() {
  if (gameOver) {
    if (digitalRead(buttonPin) == LOW) {
      delay(250);
      startGame();
    }
    return;
  }

  handleJump();
  updateJump();
  moveObstacle();
  drawGame();
}

void showStartScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("DINO JUMP GAME");
  lcd.setCursor(0, 1);
  lcd.print("Press to start");
}

void startGame() {
  obstacleCol = 15;
  isJumping = false;
  score = 0;
  moveDelay = 350;
  gameOver = false;
  lastMoveTime = millis();

  lcd.clear();
}

void handleJump() {
  if (digitalRead(buttonPin) == LOW && !isJumping) {
    isJumping = true;
    jumpStartTime = millis();
  }
}

void updateJump() {
  if (isJumping && millis() - jumpStartTime > jumpDuration) {
    isJumping = false;
  }
}

void moveObstacle() {
  if (millis() - lastMoveTime >= moveDelay) {
    lastMoveTime = millis();

    obstacleCol--;

    if (obstacleCol < 0) {
      obstacleCol = 15;
      score++;

      // Make game slightly faster as score increases
      if (moveDelay > 150) {
        moveDelay -= 10;
      }
    }

    checkCollision();
  }
}

void checkCollision() {
  int playerRow = isJumping ? 0 : 1;
  int obstacleRow = 1;

  if (obstacleCol == playerCol && playerRow == obstacleRow) {
    endGame();
  }
}

void drawGame() {
  lcd.clear();

  // Draw score
  lcd.setCursor(10, 0);
  lcd.print("S:");
  lcd.print(score);

  // Draw player
  int playerRow = isJumping ? 0 : 1;
  lcd.setCursor(playerCol, playerRow);
  lcd.write(byte(0));

  // Draw obstacle
  lcd.setCursor(obstacleCol, 1);
  lcd.write(byte(1));
}

void endGame() {
  gameOver = true;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("GAME OVER");
  lcd.setCursor(0, 1);
  lcd.print("Score: ");
  lcd.print(score);

  delay(1000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Press button");
  lcd.setCursor(0, 1);
  lcd.print("to restart");
}
