#include "secrets.h"
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define DHT_PIN 4
#define DHT_TYPE DHT22

#define BUZZER_PIN 16
#define LED_PIN 18

#define POWER_DETECT_PIN 34
#define DEEP_SLEEP_TIME_SECONDS 30

#define BUZZER_ON LOW
#define BUZZER_OFF HIGH

#define DEVICE_ID "nr1"
#define MQTT_CLIENT_ID "esp32_nr1"

#define MQTT_TOPIC_DATA "esp32/" DEVICE_ID "/data"
#define MQTT_TOPIC_STATUS "esp32/" DEVICE_ID "/status"
#define MQTT_TOPIC_ALARM "esp32/" DEVICE_ID "/alarm"
#define MQTT_TOPIC_CONFIG "esp32/" DEVICE_ID "/config"
#define MQTT_TOPIC_EVENT "esp32/" DEVICE_ID "/event"
#define MQTT_TOPIC_DEVICE_STATUS "esp32/" DEVICE_ID "/device_status"

DHT dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
WiFiClient espClient;
PubSubClient mqttClient(espClient);

float temperature = 0.0;
float humidity = 0.0;

float tempMax = 30.0;
float humMax = 70.0;

bool lastMainPowerState = true;
RTC_DATA_ATTR bool wasOnBattery = false;

String powerSource = "main";
String workMode = "normal";

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

void lcdMeasurements()
{
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature, 1);
  lcd.print(" C");

  lcd.setCursor(0, 1);
  lcd.print("Humi: ");
  lcd.print(humidity, 1);
  lcd.print(" %");
}

void lcdSensorError()
{
  lcd.setCursor(0, 0);
  lcd.print("DHT22 ERROR");
  lcd.setCursor(0, 1);
  lcd.print("Check sensor");
}

void lcdAlarm()
{
  lcd.setCursor(0, 0);
  lcd.print("ALARM!!!");
  lcd.setCursor(0, 1);
  if (temperature > tempMax)
  {
    lcd.print("HIGH temp!");
  }
  else
  {
    lcd.print("HIGH humi");
  }
}

bool readSensor()
{
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity))
  {
    return false;
  }
  else
  {
    return true;
  }
}

bool checkAlarm()
{
  if (temperature > tempMax || humidity > humMax)
  {
    return true;
  }
  else
  {
    return false;
  }
}

void connectWIFI()
{
  Serial.print("Connecting to WiFi");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;

  while (WiFi.status() != WL_CONNECTED && attempts < 20)
  {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.print("WiFi connected, ESP32 IP: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("WiFi connection failed");
  }
}

void publishDeviceStatus()
{
  if (!mqttClient.connected())
  {
    return;
  }
  JsonDocument doc;
  doc["device"] = DEVICE_ID;
  doc["power_source"] = powerSource;
  doc["work_mode"] = workMode;
  doc["wifi_status"] = WiFi.status() == WL_CONNECTED;
  doc["mqtt_status"] = mqttClient.connected();
  char payload[192];
  serializeJson(doc, payload);
  mqttClient.publish(MQTT_TOPIC_DEVICE_STATUS, payload);
  Serial.print("MQTT device status sent: ");
  Serial.println(payload);
}

void checkWIFI()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    return;
  }
  Serial.println("WI-Fi disconnected");
  connectWIFI();
}

void publishSystemEvent(const char *eventType, const char *message)
{
  if (!mqttClient.connected())
  {
    return;
  }
  JsonDocument doc;
  doc["device"] = DEVICE_ID;
  doc["event_type"] = eventType;
  doc["message"] = message;
  char payload[160];
  serializeJson(doc, payload);
  mqttClient.publish(MQTT_TOPIC_EVENT, payload);
  Serial.print("MQTT event sent: ");
  Serial.println(payload);
}

void connectMQTT()
{
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  int attempts = 0;
  while (!mqttClient.connected() && attempts < 5)
  {
    Serial.print("Connecting to MQTT");
    if (mqttClient.connect(MQTT_CLIENT_ID))
    {
      Serial.println("MQTT connected");

      mqttClient.publish(MQTT_TOPIC_STATUS, "online");
      mqttClient.subscribe(MQTT_TOPIC_CONFIG);
      Serial.println("MQTT status sent and topic subscribed");
      publishSystemEvent("MQTT_CONNECTED", "MQTT connection");
      publishDeviceStatus();
    }
    else
    {
      Serial.print("failed, ");
      Serial.println(mqttClient.state());
      attempts++;
      delay(2000);
    }
  }
  if (!mqttClient.connected())
  {
    Serial.println("MQTT connection failed");
  }
}

void checkMQTT()
{
  if (mqttClient.connected())
  {
    return;
  }
  Serial.println("MQTT disconnected");
  connectMQTT();
}

