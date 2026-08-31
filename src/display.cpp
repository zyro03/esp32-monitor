#include "display.h"
#include "network.h"
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

extern float temperature;
extern float humidity;
extern float tempMax;
extern float tempMin;
extern float humMax;
extern float humMin;
extern String powerSource;

void lcdMeasurements()
{
    lcd.setCursor(0, 0);
    lcd.print("Temp:   ");
    lcd.print(temperature, 1);
    lcd.print(" C  ");

    lcd.setCursor(0, 1);
    lcd.print("Wilg:   ");
    lcd.print(humidity, 1);
    lcd.print(" %  ");
}

void lcdAlarm()
{
    bool tempLow = temperature < tempMin;
    bool tempHigh = temperature > tempMax;

    bool humLow = humidity < humMin;
    bool humHigh = humidity > humMax;

    bool tempAlarm = tempLow || tempHigh;
    bool humAlarm = humLow || humHigh;

    lcd.setCursor(0, 0);
    lcd.print("ALARM!          ");

    lcd.setCursor(0, 1);

    if (tempAlarm && humAlarm)
    {
        if (tempLow)
        {
            lcd.print("T:NIS ");
        }
        else
        {
            lcd.print("T:WYS ");
        }

        if (humLow)
        {
            lcd.print("H:NIS     ");
        }
        else
        {
            lcd.print("H:WYS     ");
        }
    }
    else if (tempLow)
    {
        lcd.print("TEMP ZA NISKA   ");
    }
    else if (tempHigh)
    {
        lcd.print("TEMP ZA WYSOKA  ");
    }
    else if (humLow)
    {
        lcd.print("WILG ZA NISKA   ");
    }
    else if (humHigh)
    {
        lcd.print("WILG ZA WYSOKA  ");
    }
    else
    {
        lcd.print("ALARM           ");
    }
}

void lcdBatterySleep()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ZASILANIE: AKU  ");
    lcd.setCursor(0, 1);
    lcd.print("T:");
    lcd.print(temperature, 1);
    lcd.print("C");
    lcd.setCursor(9, 1);
    lcd.print("W:");
    lcd.print(humidity, 1);
    lcd.print("%");
}

void initDisplay()
{
    Wire.begin(17, 21);
    lcd.init();
    lcd.backlight();
    lcd.clear();
}
void lcdStatus()
{
    lcd.setCursor(0, 0);

    if (powerSource == "main")
    {
        lcd.print("Zasilanie: SIEC ");
    }
    else
    {
        lcd.print("Zasilanie: AKU  ");
    }

    lcd.setCursor(0, 1);

    if (isMQTTConnected())
    {
        lcd.print("MQTT: POLACZONY ");
    }
    else
    {
        lcd.print("MQTT: BRAK      ");
    }
}

void disableDisplay()
{
    lcd.clear();
    lcd.noBacklight();
}