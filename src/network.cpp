#include "network.h"
#include "secrets.h"

#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>

#define DEVICE_ID "nr1"
#define MQTT_CLIENT_ID "esp32_nr1"

#define MQTT_TOPIC_DATA "esp32/" DEVICE_ID "/data"
#define MQTT_TOPIC_STATUS "esp32/" DEVICE_ID "/status"
#define MQTT_TOPIC_CONFIG "esp32/" DEVICE_ID "/config"
#define MQTT_TOPIC_EVENT "esp32/" DEVICE_ID "/event"
#define MQTT_TOPIC_DEVICE_STATUS "esp32/" DEVICE_ID "/device_status"

WiFiClient espClient;
PubSubClient mqttClient(espClient);

extern float temperature;
extern float humidity;
extern float tempMax;
extern float humMax;
extern String powerSource;
extern String workMode;

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

void checkWIFI()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    return;
  }
  Serial.println("WI-Fi disconnected");
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
  doc["wifi_status"] = WiFi.status() == WL_CONNECTED;
  doc["mqtt_status"] = mqttClient.connected();
  char payload[192];
  serializeJson(doc, payload);
  mqttClient.publish(MQTT_TOPIC_DEVICE_STATUS, payload);
  Serial.print("MQTT device status sent: ");
  Serial.println(payload);
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
    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_TOPIC_STATUS, 0, true,
                           "offline"))
    {
      Serial.println("MQTT connected");

      mqttClient.publish(MQTT_TOPIC_STATUS, "online", true);
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
    if(mqttClient.connected()){
        mqttClient.loop();
    }
}
void disconnectMQTTForSleep(){
    mqttClient.publish(MQTT_TOPIC_STATUS, "offline", true);
    mqttClient.loop();
    delay(200);
    mqttClient.disconnect();
}