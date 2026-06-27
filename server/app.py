from flask import Flask

from database import init_database, get_latest_measurements

app = Flask(__name__)


@app.route("/")
def index():
    measurements = get_latest_measurements()

    page = "<h1>ESP32 measurements</h1>"

    for row in measurements:
        page += f"""
        <p>
            ID: {row[0]} |
            Device: {row[1]} |
            Temperature: {row[2]} C |
            Humidity: {row[3]} % |
            Alarm: {row[4]} |
            Time: {row[5]}
        </p>
        """

    return page


if __name__ == "__main__":
    init_database()
    app.run(debug=True)