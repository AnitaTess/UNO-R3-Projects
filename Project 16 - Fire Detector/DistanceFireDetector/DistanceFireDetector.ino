// Flame sensor pins
const int flameDigitalPin = 2;
const int flameAnalogPin = A0;

// LED pins
const int whiteLedPin = 3;
const int greenLedPin = 4;
const int yellowLedPin = 5;
const int redLedPin = 6;

// Lower analog value = stronger/closer flame
const int noFireThreshold = 950;
const int farFlameThreshold = 700;
const int mediumFlameThreshold = 350;

void setup() {
  Serial.begin(9600);

  pinMode(flameDigitalPin, INPUT);

  pinMode(whiteLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  pinMode(yellowLedPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);

  showNoFire();

  Serial.println("=== Flame Intensity Monitor ===");
  Serial.println("White = no fire");
  Serial.println("Green = weak/far flame");
  Serial.println("Yellow = medium flame");
  Serial.println("Red = strong/close flame");
  Serial.println();
}

void loop() {
  int digitalValue = digitalRead(flameDigitalPin);
  int analogValue = analogRead(flameAnalogPin);

  Serial.print("Digital: ");
  Serial.print(digitalValue);
  Serial.print(" | Analog: ");
  Serial.print(analogValue);
  Serial.print(" | Status: ");

  if (analogValue >= noFireThreshold) {
    showNoFire();
    Serial.println("No fire");
  }
  else if (analogValue >= farFlameThreshold) {
    showFarFire();
    Serial.println("Far / weak flame");
  }
  else if (analogValue >= mediumFlameThreshold) {
    showMediumFire();
    Serial.println("Medium flame");
  }
  else {
    showCloseFire();
    Serial.println("Close / intense flame");
  }

  delay(200);
}

void turnAllLedsOff() {
  digitalWrite(whiteLedPin, LOW);
  digitalWrite(greenLedPin, LOW);
  digitalWrite(yellowLedPin, LOW);
  digitalWrite(redLedPin, LOW);
}

void showNoFire() {
  turnAllLedsOff();
  digitalWrite(whiteLedPin, HIGH);
}

void showFarFire() {
  turnAllLedsOff();
  digitalWrite(greenLedPin, HIGH);
}

void showMediumFire() {
  turnAllLedsOff();
  digitalWrite(yellowLedPin, HIGH);
}

void showCloseFire() {
  turnAllLedsOff();
  digitalWrite(redLedPin, HIGH);
}
