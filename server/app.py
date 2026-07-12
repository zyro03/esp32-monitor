from flask import Flask, request, redirect, render_template, jsonify
import json
import paho.mqtt.client as mqtt

from database import (
    init_database,
    get_latest_measurements,
    get_latest_alarm_events,
    get_latest_device_status,
    get_settings,
    update_settings,
    get_measurements_last_24h,
    get_alarm_events_last_24h,
    get_device_status_last_24h,
    get_latest_system_events,
    get_measurements_for_chart,
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
    system_events = get_latest_system_events()
    device_status = get_latest_device_status()

    return render_template(
        "index.html",
        measurements=measurements,
        alarm_events=alarm_events,
        settings=settings,
        device_status=device_status,
        system_events = system_events
    )

@app.route("/settings", methods=["POST"])
def settings():
    temp_max = float(request.form["temp_max"])
    hum_max = float(request.form["hum_max"])

    update_settings(temp_max, hum_max)
    publish_config(temp_max, hum_max)

    return redirect("/")

@app.route("/measurements")
def measurements_page():
    measurements = get_measurements_last_24h()

    return render_template(
        "measurements.html",
        measurements=measurements,
    )


@app.route("/alarms")
def alarms_page():
    alarm_events = get_alarm_events_last_24h()

    return render_template(
        "alarms.html",
        alarm_events=alarm_events,
    )


@app.route("/status")
def status_page():
    device_statuses = get_device_status_last_24h()

    return render_template(
        "status.html",
        device_statuses=device_statuses,
    )

@app.route("/chart-data")
def chart_data():
    rows = get_measurements_for_chart()

    labels = []
    temperatures = []
    humidities = []

    for row in rows:
        temperatures.append(row[0])
        humidities.append(row[1])
        labels.append(row[2][11:16])

    return jsonify({
        "labels": labels,
        "temperatures": temperatures,
        "humidities": humidities
    })

if __name__ == "__main__":
    init_database()
    app.run(debug=True, port=5001)