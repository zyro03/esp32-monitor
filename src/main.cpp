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

}

void loop(){
  
}