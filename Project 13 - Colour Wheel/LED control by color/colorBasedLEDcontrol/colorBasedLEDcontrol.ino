// TCS230 / TCS3200 pins
const int S0 = 4;
const int S1 = 5;
const int S2 = 6;
const int S3 = 7;
const int sensorOut = 8;

// RGB LED pins
const int redLedPin = 10;
const int greenLedPin = 11;
const int blueLedPin = 3;

// Set to true if your RGB LED is common anode
// Set to false if your RGB LED is common cathode
const bool COMMON_ANODE = false;

int redValue;
int greenValue;
int blueValue;

String lastDetectedColour = "";

void setup() {
  Serial.begin(9600);

  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);

  pinMode(redLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  pinMode(blueLedPin, OUTPUT);

  // Frequency scaling: 20%
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

  turnLedOff();

  Serial.println("TCS3200 colour-controlled RGB LED started");
}

void loop() {
  redValue = readRed();
  greenValue = readGreen();
  blueValue = readBlue();

  String detectedColour = detectColour(redValue, greenValue, blueValue);

  // If unknown, do nothing. Keep the LED in its current state.
  if (detectedColour == "Unknown") {
    delay(250);
    return;
  }

  // Only react when colour changes
  if (detectedColour != lastDetectedColour) {
    Serial.print("R: ");
    Serial.print(redValue);
    Serial.print(" | G: ");
    Serial.print(greenValue);
    Serial.print(" | B: ");
    Serial.print(blueValue);
    Serial.print("  -->  ");
    Serial.println(detectedColour);

    applyColourToLed(detectedColour);

    lastDetectedColour = detectedColour;
  }

  delay(250);
}

int readRed() {
  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);
  delay(50);
  return pulseIn(sensorOut, LOW);
}

int readGreen() {
  digitalWrite(S2, HIGH);
  digitalWrite(S3, HIGH);
  delay(50);
  return pulseIn(sensorOut, LOW);
}

int readBlue() {
  digitalWrite(S2, LOW);
  digitalWrite(S3, HIGH);
  delay(50);
  return pulseIn(sensorOut, LOW);
}

String detectColour(int r, int g, int b) {
  /*
    Calibrated for your TCS230/TCS3200 readings.
    Lower value = stronger detected colour.
  */

  // White
  if (r >= 10 && r <= 45 &&
      g >= 15 && g <= 45 &&
      b >= 10 && b <= 60 &&
      abs(r - g) < 25 &&
      abs(g - b) < 30) {
    return "White";
  }

  // Black
  if (r >= 200 && g >= 220 && b >= 190) {
    return "Black";
  }

  // Yellow: red lowest, green middle, blue highest
  if (r >= 25 && r <= 50 &&
      g >= 45 && g <= 75 &&
      b >= 70 && b <= 110 &&
      r < g &&
      g < b) {
    return "Yellow";
  }

  // Orange
  if (r >= 40 && r <= 70 &&
      g >= 105 && g <= 140 &&
      b >= 105 && b <= 145) {
    return "Orange";
  }

  // Red
  if (r >= 45 && r <= 75 &&
      g >= 130 && g <= 170 &&
      b >= 95 && b <= 135) {
    return "Red";
  }

  // Purple
  if (r >= 75 && r <= 110 &&
      g >= 120 && g <= 160 &&
      b >= 75 && b <= 110) {
    return "Purple";
  }

  // Green
  if (r >= 105 && r <= 145 &&
      g >= 85 && g <= 125 &&
      b >= 90 && b <= 130 &&
      g < r &&
      g <= b + 15) {
    return "Green";
  }

  // Blue: blue clearly lowest
  if (r >= 95 && r <= 160 &&
      g >= 80 && g <= 150 &&
      b >= 35 && b <= 100 &&
      b < r - 25 &&
      b < g - 20) {
    return "Blue";
  }

  return "Unknown";
}

void applyColourToLed(String colour) {
  if (colour == "White") {
    setRGB(255, 255, 255);
  } 
  else if (colour == "Yellow") {
    setRGB(255, 180, 0);
  } 
  else if (colour == "Orange") {
    setRGB(255, 70, 0);
  } 
  else if (colour == "Red") {
    setRGB(255, 0, 0);
  } 
  else if (colour == "Purple") {
    setRGB(180, 0, 255);
  } 
  else if (colour == "Green") {
    setRGB(0, 255, 0);
  } 
  else if (colour == "Blue") {
    setRGB(0, 0, 255);
  } 
  else if (colour == "Black") {
    turnLedOff();
  }
}

void setRGB(int redBrightness, int greenBrightness, int blueBrightness) {
  if (COMMON_ANODE) {
    redBrightness = 255 - redBrightness;
    greenBrightness = 255 - greenBrightness;
    blueBrightness = 255 - blueBrightness;
  }

  analogWrite(redLedPin, redBrightness);
  analogWrite(greenLedPin, greenBrightness);
  analogWrite(blueLedPin, blueBrightness);
}

void turnLedOff() {
  setRGB(0, 0, 0);
}