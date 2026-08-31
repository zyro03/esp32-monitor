import json
import paho.mqtt.client as mqtt
from database import init_database, save_measurement, save_alarm_event, save_system_event, save_device_status, get_settings, get_last_alarm_state
from config import MQTT_BROKER, MQTT_PORT, MQTT_TOPIC_DATA, MQTT_TOPIC_EVENT, MQTT_TOPIC_DEVICE_STATUS, MQTT_USERNAME, MQTT_PASSWORD, MQTT_TOPIC_CONFIG

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT broker")
        client.subscribe(MQTT_TOPIC_DATA)
        client.subscribe(MQTT_TOPIC_EVENT)
        client.subscribe(MQTT_TOPIC_DEVICE_STATUS)
        print("Subscribed:", MQTT_TOPIC_DATA)
        print("Subscribed:", MQTT_TOPIC_EVENT)
        print("Subscribed:", MQTT_TOPIC_DEVICE_STATUS)
        settings = get_settings()
        config_payload = {
            "temp_min": settings["temp_min"],
            "temp_max": settings["temp_max"],
            "hum_min": settings["hum_min"],
            "hum_max": settings["hum_max"]}
        client.publish(
            MQTT_TOPIC_CONFIG,
            json.dumps(config_payload),
            retain=True)
        print("Current configuration published:", config_payload)
    else:
        print("MQTT connection failed, code:", rc)

def handle_message(client, userdata, message):
    global last_alarm_state

    topic = message.topic

    try:
        data = json.loads(message.payload.decode("utf-8"))
    except (json.JSONDecodeError, UnicodeDecodeError):
        print("Invalid MQTT message")
        return

    if topic == MQTT_TOPIC_DATA:
        save_measurement(
            data["device"],
            data["temperature"],
            data["humidity"],
            data["alarm"]
        )

        if data["alarm"] and not last_alarm_state:
            reason = data.get("alarm_reason", "")

            if not reason:
                settings = get_settings()

                reasons = []

                temperature = data["temperature"]
                humidity = data["humidity"]

                if temperature < settings["temp_min"]:
                    reasons.append("LOW_TEMP")
                elif temperature > settings["temp_max"]:
                    reasons.append("HIGH_TEMP")

                if humidity < settings["hum_min"]:
                    reasons.append("LOW_HUM")
                elif humidity > settings["hum_max"]:
                    reasons.append("HIGH_HUM")

                reason = "_AND_".join(reasons)

            save_alarm_event(
                data["device"],
                data["temperature"],
                data["humidity"],
                reason
            )

            save_system_event(
                data["device"],
                "ALARM_ON",
                "Alarm activated"
            )

            print("Alarm event saved")
            print("Alarm ON system event saved")

        elif not data["alarm"] and last_alarm_state:
            save_system_event(
                data["device"],
                "ALARM_OFF",
                "Alarm ended - parameters returned to normal"
            )
            print("Alarm OFF event saved")
        last_alarm_state = data["alarm"]
        print("Saved measurement:", data)
    elif topic == MQTT_TOPIC_EVENT:
        save_system_event(
            data["device"],
            data["event_type"],
            data.get("message", "")
        )
        print("Saved system event:", data)
    elif topic == MQTT_TOPIC_DEVICE_STATUS:
        save_device_status(
            data["device"],
            data["power_source"],
            data["work_mode"],
            data["wifi_status"],
            data["mqtt_status"])
        print("Saved device status:", data)
init_database()
last_alarm_state = get_last_alarm_state()
print("Last alarm state loaded from database:", last_alarm_state)

client = mqtt.Client()

client.on_connect = on_connect
client.on_message = handle_message
client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)
client.reconnect_delay_set(min_delay=2, max_delay=10)
client.connect_async(MQTT_BROKER, MQTT_PORT)
print("MQTT client started")
client.loop_forever(retry_first_connection=True)