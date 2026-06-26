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

#define DHT_PIN 4
#define DHT_TYPE DHT22

#define BUZZER_PIN 16
#define LED_PIN 18

#define BUZZER_ON LOW
#define BUZZER_OFF HIGH

#define TEMP_MAX 30.0
#define HUM_MAX 70.0

DHT dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

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
}

void loop()
{
  lcd.clear();
  bool sensorStatus = readSensor();

  if (!sensorStatus)
  {
    alarmOFF();
    lcdSensorError();
  }
  else
  {
    bool alarmStatus = checkAlarm();
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
  delay(2000);
}