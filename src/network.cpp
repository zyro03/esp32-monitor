#include "network.h"
#include "secrets.h"

#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>

#define DEVICE_ID "nr1"
#define MQTT_CLIENT_ID "esp32_nr1"

#define MQTT_TOPIC_DATA "esp32/" DEVICE_ID "/data"
#define MQTT_TOPIC_CONFIG "esp32/" DEVICE_ID "/config"
#define MQTT_TOPIC_EVENT "esp32/" DEVICE_ID "/event"
#define MQTT_TOPIC_DEVICE_STATUS "esp32/" DEVICE_ID "/device_status"

WiFiClient espClient;
PubSubClient mqttClient(espClient);
bool wifiEventPending = false;

extern float temperature;
extern float humidity;

extern float tempMin;
extern float tempMax;

extern float humMin;
extern float humMax;

extern String powerSource;
extern String workMode;

void connectWIFI()
{
  Serial.print("[WIFI] Connecting");

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
    Serial.println();
    Serial.print("[WIFI] Connected | IP: ");
    Serial.println(WiFi.localIP());
    wifiEventPending = true;
  }
  else
  {
    Serial.println();
    Serial.print("[WIFI] Connection failed | status=");
    Serial.println(WiFi.status());
  }
}

void checkWIFI()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    return;
  }
  Serial.println("[WIFI] Disconnected");
  connectWIFI();
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
  if (workMode == "deep_sleep")
  {
    doc["wifi_status"] = false;
    doc["mqtt_status"] = false;
  }
  else
  {
    doc["wifi_status"] = true;
    doc["mqtt_status"] = true;
  }
  char payload[192];
  serializeJson(doc, payload);
  mqttClient.publish(MQTT_TOPIC_DEVICE_STATUS, payload);
  Serial.println("[MQTT] Device status sent");
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
  Serial.print("[MQTT] Event sent: ");
  Serial.println(eventType);
}

void connectMQTT()
{
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  int attempts = 0;

  while (!mqttClient.connected() && attempts < 5)
  {
    if (mqttClient.connect(
            MQTT_CLIENT_ID,
            MQTT_USERNAME,
            MQTT_PASSWORD))
    {
      Serial.println("[MQTT] Connected");
      mqttClient.subscribe(MQTT_TOPIC_CONFIG);
      Serial.println("[MQTT] Config topic subscribed");

      publishSystemEvent("MQTT_CONNECTED", "MQTT connection");
      if (wifiEventPending)
      {
        publishSystemEvent("WIFI_CONNECTED", "WiFi connection");
        wifiEventPending = false;
      }
      publishDeviceStatus();
    }
    else
    {
      Serial.print("[MQTT] Connection failed");
      Serial.println(mqttClient.state());

      attempts++;
      delay(2000);
    }
  }

  if (!mqttClient.connected())
  {
    Serial.println("[MQTT] Connection unavailable");
  }
}

void checkMQTT()
{
  if (mqttClient.connected())
  {
    return;
  }
  Serial.println("[MQTT] Disconnected");
  connectMQTT();
}

void publishMeasurements(bool alarmStatus)
{
  if (!mqttClient.connected())
  {
    return;
  }
  String alarmReason = "";
  if (alarmStatus)
  {
    if (temperature < tempMin)
      alarmReason += "LOW_TEMP";
    else if (temperature > tempMax)
      alarmReason += "HIGH_TEMP";
    if (humidity < humMin)
    {
      if (alarmReason != "")
        alarmReason += "_AND_";

      alarmReason += "LOW_HUM";
    }
    else if (humidity > humMax)
    {
      if (alarmReason != "")
        alarmReason += "_AND_";

      alarmReason += "HIGH_HUM";
    }
  }
  JsonDocument doc;
  doc["device"] = DEVICE_ID;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["alarm"] = alarmStatus;
  doc["alarm_reason"] = alarmReason;
  char payload[160];
  serializeJson(doc, payload);
  mqttClient.publish(MQTT_TOPIC_DATA, payload);
  Serial.println("[MQTT] Measurement sent");
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
    Serial.println("[MQTT] Invalid configuration JSON");
    return;
  }
  if (!doc["temp_min"].is<float>() ||
      !doc["temp_max"].is<float>() ||
      !doc["hum_min"].is<float>() ||
      !doc["hum_max"].is<float>())
  {
    Serial.println("[MQTT] Missing or invalid configuration");
    return;
  }
  tempMin = doc["temp_min"];
  tempMax = doc["temp_max"];
  humMin = doc["hum_min"];
  humMax = doc["hum_max"];
  Serial.println("[MQTT] New alarm thresholds");
  publishSystemEvent(
    "CONFIG_UPDATED",
    "Alarm thresholds updated");
}

void initNetwork()
{
  mqttClient.setCallback(handleMqttMessage);
}
bool isWiFiConnected()
{
  return WiFi.status() == WL_CONNECTED;
}
bool isMQTTConnected()
{
  return mqttClient.connected();
}
void processMQTT()
{
  if (mqttClient.connected())
  {
    mqttClient.loop();
  }
}
void disconnectMQTTForSleep()
{
  mqttClient.disconnect();
}