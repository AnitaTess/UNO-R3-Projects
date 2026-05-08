// IR Remote + Buzzer Melody + RGB LED Control
// Using old ELEGOO IRremote.h style
// Avoids tone() because tone() conflicts with IRremote timer on Arduino UNO

#include "IRremote.h"
#include "pitches.h"

// ---------- PINS ----------
int receiver = 11; // IR receiver signal pin

const int buzzerPin = 7;

const int bluePin = 2;
const int greenPin = 3;
const int redPin = 4;

// If your RGB LED works backwards, change these:
// For common cathode RGB LED: HIGH = on, LOW = off
// For common anode RGB LED: LOW = on, HIGH = off
const int LED_ON = HIGH;
const int LED_OFF = LOW;

// ---------- IR SETUP ----------
IRrecv irrecv(receiver);
decode_results results;

// ---------- SYSTEM STATE ----------
bool systemOn = false;

bool melodyOn = false;
bool melodyPaused = false;

bool ledEnabled = false;
bool rainbowMode = true;
bool rainbowPaused = false;
bool alarmMode = false;

// ---------- LED TIMING ----------
unsigned long lastLedChange = 0;
int ledSpeed = 500; // lower = faster colour changing
int rainbowIndex = 0;

unsigned long lastAlarmBlink = 0;
bool alarmLedState = false;

// ---------- MELODY TIMING ----------
unsigned long nextNoteTime = 0;
int melodySpeed = 350; // lower = faster melody
int melodyIndex = 0;

// Simple melody
int melody[] = {
  NOTE_C5, NOTE_C5, NOTE_G5, NOTE_G5,
  NOTE_A5, NOTE_A5, NOTE_G5,
  NOTE_F5, NOTE_F5, NOTE_E5, NOTE_E5,
  NOTE_D5, NOTE_D5, NOTE_C5
};

int melodyLength = sizeof(melody) / sizeof(melody[0]);

// ---------- MANUAL BUZZER VARIABLES ----------
bool buzzerPlaying = false;
unsigned long lastBuzzToggle = 0;
unsigned long buzzHalfPeriod = 1000;
bool buzzState = LOW;

// ---------- MANUAL BUZZER FUNCTIONS ----------

void startManualTone(int frequency) {
  if (frequency <= 0) {
    buzzerPlaying = false;
    digitalWrite(buzzerPin, LOW);
    return;
  }

  buzzHalfPeriod = 1000000UL / frequency / 2;
  buzzerPlaying = true;
}

void stopManualTone() {
  buzzerPlaying = false;
  digitalWrite(buzzerPin, LOW);
}

void updateManualTone() {
  if (!buzzerPlaying) return;

  unsigned long currentMicros = micros();

  if (currentMicros - lastBuzzToggle >= buzzHalfPeriod) {
    lastBuzzToggle = currentMicros;
    buzzState = !buzzState;
    digitalWrite(buzzerPin, buzzState);
  }
}

// ---------- LED FUNCTIONS ----------

void setRGB(bool red, bool green, bool blue) {
  digitalWrite(redPin, red ? LED_ON : LED_OFF);
  digitalWrite(greenPin, green ? LED_ON : LED_OFF);
  digitalWrite(bluePin, blue ? LED_ON : LED_OFF);
}

void ledOff() {
  setRGB(false, false, false);
}

void setRainbowColour(int index) {
  switch (index) {
    case 0:
      setRGB(true, false, false);   // red
      break;
    case 1:
      setRGB(true, true, false);    // yellow / orange-ish
      break;
    case 2:
      setRGB(false, true, false);   // green
      break;
    case 3:
      setRGB(false, true, true);    // cyan
      break;
    case 4:
      setRGB(false, false, true);   // blue
      break;
    case 5:
      setRGB(true, false, true);    // purple / pink-ish
      break;
    case 6:
      setRGB(true, true, true);     // white
      break;
  }
}

