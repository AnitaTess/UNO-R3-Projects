#include <Wire.h>

// GY-521 / MPU6050 I2C address
const int MPU_ADDR = 0x68;

// Raw accelerometer values
int16_t ax, ay, az;

// Baseline values when wand is still
long baseX = 0;
long baseY = 0;
long baseZ = 0;

// Separate thresholds
int xFlickThreshold = 14000; // left/right harder to trigger
int yFlickThreshold = 9000;  // up/down easier

// Prevents one movement being detected many times
unsigned long lastGestureTime = 0;
const unsigned long cooldown = 700;

void setup() {
  Serial.begin(9600);
  Wire.begin();

  // Wake up MPU6050
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  Serial.println("WAND_READYING");
  delay(1500);

  calibrateSensor();

  Serial.println("WAND_READY");
}

void loop() {
  readMPU6050();

  int xMove = ax - baseX;
  int yMove = ay - baseY;

  if (millis() - lastGestureTime < cooldown) {
    delay(50);
    return;
  }

  if (xMove > xFlickThreshold) {
    Serial.println("RIGHT");
    lastGestureTime = millis();
  }
  else if (xMove < -xFlickThreshold) {
    Serial.println("LEFT");
    lastGestureTime = millis();
  }
  else if (yMove > yFlickThreshold) {
    Serial.println("UP");
    lastGestureTime = millis();
  }
  else if (yMove < -yFlickThreshold) {
    Serial.println("DOWN");
    lastGestureTime = millis();
  }

  delay(50);
}

void calibrateSensor() {
  long totalX = 0;
  long totalY = 0;
  long totalZ = 0;

  for (int i = 0; i < 100; i++) {
    readMPU6050();

    totalX += ax;
    totalY += ay;
    totalZ += az;

    delay(10);
  }

  baseX = totalX / 100;
  baseY = totalY / 100;
  baseZ = totalZ / 100;
}

void readMPU6050() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 6, true);

  ax = Wire.read() << 8 | Wire.read();
  ay = Wire.read() << 8 | Wire.read();
  az = Wire.read() << 8 | Wire.read();
}