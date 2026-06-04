import processing.serial.*;

Serial arduino;

String portName = "COM3"; // Change to your Arduino Uno port
String sensorData = "00000";

boolean[] touched = new boolean[5];
float[] bend = new float[5];

String[] fingerNames = {"Thumb", "Index", "Middle", "Ring", "Pinky"};

void setup() {
  size(1000, 720);
  smooth(8);

  println(Serial.list());

  arduino = new Serial(this, portName, 9600);
  arduino.bufferUntil('\n');

  textAlign(CENTER, CENTER);
}

void draw() {
  background(18, 23, 31);

  readTouches();
  updateBends();

  drawHand();
  drawSensorStatus();
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

void updateBends() {
  for (int i = 0; i < 5; i++) {
    float target = touched[i] ? 1.0 : 0.0;
    bend[i] = lerp(bend[i], target, 0.16);
  }
}

void drawHand() {
  pushMatrix();
  translate(width / 2, height - 105);

  drawGlow();

  // Fingers are drawn first so their bases sit behind the palm
drawMainFinger(-95, -380, 150, 42, bend[1], touched[1]); // Index, S2
drawMainFinger(-25, -400, 170, 44, bend[2], touched[2]); // Middle, S3
drawMainFinger(45, -385, 150, 42, bend[3], touched[3]);  // Ring, S4
drawMainFinger(110, -355, 120, 38, bend[4], touched[4]); // Pinky, S5

  drawThumb(-150, -220, bend[0], touched[0]);              // Thumb, S1

  drawPalm();
  drawWrist();

  popMatrix();
}

void drawGlow() {
  noStroke();
  fill(255, 255, 255, 10);
  ellipse(0, -270, 520, 520);
}

void drawPalm() {
  rectMode(CENTER);
  noStroke();

  // outer palm
  fill(205, 210, 216);
  rect(0, -190, 320, 270, 70);

  // inner palm highlight
  fill(228, 232, 235);
  rect(0, -205, 285, 230, 58);

  // soft palm outline
  noFill();
  stroke(255, 255, 255, 130);
  strokeWeight(4);
  rect(0, -205, 285, 230, 58);
}

void drawWrist() {
  rectMode(CENTER);
  noStroke();

  fill(175, 182, 190);
  rect(0, -15, 135, 135, 35);

  fill(205, 210, 216);
  rect(0, -55, 170, 95, 38);

  noFill();
  stroke(255, 255, 255, 90);
  strokeWeight(3);
  rect(0, -55, 170, 95, 38);
}

void drawMainFinger(float x, float baseY, float len, float w, float b, boolean active) {
  /*
    This creates a "bend backward" effect:
    - straight finger is full length
    - touched finger becomes visually shorter
    - fingertip moves downward toward the palm
    - finger stays in the same lane instead of bending sideways
  */

  float straightTipY = baseY - len;
  float bentTipY = baseY - len * 0.45;

  float tipY = lerp(straightTipY, bentTipY, b);

  float joint1Y = lerp(baseY - len * 0.33, baseY - len * 0.24, b);
  float joint2Y = lerp(baseY - len * 0.66, baseY - len * 0.36, b);

  // small curve when bent, but not sideways
  float curve = 18 * b;

  PVector p0 = new PVector(x, baseY);
  PVector p1 = new PVector(x - curve * 0.25, joint1Y);
  PVector p2 = new PVector(x + curve * 0.25, joint2Y);
  PVector p3 = new PVector(x, tipY);

  if (active) {
    stroke(99, 229, 166);
  } else {
    stroke(225, 230, 235);
  }

  strokeWeight(w);
  strokeCap(ROUND);
  noFill();

  bezier(p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);

  // finger highlight
  stroke(255, 255, 255, 70);
  strokeWeight(w * 0.28);
  bezier(p0.x - w * 0.12, p0.y, p1.x - w * 0.12, p1.y, p2.x - w * 0.12, p2.y, p3.x - w * 0.12, p3.y);

  // joints
  drawFingerJoint(x, joint1Y, w * 0.55, active);
  drawFingerJoint(x, joint2Y, w * 0.55, active);

  // fingertip
  drawFingerTip(x, tipY, w * 0.8, active);
}

void drawThumb(float baseX, float baseY, float b, boolean active) {
  float len = 120;
  float w = 42;

  float tipX = lerp(baseX - 100, baseX - 60, b);
  float tipY = lerp(baseY - 65, baseY - 25, b);

  float c1x = lerp(baseX - 45, baseX - 35, b);
  float c1y = lerp(baseY - 45, baseY - 25, b);

  float c2x = lerp(baseX - 80, baseX - 55, b);
  float c2y = lerp(baseY - 75, baseY - 35, b);

  if (active) {
    stroke(99, 229, 166);
  } else {
    stroke(225, 230, 235);
  }

  strokeWeight(w);
  strokeCap(ROUND);
  noFill();

  bezier(baseX, baseY, c1x, c1y, c2x, c2y, tipX, tipY);

  stroke(255, 255, 255, 70);
  strokeWeight(w * 0.25);
  bezier(baseX - 5, baseY - 5, c1x - 5, c1y - 5, c2x - 5, c2y - 5, tipX - 5, tipY - 5);

  drawFingerJoint((baseX + tipX) / 2, (baseY + tipY) / 2, w * 0.55, active);
  drawFingerTip(tipX, tipY, w * 0.78, active);
}

void drawFingerJoint(float x, float y, float size, boolean active) {
  noStroke();

  if (active) {
    fill(75, 190, 135, 190);
  } else {
    fill(190, 198, 205, 180);
  }

  ellipse(x, y, size, size);
}

void drawFingerTip(float x, float y, float size, boolean active) {
  noStroke();

  if (active) {
    fill(135, 245, 190);
  } else {
    fill(245);
  }

  ellipse(x, y, size, size);
}

void drawSensorStatus() {
  int startX = 170;
  int y = height - 45;
  int gap = 165;

  for (int i = 0; i < 5; i++) {
    int x = startX + i * gap;

    fill(230);
    textSize(16);
    text(fingerNames[i], x, y - 44);

    if (touched[i]) {
      fill(99, 229, 166);
    } else {
      fill(50, 58, 68);
    }

    noStroke();
    ellipse(x, y, 50, 50);

    stroke(255, 255, 255, 160);
    strokeWeight(2);
    noFill();
    ellipse(x, y, 50, 50);

    fill(255);
    textSize(18);
    text("S" + (i + 1), x, y);

    if (touched[i]) {
      fill(135, 245, 190);
      textSize(12);
      text("TOUCHED", x, y + 42);
    }
  }
}
