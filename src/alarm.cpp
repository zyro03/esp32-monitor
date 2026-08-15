#include "alarm.h"
#include <Arduino.h>

#define BUZZER_PIN 4
#define LED_PIN 2

#define BUZZER_ON LOW
#define BUZZER_OFF HIGH

void alarmOFF()
{
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, BUZZER_OFF);
}

void alarmON()
{
  digitalWrite(LED_PIN, HIGH);
  digitalWrite(BUZZER_PIN, BUZZER_ON);
}

void initAlarm()
{
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  alarmOFF();
}