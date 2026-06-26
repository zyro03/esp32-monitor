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

#define TEMP_MAX 30.0
#define HUM_MAX 70.0

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
  dht.begin();
  
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(BUZZER_PIN, HIGH);
  digitalWrite(LED_PIN, LOW);
}

void loop(){
  counter++;
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  lcd.clear();
  
  if (isnan(temperature) || isnan(humidity)){
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, HIGH);
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

    bool alarm = false;
    if(temperature > TEMP_MAX || humidity > HUM_MAX){
      alarm = true;}
      
    if (alarm){
      digitalWrite(LED_PIN, HIGH);
      digitalWrite(BUZZER_PIN, LOW);
      Serial.println("ALARM");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("ALARM");
      lcd.setCursor(0,1);
      if(temperature > TEMP_MAX){
        lcd.print("High temp");}
        else{
          lcd.print("High humi");}}
    else {
      digitalWrite(LED_PIN, LOW);
      digitalWrite(BUZZER_PIN, HIGH);
      }
  }
  delay(2000);
}