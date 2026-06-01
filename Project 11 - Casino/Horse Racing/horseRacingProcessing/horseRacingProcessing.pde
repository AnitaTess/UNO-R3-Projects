import processing.serial.*;

Serial myPort;

String mode = "WAITING";

int bank = 20;
int bet = 5;
int selectedHorse = 0;
int winnerHorse = -1;
int playerPlace = 0;
int payout = 0;
int profitLoss = 0;

int[] horsePos = {0, 0, 0, 0, 0};
int[] finishOrder = {-1, -1, -1, -1, -1};

String[] horseNames = {
  "BLUE", "YELLOW", "GREEN", "WHITE", "RED"
};

color[] horseColors = {
  color(70, 140, 255),
  color(255, 220, 50),
  color(80, 220, 90),
  color(245),
  color(240, 70, 70)
};

void setup() {
  size(1100, 650);
  smooth();

  println(Serial.list());

  myPort = new Serial(this, Serial.list()[0], 9600);
  myPort.bufferUntil('\n');
}

void draw() {
  background(18, 110, 48);

  drawTopBar();
  drawTrack();

  if (mode.equals("BET")) {
    drawBetScreen();
  }
  else if (mode.equals("HORSE")) {
    drawHorseSelectScreen();
  }
  else if (mode.equals("RACE")) {
    drawRaceScreen();
  }
  else if (mode.equals("RESULT")) {
    drawResultScreen();
  }
  else if (mode.equals("GAMEOVER")) {
    drawGameOverScreen();
  }
  else if (mode.equals("GAMEWON")) {
    drawGameWonScreen();
  }
  else {
    drawWaitingScreen();
  }
}

// --------------------
// Serial parsing
// --------------------

void serialEvent(Serial p) {
  String line = trim(p.readStringUntil('\n'));

  if (line == null || line.length() == 0) return;

  println(line);

  String[] parts = split(line, ',');

  if (parts[0].equals("BET")) {
    mode = "BET";
    bank = int(parts[1]);
    bet = int(parts[2]);
  }

  else if (parts[0].equals("HORSE")) {
    mode = "HORSE";
    bank = int(parts[1]);
    bet = int(parts[2]);
    selectedHorse = int(parts[3]);
  }

  else if (parts[0].equals("RACE")) {
    mode = "RACE";
    bank = int(parts[1]);
    bet = int(parts[2]);
    selectedHorse = int(parts[3]);

    for (int i = 0; i < 5; i++) {
      horsePos[i] = int(parts[4 + i]);
    }
  }

  else if (parts[0].equals("RESULT")) {
    mode = "RESULT";
    bank = int(parts[1]);
    bet = int(parts[2]);
    selectedHorse = int(parts[3]);
    winnerHorse = int(parts[4]);
    playerPlace = int(parts[5]);
    payout = int(parts[6]);
    profitLoss = int(parts[7]);

    for (int i = 0; i < 5; i++) {
      finishOrder[i] = int(parts[8 + i]);
    }
  }

  else if (parts[0].equals("GAMEOVER")) {
    mode = "GAMEOVER";
    bank = int(parts[1]);
  }

  else if (parts[0].equals("GAMEWON")) {
    mode = "GAMEWON";
    bank = int(parts[1]);
  }
}

// --------------------
// Drawing
// --------------------

void drawTopBar() {
  noStroke();
  fill(10, 70, 30);
  rect(0, 0, width, 75);

  fill(255);
  textAlign(LEFT, CENTER);
  textSize(28);
  text("BANK: £" + bank, 30, 38);

  textAlign(CENTER, CENTER);
  text("HORSE RACE CASINO", width / 2, 38);

  textAlign(RIGHT, CENTER);
  text("BET: £" + bet, width - 30, 38);
}

