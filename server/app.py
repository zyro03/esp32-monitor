from flask import Flask

app = Flask(__name__)


@app.route("/")
def index():
    return "esp32 monitor"


if __name__ == "__main__":
    app.run(debug=True)