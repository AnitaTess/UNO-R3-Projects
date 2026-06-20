#include <avr/pgmspace.h>

// TCS230 / TCS3200 pins
const int S0 = 4;
const int S1 = 5;
const int S2 = 6;
const int S3 = 7;
const int sensorOut = 8;

// Button pin
const int buttonPin = 13;

// Game settings
const int winningScore = 10;
const int objectsPerColour = 15;

int redValue;
int greenValue;
int blueValue;

int score = 0;
bool waitingForButton = true;
bool scanning = false;
bool gameWon = false;

int targetColourIndex = -1;
char targetObject[24];

// Button debounce
bool lastButtonReading = HIGH;
bool buttonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// Colour indexes
const int GREEN = 0;
const int WHITE = 1;
const int BLACK = 2;
const int RED = 3;
const int YELLOW = 4;
const int ORANGE = 5;
const int BLUE = 6;
const int PURPLE = 7;

const char colourNames[][8] PROGMEM = {
  "Green", "White", "Black", "Red", "Yellow", "Orange", "Blue", "Purple"
};

// Objects stored in flash memory to avoid Uno RAM issues
const char obj_0_0[] PROGMEM = "grass";
const char obj_0_1[] PROGMEM = "asparagus";
const char obj_0_2[] PROGMEM = "spinach";
const char obj_0_3[] PROGMEM = "crocodile";
const char obj_0_4[] PROGMEM = "frog";
const char obj_0_5[] PROGMEM = "broccoli";
const char obj_0_6[] PROGMEM = "moss";
const char obj_0_7[] PROGMEM = "emerald";
const char obj_0_8[] PROGMEM = "basil";
const char obj_0_9[] PROGMEM = "clover";
const char obj_0_10[] PROGMEM = "cucumber";
const char obj_0_11[] PROGMEM = "lime";
const char obj_0_12[] PROGMEM = "pea";
const char obj_0_13[] PROGMEM = "lettuce";
const char obj_0_14[] PROGMEM = "bamboo";

const char obj_1_0[] PROGMEM = "paper";
const char obj_1_1[] PROGMEM = "snow";
const char obj_1_2[] PROGMEM = "wedding dress";
const char obj_1_3[] PROGMEM = "milk";
const char obj_1_4[] PROGMEM = "chalk";
const char obj_1_5[] PROGMEM = "salt";
const char obj_1_6[] PROGMEM = "sugar";
const char obj_1_7[] PROGMEM = "flour";
const char obj_1_8[] PROGMEM = "rice";
const char obj_1_9[] PROGMEM = "polar bear";
const char obj_1_10[] PROGMEM = "tissue";
const char obj_1_11[] PROGMEM = "yogurt";
const char obj_1_12[] PROGMEM = "cotton";
const char obj_1_13[] PROGMEM = "dove";
const char obj_1_14[] PROGMEM = "swan";

const char obj_2_0[] PROGMEM = "raven";
const char obj_2_1[] PROGMEM = "obsidian";
const char obj_2_2[] PROGMEM = "coal";
const char obj_2_3[] PROGMEM = "espresso";
const char obj_2_4[] PROGMEM = "shadow";
const char obj_2_5[] PROGMEM = "bats";
const char obj_2_6[] PROGMEM = "ants";
const char obj_2_7[] PROGMEM = "asphalt";
const char obj_2_8[] PROGMEM = "crow";
const char obj_2_9[] PROGMEM = "licorice";
const char obj_2_10[] PROGMEM = "sunglasses";
const char obj_2_11[] PROGMEM = "smoke";
const char obj_2_12[] PROGMEM = "void";
const char obj_2_13[] PROGMEM = "night";
const char obj_2_14[] PROGMEM = "dark chocolate";

