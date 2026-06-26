#include <Arduino.h>

int licznik = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Start programu");
  Serial.println("Test serial monitora");
}

void loop() {
  licznik++;

  Serial.print("Komunikat nr.: ");
  Serial.println(licznik);

  Serial.print("Czas: ");
  Serial.print(millis() / 1000);

  delay(1000);
}