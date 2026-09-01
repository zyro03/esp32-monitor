#include "display.h"
#include "sensor.h"
#include "network.h"
#include "power.h"
#include "alarm.h"

#include <Arduino.h>

float temperature = 0.0;
float humidity = 0.0;
float tempMin = 10.0;
float tempMax = 40.0;
float humMin = 10.0;
float humMax = 70.0;
bool statusScreen = false;
bool startEventPending = false;
bool wakeEventPending = false;
RTC_DATA_ATTR bool wasOnBattery = false;
String powerSource = "main";
String workMode = "active";
unsigned long lastMeasurementTime = 0;
const unsigned long measurementInterval = 30000;

void setup()
{
  Serial.begin(115200);
  esp_sleep_wakeup_cause_t wakeupCause = esp_sleep_get_wakeup_cause();
  if (wakeupCause != ESP_SLEEP_WAKEUP_TIMER)
  {
    startEventPending = true;
  }
  if (wakeupCause == ESP_SLEEP_WAKEUP_TIMER)
  {
    Serial.println("[SLEEP] Wake up from timer");
  }

  initDisplay();
  initSensor();
  initAlarm();
  initPower();

  updatePowerStatus();

  initNetwork();
  connectWIFI();

  if (isWiFiConnected())
  {
    connectMQTT();
  }
  if (wakeupCause == ESP_SLEEP_WAKEUP_TIMER)
  {
    wakeEventPending = true;
  }
}

void loop()
{
  updatePowerStatus();
  if (powerSource == "battery")
  {
    bool sensorStatus = readSensor();
    if (sensorStatus && isMQTTConnected())
    {
      bool alarmStatus = checkAlarm();
      publishMeasurements(alarmStatus);
    }
    enterDeepSleep();
  }
  checkWIFI();
  if (isWiFiConnected())
  {
    checkMQTT();
  }
  processMQTT();
  if (startEventPending && isMQTTConnected())
  {
    publishSystemEvent(
        "START",
        "ESP32 started");
    startEventPending = false;
  }
  if (wakeEventPending && isMQTTConnected())
  {
    publishSystemEvent(
        "WAKE_UP",
        "ESP32 woke up from deep sleep");
    wakeEventPending = false;
  }
  if (powerSource == "main" && wasOnBattery && isMQTTConnected())
  {
    publishSystemEvent(
        "POWER_RESTORED",
        "Main power restored");
    publishDeviceStatus();
    wasOnBattery = false;
  }
  if (millis() - lastMeasurementTime >= measurementInterval)
{
  lastMeasurementTime = millis();
  bool sensorStatus = readSensor();

  if (!sensorStatus)
  {
      alarmOFF();

      if (isMQTTConnected())
      {
          publishDeviceStatus();
      }
  }
  else
  {
    Serial.print("[SENSOR] T=");
    Serial.print(temperature, 1);
    Serial.print(" C | H=");
    Serial.print(humidity, 1);
    Serial.println(" %");

    bool alarmStatus = checkAlarm();
    if (alarmStatus)
    {
      Serial.println("[ALARM] ON");
    }
    else
    {
      Serial.println("[ALARM] OFF");
    }

    if (isMQTTConnected())
    {
      publishMeasurements(alarmStatus);
      publishDeviceStatus();
    }
    else
    {
      Serial.println("[MQTT] Not connected - measurement skipped");
    }
    if (alarmStatus)
    {
      alarmON();
      lcdAlarm();
    }
    else
    {
      alarmOFF();
      if (statusScreen)
      {
        lcdStatus();
      }
      else
      {
        lcdMeasurements();
      }
      statusScreen = !statusScreen;
    }
  }
}
  delay(100);
}