const char obj_3_0[] PROGMEM = "apple";
const char obj_3_1[] PROGMEM = "cherry";
const char obj_3_2[] PROGMEM = "strawberry";
const char obj_3_3[] PROGMEM = "tomato";
const char obj_3_4[] PROGMEM = "ladybug";
const char obj_3_5[] PROGMEM = "poppy";
const char obj_3_6[] PROGMEM = "rose";
const char obj_3_7[] PROGMEM = "brick";
const char obj_3_8[] PROGMEM = "ketchup";
const char obj_3_9[] PROGMEM = "fire truck";
const char obj_3_10[] PROGMEM = "ruby";
const char obj_3_11[] PROGMEM = "chili pepper";
const char obj_3_12[] PROGMEM = "lipstick";
const char obj_3_13[] PROGMEM = "blood";
const char obj_3_14[] PROGMEM = "lobster";

const char obj_4_0[] PROGMEM = "sun";
const char obj_4_1[] PROGMEM = "cheese";
const char obj_4_2[] PROGMEM = "mustard";
const char obj_4_3[] PROGMEM = "lemon";
const char obj_4_4[] PROGMEM = "banana";
const char obj_4_5[] PROGMEM = "chick";
const char obj_4_6[] PROGMEM = "school bus";
const char obj_4_7[] PROGMEM = "corn";
const char obj_4_8[] PROGMEM = "sunflower";
const char obj_4_9[] PROGMEM = "daffodil";
const char obj_4_10[] PROGMEM = "butter";
const char obj_4_11[] PROGMEM = "dandelion";
const char obj_4_12[] PROGMEM = "French fries";
const char obj_4_13[] PROGMEM = "rubber duck";
const char obj_4_14[] PROGMEM = "taxi";

const char obj_5_0[] PROGMEM = "carrot";
const char obj_5_1[] PROGMEM = "basketball";
const char obj_5_2[] PROGMEM = "apricot";
const char obj_5_3[] PROGMEM = "goldfish";
const char obj_5_4[] PROGMEM = "tiger";
const char obj_5_5[] PROGMEM = "fox";
const char obj_5_6[] PROGMEM = "squirrel";
const char obj_5_7[] PROGMEM = "tangerine";
const char obj_5_8[] PROGMEM = "pumpkin";
const char obj_5_9[] PROGMEM = "clownfish";
const char obj_5_10[] PROGMEM = "amber";
const char obj_5_11[] PROGMEM = "peach";
const char obj_5_12[] PROGMEM = "papaya";
const char obj_5_13[] PROGMEM = "copper";
const char obj_5_14[] PROGMEM = "lava";

const char obj_6_0[] PROGMEM = "sky";
const char obj_6_1[] PROGMEM = "ocean";
const char obj_6_2[] PROGMEM = "blueberry";
const char obj_6_3[] PROGMEM = "sea";
const char obj_6_4[] PROGMEM = "denim";
const char obj_6_5[] PROGMEM = "jeans";
const char obj_6_6[] PROGMEM = "cobalt";
const char obj_6_7[] PROGMEM = "water";
const char obj_6_8[] PROGMEM = "moonstone";
const char obj_6_9[] PROGMEM = "Neptune";
const char obj_6_10[] PROGMEM = "aquamarine";
const char obj_6_11[] PROGMEM = "Uranus";
const char obj_6_12[] PROGMEM = "peacock";
const char obj_6_13[] PROGMEM = "morpho butterfly";
const char obj_6_14[] PROGMEM = "forget-me-nots";

const char obj_7_0[] PROGMEM = "eggplant";
const char obj_7_1[] PROGMEM = "lavender";
const char obj_7_2[] PROGMEM = "amethyst";
const char obj_7_3[] PROGMEM = "blackberry";
const char obj_7_4[] PROGMEM = "fig";
const char obj_7_5[] PROGMEM = "elderberry";
const char obj_7_6[] PROGMEM = "plum";
const char obj_7_7[] PROGMEM = "ube";
const char obj_7_8[] PROGMEM = "violets";
const char obj_7_9[] PROGMEM = "hyacinth";
const char obj_7_10[] PROGMEM = "iris";
const char obj_7_11[] PROGMEM = "prune";
const char obj_7_12[] PROGMEM = "raisin";
const char obj_7_13[] PROGMEM = "turnip";
const char obj_7_14[] PROGMEM = "thistle";