void publishMeasurements(bool alarmStatus)
{
  if (!mqttClient.connected())
  {
    return;
  }
  JsonDocument doc;
  doc["device"] = DEVICE_ID;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["alarm"] = alarmStatus;
  char payload[128];
  serializeJson(doc, payload);
  mqttClient.publish(MQTT_TOPIC_DATA, payload);
  Serial.print("MQTT data sent: ");
  Serial.println(payload);
}

void publishAlarm(bool alarmStatus)
{
  if (!mqttClient.connected())
  {
    return;
  }
  if (alarmStatus)
  {
    mqttClient.publish(MQTT_TOPIC_ALARM, "active");
  }
  else
  {
    mqttClient.publish(MQTT_TOPIC_ALARM, "inactive");
  }
}

void handleMqttMessage(char *topic, byte *payload, unsigned int length)
{
  if (String(topic) != MQTT_TOPIC_CONFIG)
  {
    return;
  }
  JsonDocument doc;
  if (deserializeJson(doc, payload, length))
  {
    Serial.println("fault");
    return;
  }
  tempMax = doc["temp_max"];
  humMax = doc["hum_max"];
  Serial.println("new config rec");
}

void updatePowerStatus()
{
  int powerValue = analogRead(POWER_DETECT_PIN);
  bool mainPowerPresent = powerValue > 1000;

  if (mainPowerPresent)
  {
    powerSource = "main";
    workMode = "normal";
    Serial.println("Power source: MAIN");
  }
  else
  {
    powerSource = "battery";
    workMode = "battery";
    Serial.println("Power source: BATTERY");
  }

  if (mainPowerPresent != lastMainPowerState)
  {
    if (mainPowerPresent)
    {
      publishSystemEvent("POWER_RESTORED", "Main power restored");
    }
    else
    {
      publishSystemEvent("POWER_LOSS", "Main power lost");
    }

    publishDeviceStatus();
    lastMainPowerState = mainPowerPresent;
  }
}

void enterDeepSleep()
{
  wasOnBattery = true;
  workMode = "deep_sleep";

  publishSystemEvent(
      "DEEP_SLEEP_ENTER",
      "ESP32 entering deep sleep");

  publishDeviceStatus();

  delay(500);

  alarmOFF();
  lcd.clear();
  lcd.noBacklight();

  Serial.println("Entering deep sleep for 30 seconds");
  Serial.flush();

  esp_sleep_enable_timer_wakeup(
      DEEP_SLEEP_TIME_SECONDS * 1000000ULL);

  esp_deep_sleep_start();
}

void setup()
{
  Serial.begin(115200);
  esp_sleep_wakeup_cause_t wakeupCause = esp_sleep_get_wakeup_cause();
  if (wakeupCause == ESP_SLEEP_WAKEUP_TIMER)
  {
    Serial.println("Wake up from deep sleep timer");
  }
  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  dht.begin();

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(POWER_DETECT_PIN, INPUT);
  lastMainPowerState = analogRead(POWER_DETECT_PIN) > 1000;

  digitalWrite(BUZZER_PIN, BUZZER_OFF);
  digitalWrite(LED_PIN, LOW);

  connectWIFI();
  updatePowerStatus();
  mqttClient.setCallback(handleMqttMessage);
  if (WiFi.status() == WL_CONNECTED)
  {
    connectMQTT();
  }
  if (
      wakeupCause == ESP_SLEEP_WAKEUP_TIMER &&
      mqttClient.connected())
  {
    publishSystemEvent(
        "WAKE_UP",
        "ESP32 woke up from deep sleep");
  }
  if (powerSource == "main" && wasOnBattery && mqttClient.connected())
  {
    publishSystemEvent("POWER_RESTORED", "Main power restored");
    publishDeviceStatus();
    wasOnBattery = false;
  }
}

void loop()
{
  checkWIFI();
  if (WiFi.status() == WL_CONNECTED)
  {
    checkMQTT();
  }
  if (mqttClient.connected())
  {
    mqttClient.loop();
  }
  updatePowerStatus();
  lcd.clear();
  bool sensorStatus = readSensor();

  if (!sensorStatus)
  {
    alarmOFF();
    lcdSensorError();
    if (mqttClient.connected())
    {
      publishSystemEvent("SENSOR_ERROR", "DHT22 read failed");
    }
  }
  else
  {
    Serial.print("Temperature: ");
    Serial.print(temperature, 1);
    Serial.println(" C");
    Serial.print("Humidity: ");
    Serial.print(humidity, 1);
    Serial.println(" %");
    Serial.println("-----");

    bool alarmStatus = checkAlarm();

    if (mqttClient.connected())
    {
      publishMeasurements(alarmStatus);
      publishAlarm(alarmStatus);
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
      lcdMeasurements();
    }
  }
  if (powerSource == "battery")
  {
    enterDeepSleep();
  }
  delay(3000);
}