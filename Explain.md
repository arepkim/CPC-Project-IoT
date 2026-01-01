Here is exactly what each person needs to do to "meet in the middle":

  1. The Setup (The Meeting Point)
  You (Web Dev) will rent a GCP VM.
   * This VM will have a public IP address (e.g., 123.45.67.89).
   * You will put your Flask Python script on this VM.
   * This script acts as the "receptionist." It accepts data and gives data.

  2. Your Teammate (The Hardware Side)
  Your teammate writes code in Arduino IDE.
   * Current State: The code reads a sensor and prints Distance: 20cm to the Serial Monitor (the black screen on their laptop).
   * What they add: They add 5-10 lines of code that says: "Send this `20cm` to `http://123.45.67.89/api/update-sensor`."
   * Result: The hardware now talks to your VM instead of just their laptop screen.

  3. You (The Web Side)
  You write code in React/Flask.
   * Current State: Your dashboard shows random fake numbers.
   * What you change: You change your code to say: "Go to `http://123.45.67.89/api/data` and get the number."
   * Result: Your website shows the 20cm that your teammate sent.

  ---

  Summary of Actions
   1. You: Set up the Flask app on the VM (I was about to do this in the code).
   2. Teammate: Adds an "HTTP POST" command to their Arduino code.
   3. You: Point your Flask frontend to the VM's IP address.

   -----------------------------------------------------------------------------------------------------------------------------------------------
   Here are the steps to set up the Google Cloud Platform (GCP) Compute Engine VM and the recommended specifications for your IoT project.

  Step 1: Create the VM Instance

   1. Log in to the Google Cloud Console (https://console.cloud.google.com/).
   2. Navigate to Compute Engine > VM instances.
   3. Click "CREATE INSTANCE".

  Step 2: Configure Specifications

  Since this is a student project with light traffic (one hardware device, a few web users), you want the cheapest option that works reliably.

   * Name: smart-bin-server (or anything you like).
   * Region: Select one close to you (e.g., asia-southeast1 for Singapore/Malaysia) to minimize latency.
   * Machine Configuration:
       * Series: E2
       * Machine type: e2-micro (2 vCPU, 1 GB memory).
       * Note: This is often part of the "Free Tier" (if in specific US regions) or very cheap (~$7/month). It is powerful enough for Flask + SQLite.
   * Boot Disk:
       * OS: Ubuntu
       * Version: Ubuntu 22.04 LTS (x86/64).
       * Size: 10 GB (Standard persistent disk).
   * Firewall:
       * Check [x] Allow HTTP traffic.
       * Check [x] Allow HTTPS traffic.

   4. Click "CREATE".

  Step 3: Open the Firewall Port (Critical)

  By default, GCP only allows traffic on port 80 (HTTP). Your Flask app usually runs on port 5000. You must open this port.

   1. Go to VPC network > Firewall.
   2. Click "CREATE FIREWALL RULE".
   3. Name: allow-flask-5000.
   4. Targets: "All instances in the network".
   5. Source IPv4 ranges: 0.0.0.0/0 (Allows connection from anywhere—your hardware and your web app).
   6. Protocols and ports: Check TCP and type 5000.
   7. Click "CREATE".

  Step 4: Connect & Install Python

   1. Go back to VM instances.
   2. Click the "SSH" button next to your new VM. A black terminal window will pop up.
   3. Run these commands to set up the environment:

   1     # Update the system
   2     sudo apt update
   3
   4     # Install Python and Pip
   5     sudo apt install python3 python3-pip git -y
   6
   7     # (Optional) Create a folder for your project
   8     mkdir cpc-project

   9     cd cpc-project