void drawTrack() {
  int startX = 120;
  int finishX = width - 120;
  int topY = 130;
  int laneH = 75;

  stroke(255);
  strokeWeight(3);

  for (int i = 0; i < 6; i++) {
    line(startX, topY + i * laneH, finishX, topY + i * laneH);
  }

  stroke(255, 240, 80);
  strokeWeight(5);
  line(finishX, topY, finishX, topY + 5 * laneH);

  fill(255);
  textSize(16);
  textAlign(CENTER);
  text("FINISH", finishX, topY - 15);

  for (int i = 0; i < 5; i++) {
    drawHorse(i, horsePos[i]);
  }
}

void drawHorse(int index, int pos) {
  int startX = 120;
  int finishX = width - 120;
  int topY = 130;
  int laneH = 75;

  float x = map(pos, 0, 100, startX, finishX);
  float y = topY + index * laneH + laneH / 2;

  if (index == selectedHorse) {
    stroke(255);
    strokeWeight(5);
  } else {
    noStroke();
  }

  fill(horseColors[index]);
  ellipse(x, y, 50, 34);

  fill(0);
  ellipse(x + 15, y - 5, 8, 8);

  stroke(0);
  strokeWeight(3);
  line(x - 15, y + 18, x - 25, y + 32);
  line(x + 10, y + 18, x + 20, y + 32);

  noStroke();
  fill(255);
  textAlign(RIGHT, CENTER);
  textSize(18);
  text(horseNames[index], startX - 20, y);
}

void drawBetScreen() {
  drawPanel("Choose your bet",
    "LEFT / RIGHT changes amount    SELECT confirms",
    "Current bet: £" + bet);
}

void drawHorseSelectScreen() {
  drawPanel("Choose your horse",
    "LEFT / RIGHT changes horse    SELECT starts race",
    "Selected: " + horseNames[selectedHorse]);
}

void drawRaceScreen() {
  fill(255);
  textAlign(CENTER);
  textSize(32);
  text("RACE IN PROGRESS!", width / 2, height - 55);
}

void drawResultScreen() {
  fill(0, 0, 0, 170);
  rect(220, 120, width - 440, height - 180, 20);

  fill(255);
  textAlign(CENTER);
  textSize(34);

  if (profitLoss > 0) {
    text("YOU WON £" + profitLoss + "!", width / 2, 165);
  }
  else if (profitLoss == 0) {
    text("NO GAIN, NO LOSS", width / 2, 165);
  }
  else {
    text("YOU LOST £" + abs(profitLoss), width / 2, 165);
  }

  textSize(22);
  text("Your horse: " + horseNames[selectedHorse], width / 2, 215);
  text("Your place: " + ordinal(playerPlace), width / 2, 250);
  text("Payout: £" + payout, width / 2, 285);
  text("Bank: £" + bank, width / 2, 320);

  textSize(20);
  text("Final places:", width / 2, 370);

  for (int i = 0; i < 5; i++) {
    int h = finishOrder[i];
    fill(horseColors[h]);
    text((i + 1) + ". " + horseNames[h], width / 2, 405 + i * 28);
  }

  fill(255);
  textSize(18);
  text("Press SELECT for next race", width / 2, height - 55);
}

void drawGameOverScreen() {
  drawPanel("GAME OVER",
    "You are bankrupt.",
    "Press SELECT to restart");
}

void drawGameWonScreen() {
  drawPanel("YOU WON THE GAME!",
    "You reached £500.",
    "Press SELECT to restart");
}

void drawWaitingScreen() {
  drawPanel("Waiting for Arduino",
    "Check COM port in Processing.",
    "Close Arduino Serial Monitor.");
}

void drawPanel(String title, String line1, String line2) {
  fill(0, 0, 0, 165);
  rect(180, 170, width - 360, 250, 20);

  fill(255);
  textAlign(CENTER);
  textSize(36);
  text(title, width / 2, 220);

  textSize(22);
  text(line1, width / 2, 290);

  textSize(28);
  text(line2, width / 2, 350);
}

String ordinal(int n) {
  if (n == 1) return "1st";
  if (n == 2) return "2nd";
  if (n == 3) return "3rd";
  return n + "th";
}
