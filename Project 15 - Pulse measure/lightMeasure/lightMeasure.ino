const int sensorPin = A0;

const int smoothingAmount = 10;
int readings[smoothingAmount];
int readIndex = 0;
long total = 0;
int averageValue = 0;

void setup() {
  Serial.begin(9600);

  for (int i = 0; i < smoothingAmount; i++) {
    readings[i] = analogRead(sensorPin);
    total += readings[i];
  }
}

void loop() {
  int rawValue = analogRead(sensorPin);

  total -= readings[readIndex];
  readings[readIndex] = rawValue;
  total += readings[readIndex];

  readIndex++;
  if (readIndex >= smoothingAmount) {
    readIndex = 0;
  }

  averageValue = total / smoothingAmount;

  Serial.print("Light:");
  Serial.print(averageValue);
  Serial.print(" ");
  Serial.print("Super Bright:");
  Serial.print(0);
  Serial.print(" ");
  Serial.print("Complete Darkness:");
  Serial.println(900);

  delay(30);
}
