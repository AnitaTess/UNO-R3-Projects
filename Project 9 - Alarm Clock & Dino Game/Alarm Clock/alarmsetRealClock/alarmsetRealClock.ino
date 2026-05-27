#include <Wire.h>
#include <DS3231.h>
#include <LiquidCrystal.h>
#include "pitches.h"

// LCD pins
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

// RTC
DS3231 clock;
RTCDateTime dt;

// Pins
const int buzzerPin = 6;

const int setButtonPin = 2;
const int upButtonPin = 3;
const int downButtonPin = 4;
const int stopButtonPin = 5;

// Alarm settings
int alarmHour = 7;
int alarmMinute = 30;
bool alarmEnabled = true;
bool alarmRinging = false;

int lastTriggeredDay = -1;

// Menu modes
// 0 = normal clock
// 1 = set alarm hour
// 2 = set alarm minute
// 3 = alarm on/off
int menuMode = 0;

// Timing
unsigned long lastDisplayUpdate = 0;
unsigned long lastMelodyStep = 0;
unsigned long lastButtonTime = 0;

const unsigned long buttonCooldown = 250;

// Alarm melody
int melody[] = {
  NOTE_C5, NOTE_E5, NOTE_G5, NOTE_C6,
  NOTE_G5, NOTE_E5, NOTE_C5
};

int melodyLength = sizeof(melody) / sizeof(melody[0]);
int melodyIndex = 0;

void setup() {
  Serial.begin(9600);

  Wire.begin();
  clock.begin();

  lcd.begin(16, 2);

  pinMode(buzzerPin, OUTPUT);

  pinMode(setButtonPin, INPUT_PULLUP);
  pinMode(upButtonPin, INPUT_PULLUP);
  pinMode(downButtonPin, INPUT_PULLUP);
  pinMode(stopButtonPin, INPUT_PULLUP);

  // Uncomment this ONCE if you need to set RTC time,
  // then comment it again and upload again.
  // clock.setDateTime(__DATE__, __TIME__);

  lcd.clear();
  lcd.print("Alarm Clock");
  lcd.setCursor(0, 1);
  lcd.print("RTC Ready");
  delay(1500);
  lcd.clear();

  Serial.println("Alarm clock ready");
}

void loop() {
  dt = clock.getDateTime();

  handleButtons();
  checkAlarm();

  if (alarmRinging) {
    playAlarmMelody();
  }

  if (millis() - lastDisplayUpdate >= 250) {
    lastDisplayUpdate = millis();
    updateDisplay();
  }
}

// --------------------
// Display
// --------------------

void updateDisplay() {
  lcd.clear();

  if (menuMode == 0) {
    lcd.setCursor(0, 0);
    lcd.print("Time ");

    printTwoDigits(dt.hour);
    lcd.print(":");
    printTwoDigits(dt.minute);
    lcd.print(":");
    printTwoDigits(dt.second);

    lcd.setCursor(0, 1);
    lcd.print("Alarm ");

    printTwoDigits(alarmHour);
    lcd.print(":");
    printTwoDigits(alarmMinute);

    lcd.print(alarmEnabled ? " ON" : " OFF");
  }

  else if (menuMode == 1) {
    lcd.setCursor(0, 0);
    lcd.print("SET ALARM HOUR");

    lcd.setCursor(0, 1);
    lcd.print("Alarm ");
    printTwoDigits(alarmHour);
    lcd.print(":");
    printTwoDigits(alarmMinute);
    lcd.print("  ^H");
  }

  else if (menuMode == 2) {
    lcd.setCursor(0, 0);
    lcd.print("SET ALARM MIN");

    lcd.setCursor(0, 1);
    lcd.print("Alarm ");
    printTwoDigits(alarmHour);
    lcd.print(":");
    printTwoDigits(alarmMinute);
    lcd.print("  ^M");
  }

  else if (menuMode == 3) {
    lcd.setCursor(0, 0);
    lcd.print("ALARM ON/OFF");

    lcd.setCursor(0, 1);
    lcd.print(alarmEnabled ? "Enabled: YES" : "Enabled: NO ");
  }
}

void printTwoDigits(int number) {
  if (number < 10) {
    lcd.print("0");
  }

  lcd.print(number);
}

// --------------------
// Buttons
// --------------------

void handleButtons() {
  if (millis() - lastButtonTime < buttonCooldown) {
    return;
  }

  // SET button moves through the menu
  if (digitalRead(setButtonPin) == LOW) {
    lastButtonTime = millis();

    menuMode++;

    if (menuMode > 3) {
      menuMode = 0;
    }

    updateDisplay();
    return;
  }

  // UP button
  if (digitalRead(upButtonPin) == LOW) {
    lastButtonTime = millis();

    if (menuMode == 1) {
      alarmHour++;
      if (alarmHour > 23) {
        alarmHour = 0;
      }
    }
    else if (menuMode == 2) {
      alarmMinute++;
      if (alarmMinute > 59) {
        alarmMinute = 0;
      }
    }
    else if (menuMode == 3) {
      alarmEnabled = true;
    }

    updateDisplay();
    return;
  }

  // DOWN button
  if (digitalRead(downButtonPin) == LOW) {
    lastButtonTime = millis();

    if (menuMode == 1) {
      alarmHour--;
      if (alarmHour < 0) {
        alarmHour = 23;
      }
    }
    else if (menuMode == 2) {
      alarmMinute--;
      if (alarmMinute < 0) {
        alarmMinute = 59;
      }
    }
    else if (menuMode == 3) {
      alarmEnabled = false;
    }

    updateDisplay();
    return;
  }

  // STOP button
  if (digitalRead(stopButtonPin) == LOW) {
    lastButtonTime = millis();

    if (alarmRinging) {
      stopAlarm();
    }

    return;
  }
}

// --------------------
// Alarm logic
// --------------------

void checkAlarm() {
  if (!alarmEnabled || alarmRinging) {
    return;
  }

  if (
    dt.hour == alarmHour &&
    dt.minute == alarmMinute &&
    dt.day != lastTriggeredDay
  ) {
    alarmRinging = true;
    lastTriggeredDay = dt.day;
    melodyIndex = 0;

    lcd.clear();
    lcd.print("ALARM!");
    Serial.println("ALARM!");
  }
}

void playAlarmMelody() {
  if (millis() - lastMelodyStep >= 300) {
    lastMelodyStep = millis();

    tone(buzzerPin, melody[melodyIndex], 220);

    melodyIndex++;

    if (melodyIndex >= melodyLength) {
      melodyIndex = 0;
    }
  }
}

void stopAlarm() {
  alarmRinging = false;
  noTone(buzzerPin);

  lcd.clear();
  lcd.print("Alarm stopped");
  delay(800);
  lcd.clear();

  Serial.println("Alarm stopped");
}