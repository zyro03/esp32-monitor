from flask import Flask

from database import init_database, get_latest_measurements, get_latest_alarm_events

app = Flask(__name__)


@app.route("/")
def index():
    measurements = get_latest_measurements()
    alarm_events = get_latest_alarm_events()
    page = """
    <h1>ESP32 measurements</h1>
    
    <table border="1">
        <tr>
            <th>ID</th>
            <th>Device</th>
            <th>Temperature</th>
            <th>Humidity</th>
            <th>Alarm</th>
            <th>Time</th>
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
    <h1>Alarm events</h2>
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


if __name__ == "__main__":
    init_database()
    app.run(debug=True, port=5001)