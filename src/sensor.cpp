#include "sensor.h"

#include <Arduino.h>
#include <DHT.h>

#define DHT_PIN 19
#define DHT_TYPE DHT22

DHT dht(DHT_PIN, DHT_TYPE);

extern float temperature;
extern float humidity;

extern float tempMin;
extern float tempMax;

extern float humMin;
extern float humMax;

void initSensor()
{
  dht.begin();
}

bool readSensor()
{
    float newTemperature = dht.readTemperature();
    float newHumidity = dht.readHumidity();

    if (isnan(newTemperature) || isnan(newHumidity))
    {
        return false;
    }

    temperature = newTemperature;
    humidity = newHumidity;

    return true;
}

bool checkAlarm()
{
  if (temperature > tempMax || humidity > humMax || temperature < tempMin || humidity < humMin)
  {
    return true;
  }
  else
  {
    return false;
  }
}
