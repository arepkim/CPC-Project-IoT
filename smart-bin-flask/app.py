from flask import Flask, render_template, jsonify, request
import random
import time
import json
import paho.mqtt.client as mqtt
from datetime import datetime

app = Flask(__name__)

# --- IN-MEMORY DATABASE ---
current_data = {
    "bin_id": "BIN-001",
    "level": 0,
    "humidity": 0,
    "temperature": 25,
    "status": "Normal",
    "lid_status": "Closed",
    "person_nearby": False,
    "env_alert": "Good",
    "last_updated": datetime.now().strftime("%H:%M:%S")
}

# --- MQTT SETTINGS (HiveMQ Cloud) ---
MQTT_BROKER = "091bd933360c43dba44b83699e8f12a3.s1.eu.hivemq.cloud"
MQTT_PORT = 8883
MQTT_USER = "ardyqawi"
MQTT_PW = "Ardy12345"
MQTT_TOPIC = "student/dustbin/data"

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("✅ Connected to HiveMQ Cloud!")
        client.subscribe(MQTT_TOPIC)
    else:
        print(f"❌ Failed to connect, return code {rc}")

def on_message(client, userdata, msg):
    global current_data
    try:
        # Decode the byte payload to string then to JSON
        payload = msg.payload.decode()
        data = json.loads(payload)
        print(f"📩 MQTT Message received: {payload}")

        # Map MQTT keys to our internal current_data keys
        if 'level' in data: current_data["level"] = data['level']
        if 'hum' in data: current_data["humidity"] = data['hum']
        if 'temp' in data: current_data["temperature"] = data['temp']
        if 'status' in data: current_data["status"] = data['status']
        if 'lid_status' in data: current_data["lid_status"] = data['lid_status']
        if 'env_alert' in data: current_data["env_alert"] = data['env_alert']
        
        # If lid opens via MQTT event, implies person is nearby
        if 'lid_status' in data and data['lid_status'] == "Open":
             current_data["person_nearby"] = True
        elif 'lid_status' in data and data['lid_status'] == "Closed":
             current_data["person_nearby"] = False

        current_data["last_updated"] = datetime.now().strftime("%H:%M:%S")
        
    except Exception as e:
        print(f"❌ Error processing MQTT message: {e}")

# Initialize MQTT Client with SSL
mqtt_client = mqtt.Client(client_id="Flask_Server_Bridge", transport="tcp")
mqtt_client.username_pw_set(MQTT_USER, MQTT_PW)
mqtt_client.tls_set() # Enable SSL/TLS for HiveMQ Cloud Port 8883
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message

# Start MQTT in a non-blocking background thread
try:
    mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)
    mqtt_client.loop_start()
except Exception as e:
    print(f"⚠️ Could not start MQTT client: {e}")

@app.route('/')
def dashboard():
    """Serves the main HTML page."""
    return render_template('index.html')

@app.route('/api/data')
def get_sensor_data():
    """
    API Endpoint that the Frontend calls every 2 seconds.
    It returns the latest stored sensor status (updated via MQTT).
    """
    return jsonify(current_data)

@app.route('/api/update-sensor', methods=['POST'])
def update_sensor_data():
    """
    Keep this for compatibility or direct HTTP testing.
    """
    global current_data
    try:
        data = request.get_json()
        if not data:
            return jsonify({"status": "error", "message": "No data received"}), 400
        
        if 'level' in data: current_data["level"] = data['level']
        if 'humidity' in data: current_data["humidity"] = data['humidity']
        if 'temp' in data: current_data["temperature"] = data['temp']
        if 'status' in data: current_data["status"] = data['status']
        if 'lid_status' in data: current_data["lid_status"] = data['lid_status']
        if 'person_nearby' in data: current_data["person_nearby"] = data['person_nearby']
            
        current_data["last_updated"] = datetime.now().strftime("%H:%M:%S")
        return jsonify({"status": "success"}), 200
    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0', port=5000)

#python app.py