void setColourByNumber(char key) {
  if (!systemOn) return;

  alarmMode = false;
  rainbowMode = false;
  rainbowPaused = false;
  ledEnabled = true;

  switch (key) {
    case '1':
      setRGB(true, true, false);    // yellow
      Serial.println("LED: Yellow");
      break;

    case '2':
      setRGB(true, true, false);    // orange approx
      Serial.println("LED: Orange approx");
      break;

    case '3':
      setRGB(true, false, false);   // red
      Serial.println("LED: Red");
      break;

    case '4':
      setRGB(true, false, true);    // purple
      Serial.println("LED: Purple");
      break;

    case '5':
      setRGB(false, false, true);   // blue
      Serial.println("LED: Blue");
      break;

    case '6':
      setRGB(false, true, true);    // cyan
      Serial.println("LED: Cyan");
      break;

    case '7':
      setRGB(false, true, false);   // green
      Serial.println("LED: Green");
      break;

    case '8':
      setRGB(true, true, true);     // white
      Serial.println("LED: White");
      break;

    case '9':
      setRGB(true, false, true);    // pink approx
      Serial.println("LED: Pink approx");
      break;
  }
}

// ---------- SYSTEM FUNCTIONS ----------

void turnSystemOn() {
  systemOn = true;

  melodyOn = true;
  melodyPaused = false;
  melodyIndex = 0;
  nextNoteTime = 0;

  ledEnabled = true;
  rainbowMode = true;
  rainbowPaused = false;
  alarmMode = false;
  rainbowIndex = 0;
  lastLedChange = 0;

  Serial.println("SYSTEM ON");
}

void turnSystemOff() {
  systemOn = false;

  melodyOn = false;
  melodyPaused = false;
  stopManualTone();

  ledEnabled = false;
  rainbowMode = false;
  rainbowPaused = false;
  alarmMode = false;
  ledOff();

  Serial.println("SYSTEM OFF");
}

// ---------- UPDATE MELODY ----------

void updateMelody() {
  if (!systemOn || !melodyOn || melodyPaused) {
    stopManualTone();
    return;
  }

  unsigned long currentTime = millis();

  if (currentTime >= nextNoteTime) {
    int note = melody[melodyIndex];

    startManualTone(note);

    melodyIndex++;

    if (melodyIndex >= melodyLength) {
      melodyIndex = 0;
    }

    nextNoteTime = currentTime + melodySpeed;
  }
}

// ---------- UPDATE LED ----------

void updateLED() {
  if (!systemOn || !ledEnabled) {
    ledOff();
    return;
  }

  unsigned long currentTime = millis();

  if (alarmMode) {
    if (currentTime - lastAlarmBlink >= 200) {
      lastAlarmBlink = currentTime;
      alarmLedState = !alarmLedState;

      if (alarmLedState) {
        setRGB(true, false, false); // blink red
      } else {
        ledOff();
      }
    }

    return;
  }

  if (rainbowMode && !rainbowPaused) {
    if (currentTime - lastLedChange >= ledSpeed) {
      lastLedChange = currentTime;

      setRainbowColour(rainbowIndex);

      rainbowIndex++;

      if (rainbowIndex > 6) {
        rainbowIndex = 0;
      }
    }
  }
}

// ---------- IR ACTIONS ----------

