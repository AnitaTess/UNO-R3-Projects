const int sensorPin = A0;
const int resetButtonPin = 7;

int baselineValue = 0;

// Button debounce
bool lastButtonReading = HIGH;
bool buttonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

unsigned long lastTestTime = 0;
const unsigned long testInterval = 1000;

void setup() {
  Serial.begin(9600);

  pinMode(resetButtonPin, INPUT_PULLUP);

  Serial.println("=== KY-039 Material Transparency Tester ===");
  Serial.println("Leave the sensor uncovered.");
  Serial.println("Setting baseline in 2 seconds...");
  Serial.println();

  delay(2000);
  setBaseline();

  Serial.println("Place a material over the sensor.");
  Serial.println("Press the button to reset baseline anytime.");
  Serial.println();
}

void loop() {
  handleResetButton();

  if (millis() - lastTestTime >= testInterval) {
    testMaterial();
    lastTestTime = millis();
  }
}

void handleResetButton() {
  bool reading = digitalRead(resetButtonPin);

  if (reading != lastButtonReading) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      // Button pressed
      if (buttonState == LOW) {
        setBaseline();
      }
    }
  }

  lastButtonReading = reading;
}

int getAverageReading() {
  long total = 0;
  const int samples = 30;

  for (int i = 0; i < samples; i++) {
    total += analogRead(sensorPin);
    delay(10);
  }

  return total / samples;
}

void setBaseline() {
  Serial.println("------------------------------");
  Serial.println("Resetting baseline...");
  Serial.println("Make sure the sensor is UNCOVERED.");

  delay(700);

  baselineValue = getAverageReading();

  Serial.print("New baseline: ");
  Serial.println(baselineValue);
  Serial.println("------------------------------");
  Serial.println();
}

void testMaterial() {
  int materialValue = getAverageReading();
  int difference = materialValue - baselineValue;

  Serial.print("Baseline: ");
  Serial.print(baselineValue);

  Serial.print(" | Current: ");
  Serial.print(materialValue);

  Serial.print(" | Difference: ");
  Serial.print(difference);

  Serial.print(" | Result: ");

  /*
    For your KY-039 setup:
    Bigger value usually means more light is blocked.
  */

  if (difference < -500) {
  Serial.println("Tissue");
}
else if (difference < -240) {
  Serial.println("Foam");
}
else if (difference > -239 && difference < 40) {
  Serial.println("No material detected");
}
  else if (difference < 70 && difference > 41) {
    Serial.println("Finger");
  } 
  else if (difference < 116) {
    Serial.println("Foil");
  } 
  else {
    Serial.println("Plastic");
  }
}
