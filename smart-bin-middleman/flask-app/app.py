from flask import Flask, render_template, jsonify, request
import json
import paho.mqtt.client as mqtt
from datetime import datetime
import firebase_admin
from firebase_admin import credentials, db  # Import Firebase modules

app = Flask(__name__)

# --- FIREBASE SETUP ---
# 1. Load the credentials from the JSON file you downloaded
cred = credentials.Certificate("serviceAccountKey.json")

# 2. Initialize the app with your Database URL
# IMPORTANT: Replace the URL below with YOUR Firebase Realtime Database URL
firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://smart-bin-f0564-default-rtdb.firebaseio.com'
})

# --- IN-MEMORY DATABASE (For Live View) ---
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
        # 1. Decode MQTT Data
        payload = msg.payload.decode()
        data = json.loads(payload)
        print(f"📩 MQTT Message received: {payload}")

        # 2. Update Local Data (For Live Dashboard)
        if 'level' in data: current_data["level"] = data['level']
        if 'hum' in data: current_data["humidity"] = data['hum']
        if 'temp' in data: current_data["temperature"] = data['temp']
        if 'status' in data: current_data["status"] = data['status']
        if 'lid_status' in data: current_data["lid_status"] = data['lid_status']
        if 'env_alert' in data: current_data["env_alert"] = data['env_alert']
        
        if 'lid_status' in data and data['lid_status'] == "Open":
             current_data["person_nearby"] = True
        elif 'lid_status' in data and data['lid_status'] == "Closed":
             current_data["person_nearby"] = False

        # Add a timestamp
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        current_data["last_updated"] = timestamp

        # --- 3. SEND TO FIREBASE (New Part) ---
        # We create a new entry in the 'history' node
        # This keeps a record of EVERY reading
        
        firebase_data = current_data.copy() # Copy data so we don't mess up the live one
        firebase_data['timestamp'] = timestamp # Ensure timestamp is saved
        
        # Push to Firebase (creates a unique ID like -NkT7...)
        ref = db.reference('sensor_history')
        ref.push(firebase_data)
        
        print("🔥 Data saved to Firebase History!")

    except Exception as e:
        print(f"❌ Error processing: {e}")

# Initialize MQTT Client with SSL
mqtt_client = mqtt.Client(client_id="Flask_Server_Bridge", transport="tcp")
mqtt_client.username_pw_set(MQTT_USER, MQTT_PW)
mqtt_client.tls_set()
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message

try:
    mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)
    mqtt_client.loop_start()
except Exception as e:
    print(f"⚠️ Could not start MQTT client: {e}")

@app.route('/')
def dashboard():
    return render_template('index.html')

@app.route('/api/data')
def get_sensor_data():
    return jsonify(current_data)

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0', port=5000)