void handleIR(unsigned long code) {
  switch (code) {

    case 0xFFA25D: // POWER
      Serial.println("POWER");

      if (systemOn) {
        turnSystemOff();
      } else {
        turnSystemOn();
      }
      break;

    case 0xFFE21D: // FUNC/STOP
      Serial.println("FUNC/STOP");

      if (systemOn) {
        melodyPaused = !melodyPaused;

        if (melodyPaused) {
          stopManualTone();
          Serial.println("Melody paused");
        } else {
          Serial.println("Melody resumed");
        }
      }
      break;

    case 0xFF629D: // VOL+
      Serial.println("VOL+ - volume change not supported with this buzzer setup");
      break;

    case 0xFFA857: // VOL-
      Serial.println("VOL- - volume change not supported with this buzzer setup");
      break;

    case 0xFF22DD: // FAST BACK
      Serial.println("FAST BACK - slower LED colour change");

      ledSpeed += 100;

      if (ledSpeed > 2000) {
        ledSpeed = 2000;
      }

      Serial.print("LED speed: ");
      Serial.println(ledSpeed);
      break;

    case 0xFFC23D: // FAST FORWARD
      Serial.println("FAST FORWARD - faster LED colour change");

      ledSpeed -= 100;

      if (ledSpeed < 100) {
        ledSpeed = 100;
      }

      Serial.print("LED speed: ");
      Serial.println(ledSpeed);
      break;

    case 0xFF02FD: // PLAY/PAUSE
      Serial.println("PLAY/PAUSE");

      if (systemOn && rainbowMode) {
        rainbowPaused = !rainbowPaused;

        if (rainbowPaused) {
          Serial.println("Rainbow paused on current colour");
        } else {
          Serial.println("Rainbow resumed");
        }
      }
      break;

    case 0xFFE01F: // DOWN
      Serial.println("DOWN - slower melody");

      melodySpeed += 50;

      if (melodySpeed > 1000) {
        melodySpeed = 1000;
      }

      Serial.print("Melody speed: ");
      Serial.println(melodySpeed);
      break;

    case 0xFF906F: // UP
      Serial.println("UP - faster melody");

      melodySpeed -= 50;

      if (melodySpeed < 120) {
        melodySpeed = 120;
      }

      Serial.print("Melody speed: ");
      Serial.println(melodySpeed);
      break;

    case 0xFFB04F: // ST/REPT
      Serial.println("ST/REPT - alarm LED mode");

      if (systemOn) {
        ledEnabled = true;
        alarmMode = true;
        rainbowMode = false;
        rainbowPaused = false;
      }
      break;

   case 0xFF9867: // EQ / EX on some remotes
  Serial.println("EQ/EX - toggle LED on/off, melody unaffected");

  if (systemOn) {
    if (ledEnabled) {
      // LED is currently on, so turn it off
      ledEnabled = false;
      alarmMode = false;
      rainbowMode = false;
      rainbowPaused = false;
      ledOff();

      Serial.println("LED turned off");
    } 
    else {
      // LED is currently off, so turn it back on in default rainbow mode
      ledEnabled = true;
      alarmMode = false;
      rainbowMode = true;
      rainbowPaused = false;

      Serial.println("LED turned back on in rainbow mode");
    }
  }
  break;

    case 0xFF6897: // 0
      Serial.println("0 - default rainbow mode");

      if (systemOn) {
        ledEnabled = true;
        rainbowMode = true;
        rainbowPaused = false;
        alarmMode = false;
      }
      break;

    case 0xFF30CF: // 1
      setColourByNumber('1');
      break;

    case 0xFF18E7: // 2
      setColourByNumber('2');
      break;

    case 0xFF7A85: // 3
      setColourByNumber('3');
      break;

    case 0xFF10EF: // 4
      setColourByNumber('4');
      break;

    case 0xFF38C7: // 5
      setColourByNumber('5');
      break;

    case 0xFF5AA5: // 6
      setColourByNumber('6');
      break;

    case 0xFF42BD: // 7
      setColourByNumber('7');
      break;

    case 0xFF4AB5: // 8
      setColourByNumber('8');
      break;

    case 0xFF52AD: // 9
      setColourByNumber('9');
      break;

    case 0xFFFFFFFF:
      // Button held down repeat signal
      Serial.println("REPEAT");
      break;

    default:
      Serial.println("Other button");
      break;
  }
}

// ---------- SETUP ----------

void setup() {
  Serial.begin(9600);
  Serial.println("IR Remote RGB LED + Buzzer Controller");

  irrecv.enableIRIn();

  pinMode(buzzerPin, OUTPUT);

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  digitalWrite(buzzerPin, LOW);
  ledOff();
}

// ---------- LOOP ----------

void loop() {
  if (irrecv.decode(&results)) {
    handleIR(results.value);
    irrecv.resume();
  }

  updateMelody();
  updateLED();
  updateManualTone();
}