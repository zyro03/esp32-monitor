import json
import paho.mqtt.client as mqtt
from database import init_database, save_measurement, save_alarm_event, save_system_event, save_device_status, get_settings
from config import MQTT_BROKER, MQTT_PORT, MQTT_TOPIC_DATA, MQTT_TOPIC_EVENT, MQTT_TOPIC_DEVICE_STATUS

def handle_message(client, userdata, message):
    topic = message.topic
    data = json.loads(message.payload.decode("utf-8"))
    if topic == MQTT_TOPIC_DATA:
        save_measurement(
            data["device"],
            data["temperature"],
            data["humidity"],
            data["alarm"]
        )
        if data["alarm"]:
            settings = get_settings()
            temp_alarm = data["temperature"] > settings["temp_max"]
            hum_alarm = data["humidity"] > settings["hum_max"]

            if temp_alarm and hum_alarm:
                reason = "HIGH_TEMP_AND_HUM"
            elif temp_alarm:
                reason = "HIGH_TEMP"
            elif hum_alarm:
                reason = "HIGH_HUM"
            else:
                reason = "ALARM"

            save_alarm_event(
                data["device"],
                data["temperature"],
                data["humidity"],
                reason
            )
            print("Alarm event saved")
        print("Saved measurement:", data)
    elif topic == MQTT_TOPIC_EVENT:
        save_system_event(
            data["device"],
            data["event_type"],
            data.get("message", ""))
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

client = mqtt.Client()
client.on_message = handle_message

client.connect(MQTT_BROKER, MQTT_PORT)
client.subscribe(MQTT_TOPIC_DATA)
client.subscribe(MQTT_TOPIC_EVENT)
client.subscribe(MQTT_TOPIC_DEVICE_STATUS)

print("MQTT client started")
print("Subscribed:", MQTT_TOPIC_DATA)
print("Subscribed:", MQTT_TOPIC_EVENT)
print("Subscribed:", MQTT_TOPIC_DEVICE_STATUS)

client.loop_forever()