const char* const objects[8][15] PROGMEM = {
  {obj_0_0, obj_0_1, obj_0_2, obj_0_3, obj_0_4, obj_0_5, obj_0_6, obj_0_7, obj_0_8, obj_0_9, obj_0_10, obj_0_11, obj_0_12, obj_0_13, obj_0_14},
  {obj_1_0, obj_1_1, obj_1_2, obj_1_3, obj_1_4, obj_1_5, obj_1_6, obj_1_7, obj_1_8, obj_1_9, obj_1_10, obj_1_11, obj_1_12, obj_1_13, obj_1_14},
  {obj_2_0, obj_2_1, obj_2_2, obj_2_3, obj_2_4, obj_2_5, obj_2_6, obj_2_7, obj_2_8, obj_2_9, obj_2_10, obj_2_11, obj_2_12, obj_2_13, obj_2_14},
  {obj_3_0, obj_3_1, obj_3_2, obj_3_3, obj_3_4, obj_3_5, obj_3_6, obj_3_7, obj_3_8, obj_3_9, obj_3_10, obj_3_11, obj_3_12, obj_3_13, obj_3_14},
  {obj_4_0, obj_4_1, obj_4_2, obj_4_3, obj_4_4, obj_4_5, obj_4_6, obj_4_7, obj_4_8, obj_4_9, obj_4_10, obj_4_11, obj_4_12, obj_4_13, obj_4_14},
  {obj_5_0, obj_5_1, obj_5_2, obj_5_3, obj_5_4, obj_5_5, obj_5_6, obj_5_7, obj_5_8, obj_5_9, obj_5_10, obj_5_11, obj_5_12, obj_5_13, obj_5_14},
  {obj_6_0, obj_6_1, obj_6_2, obj_6_3, obj_6_4, obj_6_5, obj_6_6, obj_6_7, obj_6_8, obj_6_9, obj_6_10, obj_6_11, obj_6_12, obj_6_13, obj_6_14},
  {obj_7_0, obj_7_1, obj_7_2, obj_7_3, obj_7_4, obj_7_5, obj_7_6, obj_7_7, obj_7_8, obj_7_9, obj_7_10, obj_7_11, obj_7_12, obj_7_13, obj_7_14}
};

void setup() {
  Serial.begin(9600);

  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);

  pinMode(buttonPin, INPUT_PULLUP);

  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

  randomSeed(analogRead(A0));

  Serial.println(F("=== Colour Card Challenge ==="));
  Serial.println(F("Match the object to the correct colour card."));
  Serial.println(F("Press the button to scan your chosen card."));
  Serial.println(F("Score 10 correct scans to win."));
  Serial.println();

  nextRound();
}

void loop() {
  handleButton();

  if (gameWon) {
    return;
  }

  if (scanning) {
    scanUntilColourDetected();
  }
}

void handleButton() {
  bool reading = digitalRead(buttonPin);

  if (reading != lastButtonReading) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == LOW) {
        if (gameWon) {
          restartGame();
        } else if (waitingForButton) {
          Serial.println(F("Scanning..."));
          scanning = true;
          waitingForButton = false;
        }
      }
    }
  }

  lastButtonReading = reading;
}

void nextRound() {
  pickRandomObject();

  Serial.println(F("------------------------------"));
  Serial.print(F("Score: "));
  Serial.print(score);
  Serial.print(F("/"));
  Serial.println(winningScore);

  Serial.print(F("Object: "));
  Serial.println(targetObject);

  Serial.println(F("Choose the matching colour card."));
  Serial.println(F("Press the button to scan."));
  Serial.println();

  waitingForButton = true;
  scanning = false;
}

