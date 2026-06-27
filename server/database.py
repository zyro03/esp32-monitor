import sqlite3

DATABASE_NAME = "measurements.db"

def init_database():
    connection = sqlite3.connect(DATABASE_NAME)

    connection.execute("""
        CREATE TABLE IF NOT EXISTS measurements (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            device TEXT NOT NULL,
            temperature REAL NOT NULL,
            humidity REAL NOT NULL,
            alarm INTEGER NOT NULL,
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

def get_latest_measurements():
    connection = sqlite3.connect(DATABASE_NAME)
    sql = """
        SELECT id, device, temperature, humidity, alarm, created_at
        FROM measurements
        ORDER BY id DESC
        LIMIT 100
    """
    measurements = connection.execute(sql).fetchall()
    connection.close()
    return measurements
