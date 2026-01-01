from flask import Flask, render_template, jsonify, request
import random
import time
from datetime import datetime

app = Flask(__name__)

# --- IN-MEMORY DATABASE ---
# This dictionary stores the latest data received from the hardware.
current_data = {
    "bin_id": "BIN-001",
    "level": 0,
    "humidity": 0,
    "temperature": 25,
    "status": "Normal",
    "person_nearby": False,
    "last_updated": datetime.now().strftime("%H:%M:%S")
}

@app.route('/')
def dashboard():
    """Serves the main HTML page."""
    return render_template('index.html')

@app.route('/api/data')
def get_sensor_data():
    """
    API Endpoint that the Frontend calls every 2 seconds.
    It returns the latest stored sensor status.
    """
    return jsonify(current_data)

@app.route('/api/update-sensor', methods=['POST'])
def update_sensor_data():
    """
    Endpoint for the IoT Hardware (Arduino) to push data to.
    Expected JSON: {"level": 50, "humidity": 55, "temp": 30, "status": "Normal", "person_nearby": true}
    """
    global current_data
    try:
        data = request.get_json()
        if not data:
            return jsonify({"status": "error", "message": "No data received"}), 400
        
        # Update internal memory with hardware data
        if 'level' in data: current_data["level"] = data['level']
        if 'humidity' in data: current_data["humidity"] = data['humidity']
        if 'temp' in data: current_data["temperature"] = data['temp']
        if 'status' in data: current_data["status"] = data['status']
        if 'person_nearby' in data: current_data["person_nearby"] = data['person_nearby']
            
        current_data["last_updated"] = datetime.now().strftime("%H:%M:%S")
        
        print(f"✅ Data received from Hardware: {current_data}")
        return jsonify({"status": "success"}), 200

    except Exception as e:
        print(f"❌ Error updating data: {e}")
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route('/api/simulate-person', methods=['POST'])
def simulate_person():
    """Endpoint to manually toggle the 'Person Nearby' status for testing."""
    current_data["person_nearby"] = not current_data["person_nearby"]
    return jsonify({"status": "success", "new_state": current_data["person_nearby"]})

if __name__ == '__main__':
    # host='0.0.0.0' makes the server accessible via the VM's Public IP
    app.run(debug=True, host='0.0.0.0', port=5000)

#python app.py