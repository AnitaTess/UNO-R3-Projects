import processing.serial.*;

Serial arduino;

int xValue = 512;
int yValue = 512;
int buttonState = 1;

// Game settings
int gridSize = 20;
int cols;
int rows;

// Snake position lists
ArrayList<PVector> snake = new ArrayList<PVector>();

// Direction
int dirX = 1;
int dirY = 0;
int nextDirX = 1;
int nextDirY = 0;

// Food
PVector food;

// Score
int score = 0;

// Timing
int lastMoveTime = 0;
int moveDelay = 150; // lower = faster

boolean gameOver = false;

void setup() {
  size(600, 600);

  cols = width / gridSize;
  rows = height / gridSize;

  println(Serial.list());

  // Change [0] if this is not your Arduino COM port.
  arduino = new Serial(this, Serial.list()[0], 9600);
  arduino.bufferUntil('\n');

  restartGame();
}

void draw() {
  background(25);

  readJoystickDirection();

  if (!gameOver && millis() - lastMoveTime > moveDelay) {
    moveSnake();
    lastMoveTime = millis();
  }

  drawFood();
  drawSnake();
  drawScore();

  if (gameOver) {
    drawGameOver();

    // Press joystick button to restart manually too
    if (buttonState == 0) {
      restartGame();
    }
  }
}

void readJoystickDirection() {
  // Adjust these thresholds if your joystick is too sensitive
  int lowThreshold = 350;
  int highThreshold = 650;

  // Left / right
  if (xValue < lowThreshold && dirX != 1) {
    nextDirX = -1;
    nextDirY = 0;
  } 
  else if (xValue > highThreshold && dirX != -1) {
    nextDirX = 1;
    nextDirY = 0;
  }

  // Up / down
  if (yValue < lowThreshold && dirY != 1) {
    nextDirX = 0;
    nextDirY = -1;
  } 
  else if (yValue > highThreshold && dirY != -1) {
    nextDirX = 0;
    nextDirY = 1;
  }
}

void moveSnake() {
  dirX = nextDirX;
  dirY = nextDirY;

  PVector head = snake.get(0).copy();
  head.x += dirX;
  head.y += dirY;

  // Check border collision
  if (head.x < 0 || head.x >= cols || head.y < 0 || head.y >= rows) {
    triggerGameOver();
    return;
  }

  // Check self collision
  for (int i = 0; i < snake.size(); i++) {
    PVector part = snake.get(i);
    if (head.x == part.x && head.y == part.y) {
      triggerGameOver();
      return;
    }
  }

  // Add new head
  snake.add(0, head);

  // Check food collision
  if (head.x == food.x && head.y == food.y) {
    score++;
    spawnFood();

    // Optional: speed up a little as score increases
    moveDelay = max(70, moveDelay - 3);
  } 
  else {
    // Remove tail if no food eaten
    snake.remove(snake.size() - 1);
  }
}

void triggerGameOver() {
  gameOver = true;

  // Auto restart after short delay
  delay(800);
  restartGame();
}

void restartGame() {
  snake.clear();

  int startX = cols / 2;
  int startY = rows / 2;

  snake.add(new PVector(startX, startY));
  snake.add(new PVector(startX - 1, startY));
  snake.add(new PVector(startX - 2, startY));

  dirX = 1;
  dirY = 0;
  nextDirX = 1;
  nextDirY = 0;

  score = 0;
  moveDelay = 150;
  gameOver = false;

  spawnFood();
}

void spawnFood() {
  boolean validSpot = false;

  while (!validSpot) {
    food = new PVector(floor(random(cols)), floor(random(rows)));
    validSpot = true;

    for (PVector part : snake) {
      if (food.x == part.x && food.y == part.y) {
        validSpot = false;
        break;
      }
    }
  }
}

void drawSnake() {
  noStroke();

  for (int i = 0; i < snake.size(); i++) {
    PVector part = snake.get(i);

    if (i == 0) {
      fill(80, 220, 120); // head
    } 
    else {
      fill(50, 170, 90); // body
    }

    rect(part.x * gridSize, part.y * gridSize, gridSize - 1, gridSize - 1);
  }
}

void drawFood() {
  fill(230, 70, 70);
  noStroke();
  rect(food.x * gridSize, food.y * gridSize, gridSize - 1, gridSize - 1);
}

void drawScore() {
  fill(255);
  textSize(22);
  text("Score: " + score, 15, 28);

  textSize(12);
  text("X: " + xValue + "  Y: " + yValue + "  Button: " + buttonState, 15, height - 15);
}

void drawGameOver() {
  fill(255);
  textAlign(CENTER, CENTER);
  textSize(40);
  text("Game Over", width / 2, height / 2 - 20);

  textSize(18);
  text("Restarting...", width / 2, height / 2 + 25);

  textAlign(LEFT, BASELINE);
}

void serialEvent(Serial arduino) {
  String data = arduino.readStringUntil('\n');

  if (data != null) {
    data = trim(data);
    String[] values = split(data, ',');

    if (values.length == 3) {
      xValue = int(values[0]);
      yValue = int(values[1]);
      buttonState = int(values[2]);
    }
  }
}
