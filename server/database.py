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
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    """)

    connection.execute("""
        CREATE TABLE IF NOT EXISTS alarm_events (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            device TEXT NOT NULL,
            temperature FLOAT NOT NULL,
            humidity FLOAT NOT NULL,
            reason TEXT NOT NULL,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
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
        LIMIT 20
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
        LIMIT 20
    """
    measurements = connection.execute(sql).fetchall()
    connection.close()
    return measurements
