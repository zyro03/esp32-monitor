from flask import (Flask, request, redirect, render_template, jsonify, session)
import json
import paho.mqtt.client as mqtt
from datetime import datetime
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
    get_all_measurements,
    get_all_alarm_events,
    get_all_system_events
)
from config import (MQTT_BROKER, MQTT_PORT, MQTT_TOPIC_CONFIG, FLASK_SECRET_KEY, ADMIN_USERNAME, ADMIN_PASSWORD_HASH,MQTT_USERNAME, MQTT_PASSWORD,)
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

    try:
        client = mqtt.Client()
        client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)
        client.connect(MQTT_BROKER, MQTT_PORT)
        result = client.publish(MQTT_TOPIC_CONFIG, json.dumps(payload), retain=True)
        result.wait_for_publish()
        client.disconnect()
        return True
    except Exception as e:
        print(e)
        return False    

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

    device_online = False
    if device_status:
        last_update = datetime.strptime(
            device_status[6], "%Y-%m-%d %H:%M:%S")
        difference = datetime.now() - last_update
        if difference.total_seconds() < 120:
            device_online = True

    return render_template(
        "index.html",
        measurements=measurements,
        alarm_events=alarm_events,
        settings=settings,
        device_status=device_status,
        system_events = system_events,
        device_online=device_online,
    )

@app.route("/settings", methods=["POST"])
def settings():
    if not session.get("logged_in"):
        return redirect("/login")
    try:
        temp_min = float(request.form["temp_min"])
        temp_max = float(request.form["temp_max"])
        hum_min = float(request.form["hum_min"])
        hum_max = float(request.form["hum_max"])
    except (KeyError, ValueError):
        print("[SETTINGS] Invalid form data")
        return redirect("/")
    if not (-40.0 <= temp_min < temp_max <= 80.0):
        print("[SETTINGS] Invalid temperature range")
        return redirect("/")
    if not (0.0 <= hum_min < hum_max <= 100.0):
        print("[SETTINGS] Invalid humidity range")
        return redirect("/")
    success = publish_config(
        temp_min,
        temp_max,
        hum_min,
        hum_max
    )
    if success:
        update_settings(
            temp_min,
            temp_max,
            hum_min,
            hum_max
        )
    else:
        print("[SETTINGS] MQTT configuration publish failed")
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
def events_page():
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

@app.route("/measurements/history")
def measurements_history():
    if not session.get("logged_in"):
        return redirect("/login")
    measurements = get_all_measurements()
    return render_template(
        "measurements_history.html",
        measurements=measurements)


@app.route("/alarms/history")
def alarms_history():
    if not session.get("logged_in"):
        return redirect("/login")
    alarm_events = get_all_alarm_events()
    return render_template(
        "alarms_history.html",
        alarm_events=alarm_events)


@app.route("/events/history")
def events_history():
    if not session.get("logged_in"):
        return redirect("/login")
    system_events = get_all_system_events()
    return render_template(
        "events_history.html",
        system_events=system_events)

if __name__ == "__main__":
    init_database()
    app.run(
        host="0.0.0.0",
        port=5001,
        debug=True)