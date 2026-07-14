const int flameDigitalPin = 2;
const int flameAnalogPin = A0;

// Most flame sensor modules output LOW when flame is detected.
// If yours works backwards, change this to HIGH.
const int FLAME_DETECTED = LOW;

void setup() {
  Serial.begin(9600);

  pinMode(flameDigitalPin, INPUT);

  Serial.println("Flame sensor test started");
  Serial.println("Show a small flame in front of the sensor to test.");
  Serial.println();
}

void loop() {
  int digitalValue = digitalRead(flameDigitalPin);
  int analogValue = analogRead(flameAnalogPin);

  if (digitalValue == FLAME_DETECTED) {
    Serial.print("FIRE");
  } else {
    Serial.print("No fire");
  }

  Serial.print(" | Digital: ");
  Serial.print(digitalValue);

  Serial.print(" | Analog: ");
  Serial.println(analogValue);

  delay(300);
}
