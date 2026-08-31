#include "power.h"

#include "display.h"
#include "network.h"

#include <Arduino.h>

#define POWER_DETECT_PIN 33
#define DEEP_SLEEP_TIME_SECONDS 30

bool lastMainPowerState = true;
extern bool wasOnBattery;

extern String powerSource;
extern String workMode;

void alarmOFF();

void initPower()
{
  pinMode(POWER_DETECT_PIN, INPUT);
  lastMainPowerState = analogRead(POWER_DETECT_PIN) > 1000;
}

void updatePowerStatus()
{
  int powerValue = analogRead(POWER_DETECT_PIN);
  bool mainPowerPresent = powerValue > 1000;

  if (mainPowerPresent)
  {
    powerSource = "main";
    workMode = "normal";
    Serial.println("[POWER] MAIN");
  }
  else
  {
    powerSource = "battery";
    workMode = "battery";
    Serial.println("[POWER] BATTERY");
  }

  if (mainPowerPresent != lastMainPowerState)
  {
    if (mainPowerPresent)
    {
      publishSystemEvent("POWER_RESTORED", "Main power restored");
      Serial.println("[POWER] Main power restored");
    }
    else
    {
      publishSystemEvent("POWER_LOSS", "Main power lost");
      Serial.println("[POWER] Main power lost");
    }

    publishDeviceStatus();
    lastMainPowerState = mainPowerPresent;
  }
}

void enterDeepSleep()
{
  wasOnBattery = true;
  workMode = "deep_sleep";

  publishSystemEvent("DEEP_SLEEP_ENTER", "ESP32 entering deep sleep");

  publishDeviceStatus();

  disconnectMQTTForSleep();

  alarmOFF();
  lcdBatterySleep();
  delay(1500);
  disableDisplay();

  Serial.print("[SLEEP] Entering deep sleep for ");
  Serial.print(DEEP_SLEEP_TIME_SECONDS);
  Serial.println(" s");
  Serial.flush();

  esp_sleep_enable_timer_wakeup(DEEP_SLEEP_TIME_SECONDS * 1000000ULL);

  esp_deep_sleep_start();
}