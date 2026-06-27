/*
 * LCD:
 * VCC -> 5V
 * GND -> GND
 * SDA -> GPIO 21
 * SCL -> GPIO 22
 *
 * DHT22:
 * VCC  -> 3V3
 * GND  -> GND
 * DATA -> GPIO 4
 *
 * Buzzer:
 * VCC -> 3V3
 * GND -> GND
 * I/O -> GPIO 16
 *
 * LED:
 * + -> GPIO 18 - 220ohm
 * - -> GND
 */

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <WiFi.h>
#include "secrets.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define DHT_PIN 4
#define DHT_TYPE DHT22

#define BUZZER_PIN 16
#define LED_PIN 18

#define BUZZER_ON LOW
#define BUZZER_OFF HIGH

#define TEMP_MAX 30.0
#define HUM_MAX 70.0

#define MQTT_TOPIC_DATA "esp32/nr1/data"
#define MQTT_TOPIC_STATUS "esp32/nr1/status"
#define MQTT_TOPIC_ALARM "esp32/nr1/alarm"

DHT dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
WiFiClient espClient;
PubSubClient mqttClient(espClient);

float temperature = 0.0;
float humidity = 0.0;

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
  if (temperature > TEMP_MAX)
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
  if (temperature > TEMP_MAX || humidity > HUM_MAX)
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
  Serial.println();
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.print("WiFi connected, IP: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("WiFi connection failed");
  }
}

void checkWIFI()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WI-Fi disconnected");
    connectWIFI();
  }
}

void connectMQTT()
{
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  Serial.print("Connecting to MQTT");
  int attempts = 0;
  while (!mqttClient.connected() && attempts < 10)
  {
    if (mqttClient.connect("esp32"))
    {
      Serial.println();
      Serial.println("MQTT connected");
      delay(3000);
      mqttClient.publish(MQTT_TOPIC_STATUS, "online");
      Serial.println("MQTT status sent");
    }
    else
    {
      Serial.print(".");
      delay(1000);
      attempts++;
    }
  }
  if (!mqttClient.connected())
  {
    Serial.println("MQTT connection failed");
  }
}

void publishMeasurements(bool alarmStatus)
{
  if (!mqttClient.connected())
  {
    return;
  }
  StaticJsonDocument<128> doc;
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

void setup()
{
  Serial.begin(115200);
  delay(1000);
  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  dht.begin();

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(BUZZER_PIN, BUZZER_OFF);
  digitalWrite(LED_PIN, LOW);

  connectWIFI();
  connectMQTT();
}

void loop()
{
  if (mqttClient.connected())
  {
    mqttClient.loop();
  }
  checkWIFI();
  lcd.clear();
  bool sensorStatus = readSensor();

  if (!sensorStatus)
  {
    alarmOFF();
    lcdSensorError();
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
    publishMeasurements(alarmStatus);
    publishAlarm(alarmStatus);
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
  delay(3000);
}