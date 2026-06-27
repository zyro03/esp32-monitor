from flask import Flask
from database import init_database

app = Flask(__name__)


@app.route("/")
def index():
    return "esp32 monitor"


if __name__ == "__main__":
    init_database()
    app.run(debug=True)