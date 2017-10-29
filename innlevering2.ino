#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <Wire.h>
#include "RTClib.h"
RTC_DS1307 RTC;

#define TFT_CS   10
#define TFT_RST  8
#define TFT_DC   9

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

#define TFT_SCLK 13
#define TFT_MOSI 11

DateTime now;
DateTime globalAlarm;

int newHour;
int newMinute;

int buzzerPin = 2;
int btnAlarmPin = 4;
int btnMenuPin = 7;

int btnAlarmState;
int btnMenuState;

int prevMinute = -1;
int setAlarmState = false;

bool doneSettingAlarm;
bool doneSettingHour;
bool doneSettingMinute;

bool alarmCleared = false;

void setup(void) {
  Serial.begin(9600);
  Wire.begin();
  RTC.begin();
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }

  pinMode(buzzerPin, OUTPUT);
  pinMode(btnAlarmPin, INPUT);
  pinMode(btnMenuPin, INPUT);

  tft.initR(INITR_BLACKTAB);
  Serial.println("Initialized tft");
  tft.fillScreen(ST7735_BLACK);
  tft.setRotation(3);
}

void loop() {
  now = RTC.now();
  // serialPrintDate(now);
  btnAlarmState = digitalRead(btnAlarmPin);
  btnMenuState = digitalRead(btnMenuPin);

  if (btnAlarmState == HIGH) {
    setAlarmState = !setAlarmState;
  }

  if (setAlarmState) {
    prevMinute = -1;
    setAlarm();
  } else {
    doneSettingAlarm = false;
    doneSettingHour = false;
    doneSettingMinute = false;
    if (now.hour() == globalAlarm.hour() && now.minute() == globalAlarm.minute()) {
      if (btnMenuState == HIGH) {
        globalAlarm = DateTime();
        prevMinute = -1;
      }
      soundAlarm();
    } else {
      noTone(buzzerPin);
    }
  
    if (now.minute() != prevMinute) {
      char time[5];
      sprintf(time, "%d:%d", now.hour(), now.minute());
      setText(time, ST7735_WHITE);
      prevMinute = now.minute();
    }
  }
  delay(200);
}

void soundAlarm() {
  if (!alarmCleared) {
    tone(buzzerPin, 750, 150);
  } else {
    noTone(buzzerPin);
  }
}

void setAlarm() {
  DateTime alarm = RTC.now();
  newHour = alarm.hour();
  newMinute = alarm.minute();
  char time[5];

  setAlarmPrintTime(alarm);

  while (!doneSettingAlarm) {
    btnAlarmState = digitalRead(btnAlarmPin);
    btnMenuState = digitalRead(btnMenuPin);
    Serial.print("Done hours: ");
    Serial.println(doneSettingHour);
    Serial.print("Done minutes: ");
    Serial.println(doneSettingMinute);
    Serial.print("Done alarm: ");
    Serial.println(doneSettingAlarm);
    if (!doneSettingHour) {
      if (btnAlarmState == HIGH) {
        newHour = newHour + 1;
        alarm = DateTime(alarm.year(), alarm.month(), alarm.day(), newHour, alarm.minute(), alarm.second());
        setAlarmPrintTime(alarm);
      }
      if (btnMenuState == HIGH) {
        doneSettingHour = true;
      }
    } else if (doneSettingHour && !doneSettingMinute) {
      if (btnAlarmState == HIGH) {
        newMinute = newMinute + 1;
        alarm = DateTime(alarm.year(), alarm.month(), alarm.day(), alarm.hour(), newMinute, alarm.second());
        setAlarmPrintTime(alarm);
      }
      if (btnMenuState == HIGH) {
        doneSettingMinute = true;
      }
    }
    if (doneSettingHour && doneSettingMinute) {
      doneSettingAlarm = true;
    }
  }
  globalAlarm = alarm;
  setAlarmState = false;
}

void setAlarmPrintTime(DateTime alarm) {
  char time[5];
  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(25, 30);
  tft.setTextSize(2);
  tft.setTextColor(ST7735_WHITE);
  tft.print("Set alarm");

  tft.setCursor(20, 50);
  tft.setTextSize(4);
  sprintf(time, "%d:%d", alarm.hour(), alarm.minute());
  tft.print(time);
}

uint8_t addZero(uint8_t num) {
  char newNum;
  if (num > 10) {
    newNum = (char*) num;
  } else {
    newNum = (char*) num;
  }
  return newNum;
}

void setText(char *text, uint16_t color) {
  tft.fillScreen(ST7735_BLACK);
  tft.setTextColor(color);
  tft.setTextWrap(true);

  tft.setCursor(0, 0);
  tft.setTextSize(1);
  tft.print("Alarm: ");
  if (globalAlarm.year() <= now.year() + 1) {
    tft.print("On");
  } else {
    tft.print("Off");
  }

  tft.setCursor(20, 50);
  tft.setTextSize(4);
  tft.print(text);
}

void serialPrintDate(DateTime date) {
  Serial.print(date.year());
  Serial.print('/');
  Serial.print(date.month());
  Serial.print('/');
  Serial.print(date.day());
  Serial.print(' ');
  Serial.print(date.hour());
  Serial.print(':');
  Serial.print(date.minute());
  Serial.print(':');
  Serial.println(date.second());
}

