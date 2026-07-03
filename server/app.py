from flask import Flask, request, redirect, render_template
import json
import paho.mqtt.client as mqtt

from database import (
    init_database,
    get_latest_measurements,
    get_latest_alarm_events,
    get_settings,
    update_settings,
)

from config import MQTT_BROKER, MQTT_PORT, MQTT_TOPIC_CONFIG

app = Flask(__name__)

def publish_config(temp_max, hum_max):
    payload = {
        "temp_max": temp_max,
        "hum_max": hum_max,
    }

    client = mqtt.Client()
    client.connect(MQTT_BROKER, MQTT_PORT)
    client.publish(MQTT_TOPIC_CONFIG, json.dumps(payload), retain=True)
    client.disconnect()

    print("Config sent:", payload)

@app.route("/")
def index():
    measurements = get_latest_measurements()
    alarm_events = get_latest_alarm_events()
    settings = get_settings()

    return render_template(
        "index.html",
        measurements=measurements,
        alarm_events=alarm_events,
        settings=settings,
    )

@app.route("/settings", methods=["POST"])
def settings():
    temp_max = float(request.form["temp_max"])
    hum_max = float(request.form["hum_max"])

    update_settings(temp_max, hum_max)
    publish_config(temp_max, hum_max)

    return redirect("/")

if __name__ == "__main__":
    init_database()
    app.run(debug=True, port=5001)