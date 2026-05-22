import processing.serial.*;

Serial myPort;

int gridX = 1;
int gridY = 1;

float flareX;
float flareY;
float targetX;
float targetY;

float[][] pointsX = new float[3][3];
float[][] pointsY = new float[3][3];

ArrayList<Particle> particles = new ArrayList<Particle>();

void setup() {
  size(900, 650);
  smooth();

  println(Serial.list());

  // Change [0] to [1], [2], etc. if needed
  myPort = new Serial(this, Serial.list()[0], 9600);
  myPort.bufferUntil('\n');

  setupInvisibleGrid();

  flareX = pointsX[gridX][gridY];
  flareY = pointsY[gridX][gridY];
  targetX = flareX;
  targetY = flareY;
}

void draw() {
  background(0);

  updateFlare();
  drawParticles();
  drawFlare();
}

void serialEvent(Serial p) {
  String command = trim(p.readStringUntil('\n'));

  if (command == null) return;

 if (command.equals("LEFT")) {
  gridX = min(2, gridX + 1); // move visually right
  moveFlare();
}
else if (command.equals("RIGHT")) {
  gridX = max(0, gridX - 1); // move visually left
  moveFlare();
}
  else if (command.equals("UP")) {
    gridY = max(0, gridY - 1);
    moveFlare();
  }
  else if (command.equals("DOWN")) {
    gridY = min(2, gridY + 1);
    moveFlare();
  }
}

void setupInvisibleGrid() {
  float left = 160;
  float right = width - 160;
  float top = 120;
  float bottom = height - 120;

  for (int x = 0; x < 3; x++) {
    for (int y = 0; y < 3; y++) {
      pointsX[x][y] = map(x, 0, 2, left, right);
      pointsY[x][y] = map(y, 0, 2, top, bottom);
    }
  }
}

void moveFlare() {
  targetX = pointsX[gridX][gridY];
  targetY = pointsY[gridX][gridY];

  for (int i = 0; i < 35; i++) {
    particles.add(new Particle(flareX, flareY));
  }
}

void updateFlare() {
  flareX = lerp(flareX, targetX, 0.12);
  flareY = lerp(flareY, targetY, 0.12);
}

void drawFlare() {
  noStroke();

  // soft outer glow
  fill(255, 40, 0, 25);
  ellipse(flareX, flareY, 190, 190);

  fill(255, 0, 0, 45);
  ellipse(flareX, flareY, 130, 130);

  fill(255, 90, 0, 100);
  ellipse(flareX, flareY, 80, 80);

  fill(255, 190, 60, 190);
  ellipse(flareX, flareY, 45, 45);

  fill(255, 255, 190);
  ellipse(flareX, flareY, 18, 18);
}

void drawParticles() {
  for (int i = particles.size() - 1; i >= 0; i--) {
    Particle p = particles.get(i);
    p.update();
    p.draw();

    if (p.life <= 0) {
      particles.remove(i);
    }
  }
}

class Particle {
  float x;
  float y;
  float vx;
  float vy;
  float life;
  float size;

  Particle(float startX, float startY) {
    x = startX;
    y = startY;
    vx = random(-5, 5);
    vy = random(-5, 5);
    life = 255;
    size = random(4, 12);
  }

  void update() {
    x += vx;
    y += vy;

    vx *= 0.94;
    vy *= 0.94;

    life -= 7;
    size *= 0.97;
  }

  void draw() {
    noStroke();

    fill(255, random(80, 220), 0, life);
    ellipse(x, y, size, size);

    fill(255, 255, 180, life * 0.6);
    ellipse(x, y, size * 0.35, size * 0.35);
  }
}