void scanUntilColourDetected() {
  redValue = readRed();
  greenValue = readGreen();
  blueValue = readBlue();

  int detectedColourIndex = detectColourIndex(redValue, greenValue, blueValue);

  if (detectedColourIndex == -1) {
    delay(150);
    return;
  }

  scanning = false;

  Serial.print(F("Detected: "));
  printColourName(detectedColourIndex);

  Serial.print(F(" | R: "));
  Serial.print(redValue);
  Serial.print(F(" G: "));
  Serial.print(greenValue);
  Serial.print(F(" B: "));
  Serial.println(blueValue);

  if (detectedColourIndex == targetColourIndex) {
    score++;
    Serial.println(F("Correct! +1 point"));
  } else {
    Serial.print(F("Wrong! Expected: "));
    printColourName(targetColourIndex);
    Serial.println();
  }

  if (score >= winningScore) {
    Serial.println();
    Serial.println(F("YOU WIN! Final score: 10/10"));
    Serial.println(F("Press the button to restart."));
    gameWon = true;
    waitingForButton = false;
  } else {
    Serial.println();
    delay(500);
    nextRound();
  }
}

void restartGame() {
  score = 0;
  gameWon = false;
  waitingForButton = false;
  scanning = false;

  Serial.println();
  Serial.println(F("=== New Game ==="));
  nextRound();
}

void pickRandomObject() {
  targetColourIndex = random(8);
  int objectIndex = random(objectsPerColour);

  strcpy_P(
    targetObject,
    (char*)pgm_read_word(&(objects[targetColourIndex][objectIndex]))
  );
}

void printColourName(int colourIndex) {
  char colourBuffer[10];
  strcpy_P(colourBuffer, colourNames[colourIndex]);
  Serial.print(colourBuffer);
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

int detectColourIndex(int r, int g, int b) {
 // White: all values low and very close together
if (r >= 10 && r <= 30 &&
    g >= 15 && g <= 35 &&
    b >= 10 && b <= 35 &&
    abs(r - g) <= 12 &&
    abs(r - b) <= 12 &&
    abs(g - b) <= 12) {
  return WHITE;
}

  // Black
  if (r >= 200 && g >= 220 && b >= 190) {
    return BLACK;
  }

  // Orange:
  // Red is lowest, green and blue are close together.
  // This catches your live orange readings like R: 26 G: 68 B: 72.
  if (r >= 20 && r <= 85 &&
      g >= 55 && g <= 145 &&
      b >= 55 && b <= 145 &&
      r < g &&
      r < b &&
      abs(g - b) <= 18) {
    return ORANGE;
  }

  // Yellow:
  // Red is lowest, green is middle, blue is clearly higher than green.
  if (r >= 25 && r <= 55 &&
      g >= 45 && g <= 85 &&
      b >= 75 && b <= 120 &&
      r < g &&
      g < b &&
      b >= g + 20) {
    return YELLOW;
  }

  // Red
  if (r >= 45 && r <= 75 &&
      g >= 130 && g <= 170 &&
      b >= 95 && b <= 135) {
    return RED;
  }

  // Purple
  if (r >= 75 && r <= 110 &&
      g >= 120 && g <= 160 &&
      b >= 75 && b <= 110) {
    return PURPLE;
  }

  // Green
  if (r >= 105 && r <= 145 &&
      g >= 85 && g <= 125 &&
      b >= 90 && b <= 130 &&
      g < r &&
      g <= b + 15) {
    return GREEN;
  }

  // Blue
  if (r >= 95 && r <= 160 &&
      g >= 80 && g <= 150 &&
      b >= 35 && b <= 100 &&
      b < r - 25 &&
      b < g - 20) {
    return BLUE;
  }

  return -1;
}
