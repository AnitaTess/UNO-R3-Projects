//run this sketch to test your Four Digital Seven Segment Display. 
//It tests rendering of all numbers 0-9 and all 4 slots at the same time.
int data = 8;
int latch = 9;
int clock = 10;

int digitPins[4] = {2, 3, 4, A5};

const int DIGIT_ON = LOW;
const int DIGIT_OFF = HIGH;

//this can differ for you, adjust if some numbers render incorrectly in all 4 slots.
unsigned char table[] = {
  0x3F, // 0
  0x06, // 1
  0x5B, // 2
  0x4F, // 3
  0x66, // 4
  0x6D, // 5
  0x7D, // 6
  0x07, // 7
  0x7F, // 8
  0x6F, // 9
  0x00  // blank
};

void setup() {
  pinMode(data, OUTPUT);
  pinMode(latch, OUTPUT);
  pinMode(clock, OUTPUT);

  for (int i = 0; i < 4; i++) {
    pinMode(digitPins[i], OUTPUT);
    digitalWrite(digitPins[i], DIGIT_OFF);
  }
}

void loop() {
  for (int number = 0; number <= 9; number++) {
    unsigned long start = millis();

    while (millis() - start < 1500) {
      showAllDigits(number);
    }

    clearDisplay();
    delay(300);
  }
}

void showAllDigits(int number) {
  for (int i = 0; i < 4; i++) {
    allDigitsOff();
    sendSegments(table[10]); // blank first

    sendSegments(table[number]);
    delayMicroseconds(300);

    digitalWrite(digitPins[i], DIGIT_ON);
    delay(3);

    digitalWrite(digitPins[i], DIGIT_OFF);
    sendSegments(table[10]); // blank after
  }
}

void sendSegments(byte value) {
  digitalWrite(latch, LOW);
  shiftOut(data, clock, MSBFIRST, value);
  digitalWrite(latch, HIGH);
}

void allDigitsOff() {
  for (int i = 0; i < 4; i++) {
    digitalWrite(digitPins[i], DIGIT_OFF);
  }
}

void clearDisplay() {
  allDigitsOff();
  sendSegments(table[10]);
}