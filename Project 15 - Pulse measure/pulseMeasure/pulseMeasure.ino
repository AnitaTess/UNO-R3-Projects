const int sensorPin = A0;

const int smoothing = 30;
int readings[smoothing];
int indexPos = 0;
long total = 0;

void setup() {
  Serial.begin(9600);

  for (int i = 0; i < smoothing; i++) {
    readings[i] = analogRead(sensorPin);
    total += readings[i];
  }
}

void loop() {
  int raw = analogRead(sensorPin);

  total -= readings[indexPos];
  readings[indexPos] = raw;
  total += readings[indexPos];

  indexPos++;
  if (indexPos >= smoothing) {
    indexPos = 0;
  }

  int baseline = total / smoothing;
  int pulseSignal = raw - baseline;

  // Increase this if the pulse is still too small
  int amplifiedPulse = pulseSignal * 10;

  // Serial Plotter output with forced scale
  Serial.print("pulse:");
  Serial.print(amplifiedPulse);
  Serial.print(" ");
  Serial.print("min:");
  Serial.print(-100);
  Serial.print(" ");
  Serial.print("max:");
  Serial.println(100);

  delay(10);
}
