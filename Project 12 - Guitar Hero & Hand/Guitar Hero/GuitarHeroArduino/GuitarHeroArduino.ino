const int touchPins[5] = {2, 3, 4, 5, 6};

void setup() {
  Serial.begin(9600);

  for (int i = 0; i < 5; i++) {
    pinMode(touchPins[i], INPUT);
  }
}

void loop() {
  for (int i = 0; i < 5; i++) {
    int state = digitalRead(touchPins[i]);

    if (state == HIGH) {
      Serial.print("1");
    } else {
      Serial.print("0");
    }
  }

  Serial.println();
  delay(30);
}
