const int S0 = 4;
const int S1 = 5;
const int S2 = 6;
const int S3 = 7;
const int sensorOut = 8;

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

  // Frequency scaling: 20%
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

  Serial.println("TCS3200 colour detector started");
  Serial.println("Place a coloured object 1-3 cm in front of the sensor.");
}

void loop() {
  redValue = readRed();
  greenValue = readGreen();
  blueValue = readBlue();

  String detectedColour = detectColour(redValue, greenValue, blueValue);

  // Only print if a real colour is detected
  // and it is different from the previous colour
  if (detectedColour != "Unknown" && detectedColour != lastDetectedColour) {
    Serial.print("R: ");
    Serial.print(redValue);
    Serial.print(" | G: ");
    Serial.print(greenValue);
    Serial.print(" | B: ");
    Serial.print(blueValue);
    Serial.print("  -->  ");
    Serial.println(detectedColour);

    lastDetectedColour = detectedColour;
  }

  delay(300);
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

  // White: all values very low and close together
  if (r >= 10 && r <= 30 &&
      g >= 15 && g <= 35 &&
      b >= 10 && b <= 35) {
    return "White";
  }

  // Black: all values very high
  if (r >= 200 && g >= 220 && b >= 190) {
    return "Black";
  }

// Yellow: red is lowest, green is medium, blue is highest
if (r >= 25 && r <= 50 &&
    g >= 45 && g <= 75 &&
    b >= 70 && b <= 110 &&
    r < g &&
    g < b) {
  return "Yellow";
}

  // Orange: red low, green and blue around 120
  if (r >= 40 && r <= 65 &&
      g >= 105 && g <= 135 &&
      b >= 105 && b <= 140) {
    return "Orange";
  }

  // Red: red low, green very high, blue high
  if (r >= 45 && r <= 70 &&
      g >= 130 && g <= 165 &&
      b >= 95 && b <= 130) {
    return "Red";
  }

  // Purple: red and blue similar, green much higher
  if (r >= 75 && r <= 105 &&
      g >= 120 && g <= 155 &&
      b >= 75 && b <= 105) {
    return "Purple";
  }

  // Green: green is lowest, blue close-ish, red higher
  if (r >= 105 && r <= 140 &&
      g >= 90 && g <= 120 &&
      b >= 95 && b <= 125) {
    return "Green";
  }

 // Blue: blue clearly lowest
if (r >= 95 && r <= 155 &&
    g >= 80 && g <= 145 &&
    b >= 40 && b <= 95 &&
    b < r - 25 &&
    b < g - 20) {
  return "Blue";
}

  return "Unknown";
}