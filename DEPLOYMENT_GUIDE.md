# 🚀 Smart Dustbin IoT System - GCP Deployment Guide

This guide details the steps to deploy the **Smart Bin Flask Backend** to a **Google Cloud Platform (GCP) Virtual Machine**.

---

## **Phase 1: Create the Cloud Infrastructure**

### **1. Create a VM Instance**
1. Log in to the [Google Cloud Console](https://console.cloud.google.com/).
2. Navigate to **Compute Engine** > **VM instances**.
3. Click **Create Instance**.
4. **Name:** `smart-bin-server` (or your choice).
5. **Region:** Choose one close to you.
6. **Machine Type:** `e2-micro` (Eligible for free tier) or `e2-medium`.
7. **Boot Disk:** Ubuntu 20.04 LTS or 22.04 LTS (Recommended over Debian for this guide).
8. **Firewall:** Check both boxes:
   - [x] Allow HTTP traffic
   - [x] Allow HTTPS traffic
9. Click **Create**.

### **2. Configure the Firewall (Crucial Step)**
By default, GCP blocks the custom port `5000` used by Flask. We must open it.

1. Go to **VPC Network** > **Firewall**.
2. Click **Create Firewall Rule**.
3. **Name:** `allow-flask-5000`
4. **Targets:** "All instances in the network".
5. **Source IPv4 ranges:** `0.0.0.0/0` (Allows access from anywhere).
6. **Protocols and ports:**
   - Select **Specified protocols and ports**.
   - Check **TCP**.
   - Enter `5000` in the box.
7. Click **Create**.

---

## **Phase 2: Server Setup (Inside the VM)**

### **1. Connect via SSH**
1. Go back to **Compute Engine** > **VM instances**.
2. Find your new instance.
3. Click the **SSH** button. A browser-based terminal window will open.

### **2. Update System & Install Python**
Copy and paste these commands into the SSH terminal:

```bash
# Update the list of available packages
sudo apt update

# Upgrade existing packages (Type 'Y' if prompted)
sudo apt upgrade -y

# Install Python 3, Pip (package manager), and Virtual Environment tools
sudo apt install -y python3-pip python3-venv git
```

---

## **Phase 3: Deploy the Code**

### **1. Upload Your Project**
**Option A: Upload Feature (Simplest)**
1. In the SSH window top-right menu (gear icon), click **Upload file**.
2. Select your `smart-bin-flask` folder (Tip: Zip it on your PC first as `smart-bin-flask.zip` for faster upload).
3. If you uploaded a zip, unzip it:
   ```bash
   sudo apt install unzip
   unzip smart-bin-flask.zip
   ```

**Option B: Clone from GitHub (Best practice)**
If you have pushed your code to GitHub:
```bash
git clone https://github.com/YOUR_USERNAME/YOUR_REPO_NAME.git
```

### **2. Setup Python Environment**
Navigate into your project folder and install dependencies.

```bash
# 1. Enter the directory (Adjust name if yours is different)
cd smart-bin-flask

# 2. Create a virtual environment named 'venv'
python3 -m venv venv

# 3. Activate the virtual environment
source venv/bin/activate
# (You should see (venv) appear at the start of your command line)

# 4. Install project dependencies
pip install -r requirements.txt

# 5. Install Gunicorn (Production Web Server)
pip install gunicorn
```

---

## **Phase 4: Run the Application**

### **1. Test Run (Temporary)**
Run this command to check if everything works:
```bash
python3 app.py
```
*   You should see: `Running on http://0.0.0.0:5000`.
*   **Test it:** Open your computer's browser and go to http://<EXTERNAL IP>:5000, e.g.`http://34.172.131.196:5000`.
*   Press **Ctrl + C** in the SSH window to stop the server.

### **2. Production Run (Background Service)**
To keep the server running even after you close the SSH window, use `nohup` or `screen`.

**Method 1: Using Nohup (Quick)**
```bash
# Run Gunicorn in the background
nohup gunicorn --bind 0.0.0.0:5000 app:app > server.log 2>&1 &
```
*   Your server is now live 24/7!

---

## **Phase 5: Connect the Hardware**

1. Go to your **Compute Engine** dashboard and copy the **External IP** of your VM.
2. Open your Arduino code (`Smartdustbin_arduino_code.ino`).
3. Find this line:
   ```cpp
   const char* serverUrl = "http://REPLACE_WITH_VM_IP:5000/api/update-sensor";
   ```
4. Replace `REPLACE_WITH_VM_IP` with your actual External IP.
   *   *Example:* `http://34.123.45.67:5000/api/update-sensor`
5. Upload the code to your ESP32 / Maker Feather S3.

### **Done!** 🎉
Your Smart Dustbin is now sending data to the Cloud, and you can view it from anywhere in the world.
