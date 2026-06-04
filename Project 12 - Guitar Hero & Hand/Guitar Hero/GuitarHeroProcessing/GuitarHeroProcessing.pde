import processing.serial.*;

Serial arduino;

String portName = "COM3"; // Change to your Arduino Uno port
String sensorData = "00000";

ArrayList<Note> notes = new ArrayList<Note>();

int lanes = 5;
int score = 0;
int misses = 0;

int[] correctHits = new int[5];
int[] missedNotes = new int[5];

int spawnTimer = 0;
int spawnDelay = 45;

int hitLineY;
int noteSpeed = 5;

boolean[] touched = new boolean[5];
boolean[] previousTouched = new boolean[5];

int gameState = 0;
// 0 = waiting to start
// 1 = playing
// 2 = game over / summary

int gameDuration = 60000; // 60 seconds
int gameStartTime = 0;

void setup() {
  size(800, 600);
  hitLineY = height - 100;

  println(Serial.list());

  arduino = new Serial(this, portName, 9600);
  arduino.bufferUntil('\n');

  textAlign(CENTER, CENTER);
}

void draw() {
  background(20);

  readTouches();

  if (gameState == 0) {
    drawStartScreen();

    if (anyNewTouch()) {
      startGame();
    }
  } 
  else if (gameState == 1) {
    drawLanes();
    spawnNotes();
    updateNotes();
    drawGameUI();

    if (millis() - gameStartTime >= gameDuration) {
      endGame();
    }
  } 
  else if (gameState == 2) {
    drawSummaryScreen();

    if (anyNewTouch()) {
      startGame();
    }
  }

  for (int i = 0; i < 5; i++) {
    previousTouched[i] = touched[i];
  }
}

void serialEvent(Serial arduino) {
  sensorData = trim(arduino.readStringUntil('\n'));
}

void readTouches() {
  if (sensorData.length() >= 5) {
    for (int i = 0; i < 5; i++) {
      touched[i] = sensorData.charAt(i) == '1';
    }
  }
}

boolean anyNewTouch() {
  for (int i = 0; i < 5; i++) {
    if (touched[i] && !previousTouched[i]) {
      return true;
    }
  }
  return false;
}

void startGame() {
  score = 0;
  misses = 0;

  for (int i = 0; i < 5; i++) {
    correctHits[i] = 0;
    missedNotes[i] = 0;
  }

  notes.clear();

  spawnTimer = 0;
  spawnDelay = 45;
  noteSpeed = 5;

  gameStartTime = millis();
  gameState = 1;
}

void endGame() {
  notes.clear();
  gameState = 2;
}

void drawStartScreen() {
  fill(255);
  textSize(42);
  text("5-String Touch Guitar Hero", width / 2, height / 2 - 80);

  textSize(24);
  text("Touch any sensor to start", width / 2, height / 2);

  textSize(18);
  text("Game length: 60 seconds", width / 2, height / 2 + 40);
}

void drawLanes() {
  int laneWidth = width / lanes;

  for (int i = 0; i < lanes; i++) {
    int x = i * laneWidth;

    stroke(80);
    line(x, 0, x, height);

    if (touched[i]) {
      fill(90);
    } else {
      fill(40);
    }

    rect(x, hitLineY, laneWidth, 60);

    fill(255);
    textSize(24);
    text("S" + (i + 1), x + laneWidth / 2, hitLineY + 30);
  }

  stroke(255);
  strokeWeight(3);
  line(0, hitLineY, width, hitLineY);
  strokeWeight(1);
}

void spawnNotes() {
  spawnTimer++;

  if (spawnTimer >= spawnDelay) {
    int randomLane = int(random(lanes));
    notes.add(new Note(randomLane));
    spawnTimer = 0;

    if (spawnDelay > 25) {
      spawnDelay--;
    }
  }
}

void updateNotes() {
  for (int i = notes.size() - 1; i >= 0; i--) {
    Note n = notes.get(i);
    n.update();
    n.display();

    boolean newTouch = touched[n.lane] && !previousTouched[n.lane];

    if (newTouch && abs(n.y - hitLineY) < 45) {
      score++;
      correctHits[n.lane]++;
      notes.remove(i);
      continue;
    }

    if (n.y > height) {
      misses++;
      missedNotes[n.lane]++;
      notes.remove(i);
    }
  }
}

void drawGameUI() {
  int timeLeft = max(0, (gameDuration - (millis() - gameStartTime)) / 1000);

  fill(255);
  textSize(24);
  textAlign(LEFT, CENTER);
  text("Score: " + score, 20, 30);
  text("Misses: " + misses, 20, 60);
  text("Time: " + timeLeft + "s", 20, 90);

  textAlign(CENTER, CENTER);
  textSize(18);
  text("Touch the matching sensor when the note reaches the hit line", width / 2, 30);
}

void drawSummaryScreen() {
  fill(255);
  textAlign(CENTER, CENTER);

  textSize(42);
  text("Game Over", width / 2, 70);

  textSize(28);
  text("Score: " + score, width / 2, 130);
  text("Misses: " + misses, width / 2, 170);

  textSize(22);
  text("Sensor Summary", width / 2, 230);

  textSize(18);

  int startY = 280;

  for (int i = 0; i < 5; i++) {
    String line = "Sensor " + (i + 1) + 
                  "  |  Correct: " + correctHits[i] + 
                  "  |  Missed: " + missedNotes[i];

    text(line, width / 2, startY + i * 30);
  }

  int bestSensor = getBestSensor();
  int worstSensor = getWorstSensor();

  textSize(20);

  if (bestSensor != -1) {
    text("Most successful: Sensor " + (bestSensor + 1) + 
         " with " + correctHits[bestSensor] + " correct hits", width / 2, 460);
  } else {
    text("Most successful: none yet", width / 2, 460);
  }

  if (worstSensor != -1) {
    text("Most missed: Sensor " + (worstSensor + 1) + 
         " with " + missedNotes[worstSensor] + " misses", width / 2, 490);
  } else {
    text("Most missed: none", width / 2, 490);
  }

  textSize(22);
  text("Touch any sensor to restart", width / 2, 550);
}

int getBestSensor() {
  int bestIndex = -1;
  int bestValue = 0;

  for (int i = 0; i < 5; i++) {
    if (correctHits[i] > bestValue) {
      bestValue = correctHits[i];
      bestIndex = i;
    }
  }

  return bestIndex;
}

int getWorstSensor() {
  int worstIndex = -1;
  int worstValue = 0;

  for (int i = 0; i < 5; i++) {
    if (missedNotes[i] > worstValue) {
      worstValue = missedNotes[i];
      worstIndex = i;
    }
  }

  return worstIndex;
}

class Note {
  int lane;
  float y;
  int laneWidth;

  Note(int laneNumber) {
    lane = laneNumber;
    y = -30;
    laneWidth = width / lanes;
  }

  void update() {
    y += noteSpeed;
  }

  void display() {
    int x = lane * laneWidth + laneWidth / 2;

    fill(255);
    ellipse(x, y, 50, 50);
  }
}
