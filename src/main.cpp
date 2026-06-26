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
 */

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

#define DHT_PIN 4
#define DHT_TYPE DHT22

DHT dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);


int counter = 0;

void setup(){
  Serial.begin(115200);
  delay(1000);
  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  dht.begin();}

void loop(){
  counter++;
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  lcd.clear();
  
  if (isnan(temperature) || isnan(humidity)){
    Serial.println("DHT22 ERROR");
    lcd.setCursor(0,0);
    lcd.print("DHT22 ERROR");
  }
  else{
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" C");

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");

    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(temperature, 1);
    lcd.print(" C");

    lcd.setCursor(0, 1);
    lcd.print("Humi: ");
    lcd.print(humidity, 1);
    lcd.print(" %");
  }
  delay(10000);
}