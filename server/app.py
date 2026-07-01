from flask import Flask, request, redirect
from html import escape
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
    payload = {"temp_max": temp_max, "hum_max": hum_max}
    client = mqtt.Client()
    client.connect(MQTT_BROKER, MQTT_PORT)
    client.publish(MQTT_TOPIC_CONFIG, json.dumps(payload))
    client.disconnect()
    print("Config sent:", payload)


@app.route("/")
def index():
    measurements = get_latest_measurements()
    alarm_events = get_latest_alarm_events()
    settings = get_settings()
    page = f"""
    <meta http-equiv="refresh" content="5">
    <h1>ESP32 measurements</h1>
    <h2> Alarm settings</h2>
    <form method = "POST" action="/settings">
        <label>MAX temperature:</label>
        <input type="number" step="0.1" name="temp_max" value="{settings['temp_max']}">
        <br><br>
        <label>MAX humidity:</label>
        <input type="number" step="0.1" name="hum_max" value="{settings['hum_max']}">
        <br><br>
        <button type="submit">Save settings</button>
    </form>
    <h2>Latest measurements</h2>
    <table border="1">
        <tr>
            <th>ID</th>
            <th>Device</th>
            <th>Temperature</th>
            <th>Humidity</th>
            <th>Alarm</th>
            <th>Time</th>
        </tr>
    """
    for row in measurements:
        page += f"""
        <tr>
            <td>{row[0]}</td>
            <td>{row[1]}</td>
            <td>{row[2]}</td>
            <td>{row[3]}</td>
            <td>{row[4]}</td>
            <td>{row[5]}</td>
        </tr>
        """
    page += """</table>"""    

    page += """
    <h2>Alarm events</h2>
    <table border="1">
        <tr>
            <th>ID</th>
            <th>Device</th>
            <th>Temperature</th>
            <th>Humidity</th>
            <th>Reason</th>
            <th>Time</th>
        </tr>
    """
    for row in alarm_events:
        page += f"""
        <tr>
            <td>{row[0]}</td>
            <td>{row[1]}</td>
            <td>{row[2]}</td>
            <td>{row[3]}</td>
            <td>{row[4]}</td>
            <td>{row[5]}</td>
        </tr>
        """
    page += """</table>"""
    return page


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