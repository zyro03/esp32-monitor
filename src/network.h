#pragma once
#include <Arduino.h>

void connectWIFI();
void checkWIFI();

void connectMQTT();
void checkMQTT();

void publishMeasurements(bool alarmStatus);
void publishDeviceStatus();
void publishSystemEvent(const char *eventType, const char *message);

void handleMqttMessage(char *topic, byte *payload, unsigned int length);

void initNetwork();
bool isWiFiConnected();
bool isMQTTConnected();
void processMQTT();
void disconnectMQTTForSleep();