import processing.serial.*;

Serial myPort;

int angle = 90;
int distance = 999;

int maxDistance = 35;
float radarRadius = 400;

void setup() {
  size(1000, 650);
  smooth();

  println(Serial.list());

  // Change [0] if needed
  myPort = new Serial(this, Serial.list()[0], 9600);
  myPort.bufferUntil('\n');

  textSize(18);
}

void draw() {
  background(0);

  float centerX = width / 2.0;
  float centerY = height * 0.88;

  drawRadarGrid(centerX, centerY);
  drawSweepLine(centerX, centerY);
  drawCurrentTarget(centerX, centerY);
  drawInfo();
}

void serialEvent(Serial p) {
  String inData = trim(p.readStringUntil('\n'));

  if (inData != null && inData.indexOf(",") > -1) {
    String[] parts = split(inData, ',');

    if (parts.length == 2) {
      angle = int(parts[0]);
      distance = int(parts[1]);
    }
  }
}

void drawRadarGrid(float cx, float cy) {
  stroke(0, 180, 0);
  strokeWeight(2);
  noFill();

  for (int i = 1; i <= 5; i++) {
    float r = (radarRadius / 5.0) * i;
    arc(cx, cy, r * 2, r * 2, PI, TWO_PI);
  }

  line(cx - radarRadius, cy, cx + radarRadius, cy);

  for (int a = 0; a <= 180; a += 30) {
    float visualAngle = map(a, 0, 180, 180, 0);
    float rad = radians(visualAngle);

    float x = cx + cos(rad) * radarRadius;
    float y = cy - sin(rad) * radarRadius;

    line(cx, cy, x, y);
  }

  fill(0, 255, 0);
  textAlign(CENTER);

  for (int i = 1; i <= 5; i++) {
    int label = (maxDistance / 5) * i;
    float r = (radarRadius / 5.0) * i;
    text(label + "cm", cx + r, cy + 25);
  }
}

void drawSweepLine(float cx, float cy) {
  float visualAngle = map(angle, 0, 180, 180, 0);
  float rad = radians(visualAngle);

  float x = cx + cos(rad) * radarRadius;
  float y = cy - sin(rad) * radarRadius;

  stroke(0, 255, 0);
  strokeWeight(3);
  line(cx, cy, x, y);
}

void drawCurrentTarget(float cx, float cy) {
  if (distance <= maxDistance) {
    float visualAngle = map(angle, 0, 180, 180, 0);
    float rad = radians(visualAngle);

    float r = map(distance, 0, maxDistance, 0, radarRadius);

    float x = cx + cos(rad) * r;
    float y = cy - sin(rad) * r;

    noStroke();
    fill(255, 0, 0);
    ellipse(x, y, 20, 20);
  }
}

void drawInfo() {
  fill(0, 255, 0);
  textAlign(LEFT);

  text("Angle: " + angle + "°", 30, 30);

  if (distance <= maxDistance) {
    text("Distance: " + distance + " cm", 30, 60);
  } else {
    text("Distance: out of range", 30, 60);
  }

  text("Radar range: 0-" + maxDistance + " cm", 30, 90);
}
