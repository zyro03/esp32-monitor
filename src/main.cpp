#include "display.h"
#include "sensor.h"
#include "network.h"
#include "power.h"
#include "alarm.h"

#include <Arduino.h>

float temperature = 0.0;
float humidity = 0.0;

float tempMax = 30.0;
float humMax = 70.0;

bool statusScreen = false;
RTC_DATA_ATTR bool wasOnBattery = false;
RTC_DATA_ATTR bool sensorErrorActive = false;

String powerSource = "main";
String workMode = "normal";


void setup()
{
  Serial.begin(115200);
  esp_sleep_wakeup_cause_t wakeupCause = esp_sleep_get_wakeup_cause();
  if (wakeupCause == ESP_SLEEP_WAKEUP_TIMER)
  {
    Serial.println("Wake up from deep sleep timer");
  }
  initDisplay();
  initSensor();
  initAlarm();
  initPower();

  initNetwork();
  connectWIFI();
  updatePowerStatus();
  
  if (isWiFiConnected())
  {
    connectMQTT();
  }
  if (wakeupCause == ESP_SLEEP_WAKEUP_TIMER && isMQTTConnected())
  {
    publishSystemEvent("WAKE_UP", "ESP32 woke up from deep sleep");
  }
  if (powerSource == "main" && wasOnBattery && isMQTTConnected())
  {
    publishSystemEvent("POWER_RESTORED", "Main power restored");
    publishDeviceStatus();
    wasOnBattery = false;
  }
}

void loop()
{
  checkWIFI();
  if (isWiFiConnected())
  {
    checkMQTT();
  }
  processMQTT();
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
  clearDisplay();
  bool sensorStatus = readSensor();

  if (!sensorStatus)
  {
    alarmOFF();
    lcdSensorError();
    if (!sensorErrorActive && isMQTTConnected())
    {
      publishSystemEvent("SENSOR_ERROR", "DHT22 read failed");
    }
    sensorErrorActive = true;
  }
  else
  {
    if (sensorErrorActive && isMQTTConnected())
    {
      publishSystemEvent("SENSOR_RESTORED", "DHT22 restored");
    }
    sensorErrorActive = false;
    Serial.print("Temperature: ");
    Serial.print(temperature, 1);
    Serial.println(" C");
    Serial.print("Humidity: ");
    Serial.print(humidity, 1);
    Serial.println(" %");
    Serial.println("-----");

    bool alarmStatus = checkAlarm();

    if (isMQTTConnected())
    {
      publishMeasurements(alarmStatus);
      publishDeviceStatus();
    }
    else
    {
      Serial.println("MQTT not connected, measurement not published");
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
  delay(3000);
}