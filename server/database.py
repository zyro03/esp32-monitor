import sqlite3

DATABASE_NAME = "measurements.db"

def init_database():
    connection = sqlite3.connect(DATABASE_NAME)

    connection.execute("""
        CREATE TABLE IF NOT EXISTS measurements (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            device TEXT NOT NULL,
            temperature FLOAT NOT NULL,
            humidity FLOAT NOT NULL,
            alarm INTEGER NOT NULL,
            created_at TIMESTAMP DEFAULT (datetime('now', 'localtime'))
        )
    """)

    connection.execute("""
        CREATE TABLE IF NOT EXISTS alarm_events (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            device TEXT NOT NULL,
            temperature FLOAT NOT NULL,
            humidity FLOAT NOT NULL,
            reason TEXT NOT NULL,
            created_at TIMESTAMP DEFAULT (datetime('now', 'localtime'))
        )
    """)

    connection.execute("""
        CREATE TABLE IF NOT EXISTS system_events (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            device TEXT NOT NULL,
            event_type TEXT NOT NULL,
            message TEXT,
            created_at TIMESTAMP DEFAULT (datetime('now', 'localtime'))
        )
    """)

    connection.execute("""
    CREATE TABLE IF NOT EXISTS settings (
        name TEXT PRIMARY KEY,
        value REAL NOT NULL
    )
    """)

    connection.execute("""
    INSERT OR IGNORE INTO settings (name, value)
    VALUES
    ('temp_min', 10.0),
    ('temp_max', 40.0),
    ('hum_min', 10.0),
    ('hum_max', 70.0)
    """)

    connection.execute("""
    CREATE TABLE IF NOT EXISTS device_status (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        device TEXT NOT NULL,
        power_source TEXT NOT NULL,
        work_mode TEXT NOT NULL,
        wifi_status INTEGER NOT NULL,
        mqtt_status INTEGER NOT NULL,
        created_at TIMESTAMP DEFAULT (datetime('now', 'localtime'))
    )
    """)

    connection.commit()
    connection.close()

def save_measurement(device, temperature,humidity, alarm):
    connection = sqlite3.connect(DATABASE_NAME)
    connection.execute("""
        INSERT INTO measurements (device, temperature, humidity, alarm)
        VALUES (?, ?, ?, ?)
        """,
        (device, temperature, humidity, int(alarm)))
    connection.commit()
    connection.close()

def save_alarm_event(device, temperature, humidity, reason):
    connection = sqlite3.connect(DATABASE_NAME)
    connection.execute("""
        INSERT INTO alarm_events (device, temperature, humidity, reason)
        VALUES (?, ?, ?, ?)
        """,
        (device, temperature, humidity, reason))
    connection.commit()
    connection.close()

def get_latest_alarm_events():
    connection = sqlite3.connect(DATABASE_NAME)
    sql = """
        SELECT id, device, temperature, humidity, reason, created_at
        FROM alarm_events
        ORDER BY id DESC
        LIMIT 5
    """
    alarm_events = connection.execute(sql).fetchall()
    connection.close()
    return alarm_events

def get_latest_measurements():
    connection = sqlite3.connect(DATABASE_NAME)
    sql = """
        SELECT id, device, temperature, humidity, alarm, created_at
        FROM measurements
        ORDER BY id DESC
        LIMIT 5
    """
    measurements = connection.execute(sql).fetchall()
    connection.close()
    return measurements

def get_settings():
    connection = sqlite3.connect(DATABASE_NAME)
    rows = connection.execute("""
        SELECT name, value FROM settings
    """).fetchall()
    connection.close()
    settings = {}
    for row in rows:
        settings[row[0]]=row[1]
    return settings

def update_settings(temp_min, temp_max, hum_min, hum_max):
    connection = sqlite3.connect(DATABASE_NAME)
    connection.execute("""
    UPDATE settings SET value = ? WHERE name = 'temp_min'
    """, (temp_min,))
    connection.execute("""
    UPDATE settings SET value = ? WHERE name = 'temp_max'
    """, (temp_max,))
    connection.execute("""
    UPDATE settings SET value = ? WHERE name = 'hum_min'
    """, (hum_min,))
    connection.execute("""
    UPDATE settings SET value = ? WHERE name = 'hum_max'
    """, (hum_max,))
    connection.commit()
    connection.close()

