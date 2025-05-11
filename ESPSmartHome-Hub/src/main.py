# prototyping

import flask
import time

app = flask.Flask(__name__)

data = {
    "rooms": {}
}

@app.route("/update", methods=["POST"])
def update():
    form = flask.request.form.to_dict()
    
    room = form["room"]
    node = form["node"]
    
    form.pop("room")
    form.pop("node")
    
    if room not in data["rooms"]:
        data["rooms"][room] = {
            "temperature": "Unknown",
            "humidity": "Unknown",
            "lightness": "Unknown",
            "motion": "Unknown",
            "nodes": []
        }
    
    
    for param in form.keys():
        data["rooms"][room][param] = form.get(param)
    
    if node not in data["rooms"][room]["nodes"]:
        data["rooms"][room]["nodes"].append(node)
    data["rooms"][room]["nodes"].sort()
    
    data["rooms"][room]["last_update"] = time.time()
    
    return "OK"

@app.route("/")
def dashboard():
    return flask.render_template("dashboard.html")

@app.route("/json")
def home():
    return data

app.run("espsmarthome.hub", 8080)