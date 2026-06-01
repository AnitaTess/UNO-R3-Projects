#include <LiquidCrystal.h>
#include <Servo.h>

// LCD pins
LiquidCrystal lcd(5, 6, 7, 11, 12, 13);

// Servo
Servo coverServo;
const int servoPin = A3;

const int coverClosedAngle = 180;    // start timer / hide display
const int coverOpenAngle = 0;    // stop timer / reveal display

// --------------------
// 74HC595 display pins
// --------------------
int data = 8;
int latch = 9;
int clock = 10;

int digitPins[4] = {2, 3, 4, A5};

const int DIGIT_ON = LOW;
const int DIGIT_OFF = HIGH;

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

// --------------------
// Button + LEDs
// --------------------
const int buttonPin = A0;

const int redLed = A1;
const int greenLed = A2;

// --------------------
// Game state
// --------------------
bool timerRunning = false;

unsigned long startTime = 0;
unsigned long stoppedTime = 0;

unsigned long targetTime = 5000; // ms

// NOTE: 3000 = 3 seconds tolerance.
// Now set for 0.3 seconds tolerance
const unsigned long tolerance = 300;

int score = 0;

bool showingResult = false;
unsigned long resultShownAt = 0;

// Button debounce
bool lastButtonState = HIGH;
unsigned long lastButtonPress = 0;
const unsigned long buttonCooldown = 250;

void setup() {
  lcd.begin(16, 2);

  coverServo.attach(servoPin);
  coverServo.write(coverOpenAngle);

  pinMode(data, OUTPUT);
  pinMode(latch, OUTPUT);
  pinMode(clock, OUTPUT);

  for (int i = 0; i < 4; i++) {
    pinMode(digitPins[i], OUTPUT);
    digitalWrite(digitPins[i], DIGIT_OFF);
  }

  pinMode(buttonPin, INPUT_PULLUP);

  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);

  digitalWrite(redLed, LOW);
  digitalWrite(greenLed, LOW);

  randomSeed(analogRead(A4));

  lcd.clear();
  lcd.print("Reaction Timer");
  lcd.setCursor(0, 1);
  lcd.print("Press to start");

  delay(1500);

  newChallenge();
}

void loop() {
  handleButton();

  unsigned long displayTime;

  if (timerRunning) {
    displayTime = millis() - startTime;
  } else {
    displayTime = stoppedTime;
  }

  refreshTimeDisplay(displayTime);

  if (showingResult && millis() - resultShownAt > 1800) {
    showingResult = false;
    digitalWrite(redLed, LOW);
    digitalWrite(greenLed, LOW);
    newChallenge();
  }
}

// --------------------
// Game logic
// --------------------

void newChallenge() {
  //now max random time generated & requested is 15 seconds
  int halfSecondUnits = random(1, 31);
  // 30 × 500ms = 15000ms = 15 seconds
  targetTime = halfSecondUnits * 500UL;

  timerRunning = false;
  stoppedTime = 0;

  coverServo.write(coverOpenAngle);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Stop at: ");
  printTimeToLCD(targetTime);

  lcd.setCursor(0, 1);
  lcd.print("Score: ");
  lcd.print(score);
  lcd.print(" Press!");

  clearDisplay();
}

void startTimer() {
  coverServo.write(coverClosedAngle);

  timerRunning = true;
  showingResult = false;
  stoppedTime = 0;
  startTime = millis();

  digitalWrite(redLed, LOW);
  digitalWrite(greenLed, LOW);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Target: ");
  printTimeToLCD(targetTime);

  lcd.setCursor(0, 1);
  lcd.print("Timer running");
}

void stopTimer() {
  coverServo.write(coverOpenAngle);

  timerRunning = false;
  stoppedTime = millis() - startTime;

  long difference = abs((long)stoppedTime - (long)targetTime);

  lcd.clear();

  if (difference <= tolerance) {
    score++;

    digitalWrite(greenLed, HIGH);
    digitalWrite(redLed, LOW);

    lcd.setCursor(0, 0);
    lcd.print("PERFECT! +1");
  } else {
    score = 0;

    digitalWrite(redLed, HIGH);
    digitalWrite(greenLed, LOW);

    lcd.setCursor(0, 0);
    lcd.print("Missed!");
  }

  lcd.setCursor(0, 1);
  lcd.print("You: ");
  printTimeToLCD(stoppedTime);
  lcd.print(" S:");
  lcd.print(score);

  showingResult = true;
  resultShownAt = millis();
}

// --------------------
// Button
// --------------------

void handleButton() {
  bool currentButtonState = digitalRead(buttonPin);

  if (
    lastButtonState == HIGH &&
    currentButtonState == LOW &&
    millis() - lastButtonPress > buttonCooldown &&
    !showingResult
  ) {
    lastButtonPress = millis();

    if (!timerRunning) {
      startTimer();
    } else {
      stopTimer();
    }
  }

  lastButtonState = currentButtonState;
}

// --------------------
// LCD time helper
// --------------------

void printTimeToLCD(unsigned long ms) {
  int tenths = (ms + 50) / 100;
  int seconds = tenths / 10;
  int decimal = tenths % 10;

  lcd.print(seconds);
  lcd.print(".");
  lcd.print(decimal);
  lcd.print("s");
}

// --------------------
// 7-segment display
// --------------------

void allDigitsOff() {
  for (int i = 0; i < 4; i++) {
    digitalWrite(digitPins[i], DIGIT_OFF);
  }
}

void sendSegments(byte value) {
  digitalWrite(latch, LOW);
  shiftOut(data, clock, MSBFIRST, value);
  digitalWrite(latch, HIGH);
}

void clearDisplay() {
  allDigitsOff();
  sendSegments(table[10]);
}

void refreshTimeDisplay(unsigned long ms) {
  int tenths = (ms + 50) / 100;

//Display timer can keep counting up to 60.0s
  if (tenths > 600) {
    tenths = 600;
  }

  int seconds = tenths / 10;
  int decimal = tenths % 10;

  int digits[4];

  digits[0] = seconds / 100;
  digits[1] = (seconds / 10) % 10;
  digits[2] = seconds % 10;
  digits[3] = decimal;

  for (int i = 0; i < 4; i++) {
    allDigitsOff();

    byte pattern;

    if (i == 0 && digits[0] == 0) {
      pattern = table[10];
    }
    else if (i == 1 && digits[0] == 0 && digits[1] == 0) {
      pattern = table[10];
    }
    else {
      pattern = table[digits[i]];
    }

    if (i == 2) {
      pattern = pattern | 0x80;
    }

    sendSegments(pattern);
    digitalWrite(digitPins[i], DIGIT_ON);
    delay(2);
  }

  allDigitsOff();
}