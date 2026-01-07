# 🔄 Operational Flow: How to Run the Smart Bin System

Once you have completed the deployment (Phase 1-4) and uploaded the code to your Arduino (Phase 5), follow this guide to understand how the system operates in a "Live" environment.

---

## 🚀 The "Set and Forget" Concept
In a professional IoT system, you do **not** need to keep your laptop connected to the hardware or keep any terminal windows open. Each part of the system works independently.

### 1. The Cloud Server (The Brain) 🧠
*   **Where it lives:** Google Cloud Platform (GCP) Virtual Machine.
*   **How it runs:** By using the `nohup gunicorn` command, the Flask app runs in the background.
*   **Laptop Status:** You can **close your laptop** or turn it off. The server stays "alive" in the cloud 24/7.
*   **Maintenance:** You only need to log back into the VM (via SSH) if you want to update the website code or check the `server.log`.

### 2. The Hardware (The Bin) 🗑️
*   **Where it lives:** Inside your physical dustbin (Maker Feather S3 / ESP32).
*   **How it runs:** Once you have clicked **Upload** in the Arduino IDE, the code is permanently stored on the chip.
*   **Laptop Status:** You can **unplug the USB cable** from your laptop.
*   **Power Source:** Plug the ESP32 into a **Power Bank** or a **USB Wall Adapter** (like a phone charger).
*   **Networking:** As long as it is within range of the WiFi (the SSID/Password you programmed), it will automatically start sending data to your VM.

### 3. The Dashboard (The View) 📱
*   **Where it lives:** Any web browser on any device (Phone, Tablet, Laptop).
*   **How to access:** Open your browser and type: `http://<YOUR_VM_EXTERNAL_IP>:5000`
*   **Real-time updates:** The website will fetch data from the VM every 2 seconds, showing you the bin's status from anywhere in the world.

---

## 📊 Quick Comparison

| Component | Needs Laptop? | Needs Power? | Needs Internet? |
| :--- | :--- | :--- | :--- |
| **GCP VM** | ❌ No | ✅ (Cloud Data Center) | ✅ Yes |
| **ESP32 Bin** | ❌ No | ✅ (Power Bank/Wall) | ✅ Yes (WiFi) |
| **Website** | ❌ No | ✅ (Device Battery) | ✅ Yes |

---

## 📋 Demo Day Checklist
If you are presenting this project, follow these steps for a smooth demonstration:

1.  **Server Check:** Open the dashboard link on your phone/laptop to ensure the website is loading.
2.  **Power On:** Plug the Bin's ESP32 into a power bank.
3.  **Wait for WiFi:** Wait about 10–15 seconds for the "WiFi Connected" light/status.
4.  **Test the Lid:** Wave your hand in front of the IR sensor. The lid should open.
5.  **Verify Data:** Look at the dashboard. You should see the "Last Updated" time change and the "Person Nearby" status switch to **YES**.
6.  **Done!** You have successfully demonstrated a cloud-connected IoT system.
