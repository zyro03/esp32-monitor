#include "display.h"
#include "network.h"
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

extern float temperature;
extern float humidity;
extern float tempMax;
extern String powerSource;

void lcdMeasurements()
{
  lcd.setCursor(0, 0);
  lcd.print("TEMP: ");
  lcd.print(temperature, 1);
  lcd.print(" C");

  lcd.setCursor(0, 1);
  lcd.print("WILG: ");
  lcd.print(humidity, 1);
  lcd.print(" %");
}

void lcdSensorError()
{
  lcd.setCursor(0, 0);
  lcd.print("DHT22 ERROR");
  lcd.setCursor(0, 1);
  lcd.print("Sprawdz sensor!");
}

void lcdAlarm()
{
  lcd.setCursor(0, 0);
  lcd.print("ALARM!");
  lcd.setCursor(0, 1);
  if (temperature > tempMax)
  {
    lcd.print("Alert temperatura!");
  }
  else
  {
    lcd.print("Alert wilgotność!");
  }
}

void lcdBatterySleep()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Z: AKU");
  lcd.setCursor(0, 1);
  lcd.print("T:");
  lcd.print(temperature, 1);
  lcd.print("C");
  lcd.setCursor(9, 1);
  lcd.print("W:");
  lcd.print(humidity, 1);
  lcd.print("%");
}

void lcdStatus()
{
  lcd.setCursor(0, 0);
  if (isWiFiConnected())
  {
    lcd.print("WiFi:OK");
  }
  else
  {
    lcd.print("WiFi:OFF");
  }
  lcd.setCursor(9, 0);
  if (isMQTTConnected())
  {
    lcd.print("MQ:OK");
  }
  else
  {
    lcd.print("MQ:OFF");
  }
  lcd.setCursor(0, 1);
  lcd.print("Z: ");
  if (powerSource == "main")
  {
    lcd.print("GLOWNE");
  }
  else
  {
    lcd.print("BATERIA");
  }
}

void initDisplay()
{
    Wire.begin(21, 22);
    lcd.init();
    lcd.backlight();
    lcd.clear();
}

void clearDisplay(){
    lcd.clear();
}

void disableDisplay(){
    lcd.clear();
    lcd.noBacklight();
}