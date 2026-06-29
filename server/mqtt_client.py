import json
import paho.mqtt.client as mqtt

from database import init_database, save_measurement, save_alarm_event

MQTT_BROKER = "192.168.0.167"
MQTT_TOPIC = "esp32/nr1/data"

def handle_message(client, userdata, message):
    data = json.loads(message.payload.decode("utf-8"))

    save_measurement(
        data["device"],
        data["temperature"],
        data["humidity"],
        data["alarm"]
    )
    if data["alarm"]:
        reason = "High temperature or humidity"
        save_alarm_event(
            data["device"],
            data["temperature"],
            data["humidity"],
            reason
        )
        print("Alarm event saved")
    print("Saved:", data)

init_database()

client = mqtt.Client()
client.on_message = handle_message

client.connect(MQTT_BROKER, 1883)
client.subscribe(MQTT_TOPIC)

print("MQTT client started")

client.loop_forever()