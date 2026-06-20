// TCS230 / TCS3200 pins
const int S0 = 4;
const int S1 = 5;
const int S2 = 6;
const int S3 = 7;
const int sensorOut = 8;

// Buttons
const int prevButtonPin = 9;
const int nextButtonPin = 12;
const int scanButtonPin = 13;

int redValue;
int greenValue;
int blueValue;

bool scanning = false;

// Debounce
bool lastPrevReading = HIGH;
bool lastNextReading = HIGH;
bool lastScanReading = HIGH;

bool prevState = HIGH;
bool nextState = HIGH;
bool scanState = HIGH;

unsigned long lastPrevDebounce = 0;
unsigned long lastNextDebounce = 0;
unsigned long lastScanDebounce = 0;

const unsigned long debounceDelay = 50;

void setup() {
  Serial.begin(9600);

  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);

  pinMode(prevButtonPin, INPUT_PULLUP);
  pinMode(nextButtonPin, INPUT_PULLUP);
  pinMode(scanButtonPin, INPUT_PULLUP);

  // Frequency scaling: 20%
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

  Serial.println("READY");
}

void loop() {
  handleButton(prevButtonPin, lastPrevReading, prevState, lastPrevDebounce, "PREV");
  handleButton(nextButtonPin, lastNextReading, nextState, lastNextDebounce, "NEXT");
  handleScanButton();

  if (scanning) {
    scanUntilColourDetected();
  }
}

void handleButton(int pin, bool &lastReading, bool &state, unsigned long &lastDebounce, String command) {
  bool reading = digitalRead(pin);

  if (reading != lastReading) {
    lastDebounce = millis();
  }

  if ((millis() - lastDebounce) > debounceDelay) {
    if (reading != state) {
      state = reading;

      if (state == LOW && !scanning) {
        Serial.println(command);
      }
    }
  }

  lastReading = reading;
}

void handleScanButton() {
  bool reading = digitalRead(scanButtonPin);

  if (reading != lastScanReading) {
    lastScanDebounce = millis();
  }

  if ((millis() - lastScanDebounce) > debounceDelay) {
    if (reading != scanState) {
      scanState = reading;

      if (scanState == LOW && !scanning) {
        Serial.println("SCANNING");
        scanning = true;
      }
    }
  }

  lastScanReading = reading;
}

void scanUntilColourDetected() {
  redValue = readRed();
  greenValue = readGreen();
  blueValue = readBlue();

  String detectedColour = detectColour(redValue, greenValue, blueValue);

  if (detectedColour == "Unknown") {
    delay(150);
    return;
  }

  Serial.print("COLOR:");
  Serial.println(detectedColour);

  scanning = false;
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
    Calibrated for your current TCS230/TCS3200 setup.
    Lower value = stronger detected colour.
  */

  // White: all values low and close together
  if (r >= 10 && r <= 30 &&
      g >= 15 && g <= 35 &&
      b >= 10 && b <= 35 &&
      abs(r - g) <= 12 &&
      abs(r - b) <= 12 &&
      abs(g - b) <= 12) {
    return "White";
  }

  // Black
  if (r >= 200 && g >= 220 && b >= 190) {
    return "Black";
  }

  // Orange: red lowest, green and blue close together
  if (r >= 20 && r <= 85 &&
      g >= 55 && g <= 145 &&
      b >= 55 && b <= 145 &&
      r < g &&
      r < b &&
      abs(g - b) <= 18) {
    return "Orange";
  }

  // Yellow: red lowest, green middle, blue highest
  if (r >= 15 && r <= 55 &&
      g >= 30 && g <= 85 &&
      b >= 50 && b <= 120 &&
      r < g &&
      g < b &&
      b >= g + 15) {
    return "Yellow";
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

  // Blue
  if (r >= 95 && r <= 160 &&
      g >= 80 && g <= 150 &&
      b >= 35 && b <= 100 &&
      b < r - 25 &&
      b < g - 20) {
    return "Blue";
  }

  return "Unknown";
}