def save_system_event(device, event_type, message):
    connection = sqlite3.connect(DATABASE_NAME)
    connection.execute("""
        INSERT INTO system_events (device, event_type, message)
        VALUES (?, ?, ?)
    """, (device, event_type, message))
    connection.commit()
    connection.close()

def save_device_status(device, power_source, work_mode, wifi_status, mqtt_status):
    connection = sqlite3.connect(DATABASE_NAME)
    connection.execute("""
        INSERT INTO device_status (device, power_source, work_mode, wifi_status, mqtt_status)
        VALUES (?, ?, ?, ?, ?)
    """, (device, power_source, work_mode, int(wifi_status), int(mqtt_status)))
    connection.commit()
    connection.close()

def get_latest_device_status():
    connection = sqlite3.connect(DATABASE_NAME)
    row = connection.execute("""
        SELECT id, device, power_source, work_mode, wifi_status, mqtt_status, created_at
        FROM device_status
        ORDER BY id DESC
        LIMIT 1 """).fetchone()
    connection.close()
    return row

def get_measurements_last_24h():
    connection = sqlite3.connect(DATABASE_NAME)
    sql = """
        SELECT id, device, temperature, humidity, alarm, created_at
        FROM measurements
        WHERE created_at >= datetime('now', 'localtime', '-24 hours')
        ORDER BY id DESC
    """
    measurements = connection.execute(sql).fetchall()
    connection.close()
    return measurements

def get_alarm_events_last_24h():
    connection = sqlite3.connect(DATABASE_NAME)
    sql = """
        SELECT id, device, temperature, humidity, reason, created_at
        FROM alarm_events
        WHERE created_at >= datetime('now', 'localtime', '-24 hours')
        ORDER BY id DESC
    """
    alarm_events = connection.execute(sql).fetchall()
    connection.close()
    return alarm_events

def get_system_events_last_24h():
    connection = sqlite3.connect(DATABASE_NAME)
    sql = """
        SELECT id, event_type, message, created_at
        FROM system_events
        WHERE created_at >= datetime('now', 'localtime', '-24 hours')
        ORDER BY id DESC
    """
    system_events = connection.execute(sql).fetchall()
    connection.close()
    return system_events

def get_latest_system_events():
    connection = sqlite3.connect(DATABASE_NAME)
    sql = """
        SELECT id, event_type, message, created_at
        FROM system_events
        ORDER BY id DESC
        LIMIT 5
    """
    system_events = connection.execute(sql).fetchall()
    connection.close()
    return system_events

def get_measurements_for_chart():
    connection = sqlite3.connect(DATABASE_NAME)

    sql = """
        SELECT
            AVG(temperature) AS avg_temperature,
            AVG(humidity) AS avg_humidity,
            strftime('%Y-%m-%d %H:%M', created_at) AS minute
        FROM measurements
        WHERE created_at >= datetime('now', 'localtime', '-24 hours')
        GROUP BY strftime('%Y-%m-%d %H:%M', created_at)
        ORDER BY minute ASC
    """

    rows = connection.execute(sql).fetchall()
    connection.close()

    return rows

def get_all_measurements():
    connection = sqlite3.connect(DATABASE_NAME)
    sql = """
        SELECT id, device, temperature, humidity, alarm, created_at
        FROM measurements
        ORDER BY id DESC
    """
    measurements = connection.execute(sql).fetchall()
    connection.close()
    return measurements


def get_all_alarm_events():
    connection = sqlite3.connect(DATABASE_NAME)
    sql = """
        SELECT id, device, temperature, humidity, reason, created_at
        FROM alarm_events
        ORDER BY id DESC
    """
    alarm_events = connection.execute(sql).fetchall()
    connection.close()
    return alarm_events


def get_all_system_events():
    connection = sqlite3.connect(DATABASE_NAME)
    sql = """
        SELECT id, event_type, message, created_at
        FROM system_events
        ORDER BY id DESC
    """
    system_events = connection.execute(sql).fetchall()
    connection.close()
    return system_events

def get_last_alarm_state():
    connection = sqlite3.connect(DATABASE_NAME)
    row = connection.execute("""
        SELECT alarm
        FROM measurements
        ORDER BY id DESC
        LIMIT 1
    """).fetchone()
    connection.close()
    if row is None:
        return False
    return bool(row[0])