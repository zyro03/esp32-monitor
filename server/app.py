from flask import (Flask, request, redirect, render_template, jsonify, session)
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
    get_system_events_last_24h,
    get_latest_system_events,
    get_measurements_for_chart,
)

from config import (MQTT_BROKER, MQTT_PORT, MQTT_TOPIC_CONFIG, FLASK_SECRET_KEY, ADMIN_USERNAME, ADMIN_PASSWORD_HASH,)
from werkzeug.security import check_password_hash

app = Flask(__name__)
app.secret_key = FLASK_SECRET_KEY
def publish_config(temp_min, temp_max, hum_min, hum_max):
    payload = {
        "temp_min": temp_min,
        "temp_max": temp_max,
        "hum_min": hum_min,
        "hum_max": hum_max,
    }

    client = mqtt.Client()
    client.connect(MQTT_BROKER, MQTT_PORT)
    client.publish(MQTT_TOPIC_CONFIG, json.dumps(payload), retain=True)
    client.disconnect()

    print("Config sent:", payload)

@app.route("/login", methods=["GET", "POST"])
def login():
    error = None
    if session.get("logged_in"):
        return redirect("/")
    if request.method == "POST":
        username = request.form.get("username", "")
        password = request.form.get("password", "")
        username_correct = username == ADMIN_USERNAME
        password_correct = check_password_hash(ADMIN_PASSWORD_HASH, password)

        if username_correct and password_correct:
            session.clear()
            session["logged_in"] = True
            session["username"] = username
            return redirect("/")
        error = "Nieprawidłowa nazwa użytkownika lub hasło."

    return render_template("login.html", error=error)


@app.route("/logout")
def logout():
    session.clear()
    return redirect("/login")

@app.route("/")
def index():
    if not session.get("logged_in"):
        return redirect("/login")
    
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
    if not session.get("logged_in"):
        return redirect("/login")
    temp_min = float(request.form["temp_min"])
    temp_max = float(request.form["temp_max"])
    hum_min = float(request.form["hum_min"])
    hum_max = float(request.form["hum_max"])

    update_settings(temp_min, temp_max, hum_min, hum_max)
    publish_config(temp_min, temp_max, hum_min, hum_max)

    return redirect("/")

@app.route("/measurements")
def measurements_page():
    if not session.get("logged_in"):
        return redirect("/login")
    
    measurements = get_measurements_last_24h()

    return render_template(
        "measurements.html",
        measurements=measurements,
    )


@app.route("/alarms")
def alarms_page():
    if not session.get("logged_in"):
        return redirect("/login")
    
    alarm_events = get_alarm_events_last_24h()

    return render_template(
        "alarms.html",
        alarm_events=alarm_events,
    )


@app.route("/events")
def status_page():
    if not session.get("logged_in"):
        return redirect("/login")
    system_events = get_system_events_last_24h()

    return render_template(
        "events.html",
        system_events=system_events,
    )

@app.route("/chart-data")
def chart_data():
    if not session.get("logged_in"):
        return redirect("/login")
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