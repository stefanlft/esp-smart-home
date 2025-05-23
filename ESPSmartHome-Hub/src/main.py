# prototyping

import flask
import time

app = flask.Flask(__name__)

MAX_HISTORY = 100

data = {
    "rooms": {}
}

@app.route("/update", methods=["POST"])
def update():
    form = flask.request.form.to_dict()
    
    flag = False
    received = int(time.time())
    
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
            "nodes": [],
            "history": {
                "temperature": [],
                "humidity": [],
                "lightness": [],
                "motion": []
            }
        }
    
    
    for param in form.keys():
        data["rooms"][room][param] = form.get(param)
        flag = True
        
        history = data["rooms"][room]["history"]
        history[param].append({"value": form.get(param), "time": received})
        
        if len(history) > MAX_HISTORY:
            history.pop(0)
    
    if node not in data["rooms"][room]["nodes"]:
        data["rooms"][room]["nodes"].append(node)
    data["rooms"][room]["nodes"].sort()
    
    if flag:
        data["rooms"][room]["last_update"] = received
    
    
    return "OK"

@app.route("/")
def dashboard():
    return flask.render_template("dashboard.html")

@app.route("/json")
def home():
    cleaned_data = {
    "rooms": {
            room_name: {k: v for k, v in room_data.items() if k != "history"}
            for room_name, room_data in data["rooms"].items()
        }
    }

    return cleaned_data

@app.route('/history/<room>', methods=['GET'])
def get_history(room):
    room_data = data['rooms'].get(room)
    if room_data and 'history' in room_data:
        return flask.jsonify(room_data['history'])
    else:
        return 404, "Room or history not found"


app.run("0.0.0.0", 8080)