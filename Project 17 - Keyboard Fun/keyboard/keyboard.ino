#include <Wire.h>

// PCF8574 I2C addresses from your scanner
const byte PCF1 = 0x20;
const byte PCF2 = 0x21;

// 12 buttons directly on Uno
const int unoButtonPins[12] = {
  2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
};

// 26 letters total: A-Z
const char letters[26] = {
  'A', 'B', 'C', 'D', 'E', 'F',
  'G', 'H', 'I', 'J', 'K', 'L',
  'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
  'U', 'V', 'W', 'X', 'Y', 'Z'
};

bool previousButtonState[26];
bool previousSpaceState = false;

String typedText = "";

void setup() {
  Serial.begin(9600);
  Wire.begin();

  // Uno buttons use internal pullups
  for (int i = 0; i < 12; i++) {
    pinMode(unoButtonPins[i], INPUT_PULLUP);
  }

  // Set PCF8574 pins HIGH so they behave like inputs
  Wire.beginTransmission(PCF1);
  Wire.write(0xFF);
  Wire.endTransmission();

  Wire.beginTransmission(PCF2);
  Wire.write(0xFF);
  Wire.endTransmission();

  for (int i = 0; i < 26; i++) {
    previousButtonState[i] = false;
  }

  Serial.println("=== 26 Button A-Z Keyboard ===");
  Serial.println("Buttons mapped A-Z.");
  Serial.println("Press buttons to type.");
  Serial.println();
}

void loop() {
  checkUnoButtons();
  checkPCF1Buttons();
  checkPCF2Buttons();

  delay(20); // small debounce help
}

void checkUnoButtons() {
  // Uno D2-D13 = A-L = indexes 0-11
  for (int i = 0; i < 12; i++) {
    bool pressed = digitalRead(unoButtonPins[i]) == LOW;
    handleButton(i, pressed);
  }
}

void checkPCF1Buttons() {
  byte data = readPCF(PCF1);

  // PCF1 P0-P7 = M-T = indexes 12-19
  for (int pin = 0; pin < 8; pin++) {
    bool pressed = bitRead(data, pin) == 0;
    handleButton(12 + pin, pressed);
  }
}

void checkPCF2Buttons() {
  byte data = readPCF(PCF2);

  // PCF2 P0-P5 = U-Z = indexes 20-25
  for (int pin = 0; pin < 6; pin++) {
    bool pressed = bitRead(data, pin) == 0;
    handleButton(20 + pin, pressed);
  }

  // PCF2 P6 = space bar
  bool spacePressed = bitRead(data, 6) == 0;
  handleSpaceButton(spacePressed);
}

byte readPCF(byte address) {
  Wire.requestFrom(address, 1);

  if (Wire.available()) {
    return Wire.read();
  }

  return 0xFF; // treat all as not pressed if no data
}

void handleSpaceButton(bool pressed) {
  if (pressed && !previousSpaceState) {
    typedText += ' ';
    Serial.println(typedText);
  }

  previousSpaceState = pressed;
}

void handleButton(int index, bool pressed) {
  // Only type once when button changes from not pressed to pressed
  if (pressed && !previousButtonState[index]) {
    char letter = letters[index];

    typedText += letter;

    Serial.println(typedText);
  }

  previousButtonState[index] = pressed;
}