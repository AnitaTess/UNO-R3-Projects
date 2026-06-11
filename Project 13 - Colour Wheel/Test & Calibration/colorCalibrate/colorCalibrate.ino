const int S0 = 4;
const int S1 = 5;
const int S2 = 6;
const int S3 = 7;
const int sensorOut = 8;

int redValue = 0;
int greenValue = 0;
int blueValue = 0;

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

  Serial.println("TCS3200 colour sensor test started");
}

void loop() {
  redValue = readRed();
  greenValue = readGreen();
  blueValue = readBlue();

  Serial.print("Red: ");
  Serial.print(redValue);
  Serial.print(" | Green: ");
  Serial.print(greenValue);
  Serial.print(" | Blue: ");
  Serial.println(blueValue);

  delay(500);
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
