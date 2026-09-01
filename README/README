# ESP32 Monitor

Projekt przedstawia prototyp systemu monitorowania temperatury i wilgotności z wykorzystaniem ESP32, czujnika DHT22, komunikacji MQTT, serwera Flask oraz bazy SQLite.

## Uruchomienie

1. Przygotuj pliki konfiguracyjne na podstawie plików przykładowych:

include/secrets_example.h - include/secrets.h
server/config_example.py - server/config.py
mosquitto/mosquitto_example.conf - mosquitto/mosquitto.conf
mosquitto/mosquitto_passwd_example - mosquitto/mosquitto_passwd

2. Uzupełnij dane Wi-Fi oraz MQTT.

3. Uruchom broker Mosquitto.

Windows:
& "C:\Program Files\mosquitto\mosquitto.exe" -c "C:\ścieżka\do\esp32-monitor\mosquitto\mosquitto.conf" -v

macOS:
mosquitto -c /ścieżka/do/esp32-monitor/mosquitto/mosquitto.conf -v

4. Przejdź do katalogu server i aktywuj środowisko wirtualne.

Windows:
.\venv\Scripts\Activate.ps1

macOS:
source venv/bin/activate

5. Uruchom:

python mqtt_client.py
python app.py

6. Otwórz panel WWW:

http://127.0.0.1:5001
