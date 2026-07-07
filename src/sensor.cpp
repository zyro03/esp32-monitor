#include "sensor.h"

#include <Arduino.h>
#include <DHT.h>

#define DHT_PIN 4
#define DHT_TYPE DHT22

DHT dht(DHT_PIN, DHT_TYPE);

extern float temperature;
extern float humidity;
extern float tempMax;
extern float humMax;

void initSensor()
{
    dht.begin();
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

