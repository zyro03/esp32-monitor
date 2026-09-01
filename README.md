# ESP32 Monitor

Projekt przedstawia prototyp systemu bezprzewodowego monitorowania warunków środowiskowych.

## Konfiguracja

Przed uruchomieniem należy utworzyć właściwe pliki konfiguracyjne na podstawie plików przykładowych:

`include/secrets_example.h` -> `include/secrets.h`

`server/config_example.py` -> `server/config.py`

`mosquitto/mosquitto_example.conf` -> `mosquitto/mosquitto.conf`

`mosquitto/mosquitto_passwd_example` -> `mosquitto/mosquitto_passwd`

## Uruchomienie

### 1. ESP32

Uzupełnij dane Wi-Fi i MQTT w pliku:

`include/secrets.h`

Projekt można zbudować i wgrać na ESP32 za pomocą PlatformIO.

### 2. Mosquitto

Utwórz plik haseł brokera MQTT, a następnie ustaw jego ścieżkę w:

`mosquitto/mosquitto.conf`

Przykładowe uruchomienie na Windows:

`& "C:\Program Files\mosquitto\mosquitto.exe" -c "C:\ścieżka\do\esp32-monitor\mosquitto\mosquitto.conf" -v`

Przykładowe uruchomienie na macOS:

`mosquitto -c /ścieżka/do/esp32-monitor/mosquitto/mosquitto.conf -v`

### 3. Serwer Python

Przejdź do katalogu:

`server`

Utwórz środowisko wirtualne:

Windows:

`python -m venv venv`

macOS/Linux:

`python3 -m venv venv`

Aktywuj środowisko.

Windows:

`.\venv\Scripts\Activate.ps1`

macOS/Linux:

`source venv/bin/activate`

Zainstaluj wymagane biblioteki:

`pip install -r requirements.txt`

Uzupełnij konfigurację w pliku:

`server/config.py`

Następnie uruchom klienta MQTT:

`python mqtt_client.py`

W drugim terminalu uruchom aplikację Flask:

`python app.py`

## Panel WWW

Panel jest dostępny pod adresem:

`http://127.0.0.1:5001`

Aplikacja umożliwia między innymi:

- podgląd aktualnej temperatury i wilgotności,
- sprawdzenie stanu urządzenia,
- sprawdzenie źródła zasilania,
- podgląd stanu alarmowego,
- przeglądanie historii pomiarów,
- wyświetlanie wykresów,
- przeglądanie historii alarmów i zdarzeń,
- zmianę progów alarmowych.

## Bezpieczeństwo konfiguracji

Dane dostępowe do Wi-Fi, MQTT oraz panelu administratora nie są przechowywane bezpośrednio w repozytorium.

Repozytorium zawiera wyłącznie przykładowe pliki konfiguracyjne przeznaczone do utworzenia lokalnej konfiguracji systemu.
