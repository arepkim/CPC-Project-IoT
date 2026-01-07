/*
 * Project: Smart Dustbin (Website + HiveMQ Version)
 * Base: Old Code (HTTP POST) - Keeps Website working
 * Added: MQTT (HiveMQ) - Adds mobile app/dashboard support
 * UPDATE: Added Lid Status Output (Serial + MQTT)
 */

#include <WiFi.h>
#include <HTTPClient.h>       // REQUIRED for Website (GCP VM)
#include <WiFiClientSecure.h> // REQUIRED for HiveMQ (MQTT)
#include <PubSubClient.h>     // REQUIRED for MQTT
#include <ESP32Servo.h>
#include <DHT.h>

// ==========================================
// --- 1. WIFI SETTINGS ---
// ==========================================
const char* ssid = "Ini group jantan";
const char* password = "kill0000";

// ==========================================
// --- 2. WEBSITE (GCP VM) SETTINGS ---
// ==========================================
// IMPORTANT: Replace this with your VM Public IP Address
const char* serverUrl = "http://x.x.x.x:5000/api/update-sensor";

// ==========================================
// --- 3. HIVEMQ (MQTT) SETTINGS ---
// ==========================================
const char* mqtt_server = "091bd933360c43dba44b83699e8f12a3.s1.eu.hivemq.cloud";
const int mqtt_port = 8883; 
const char* mqtt_username = "ardyqawi"; 
const char* mqtt_password = "Ardy12345"; 
const char* mqtt_topic = "student/dustbin/data";

// ==========================================
// --- 4. PIN DEFINITIONS & OBJECTS ---
// ==========================================
#define PIN_SERVO   47
#define PIN_DHT     38
#define PIN_IR      9
#define PIN_TRIG    5
#define PIN_ECHO    6

#define DHTTYPE DHT11
DHT dht(PIN_DHT, DHTTYPE);
Servo myServo;

// Network Clients
WiFiClientSecure espClient;     // Secure client for MQTT
PubSubClient client(espClient); // MQTT Client object

// ==========================================
// --- 5. VARIABLES ---
// ==========================================
const int SERVO_OPEN = 90;
const int SERVO_CLOSE = 0;
const float BIN_HEIGHT_CM = 30.0;
float currentBinLevel = 0; 

// Strings for Status
String binStatus = "Normal";
String envStatus = "Good";
String lidStatus = "Closed"; // NEW: Tracks if lid is Open/Closed

void setup() {
  Serial.begin(115200);

  // Hardware Init
  dht.begin();
  myServo.setPeriodHertz(50); 
  myServo.attach(PIN_SERVO, 500, 2400);
  myServo.write(SERVO_CLOSE);
  pinMode(PIN_IR, INPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  // WiFi Connect
  setup_wifi();

  // --- MQTT SETUP ---
  espClient.setInsecure(); // Required for HiveMQ SSL
  client.setServer(mqtt_server, mqtt_port);
}

void setup_wifi() {
  delay(10);
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
}

void reconnect() {
  if (WiFi.status() == WL_CONNECTED && !client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32-Dustbin-" + String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" (skipping for now)"); 
    }
  }
}

float readUltrasonic() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  long duration = pulseIn(PIN_ECHO, HIGH, 30000);
  if (duration == 0) return 0.0; 
  return duration * 0.034 / 2;
}

void loop() {
  // 1. Maintain MQTT Connection
  if (!client.connected()) {
    reconnect();
  }
  client.loop(); 

  // 2. Main Logic 
  if (WiFi.status() == WL_CONNECTED) {
    static unsigned long lastRead = 0;
    
    // Timer: Every 2 seconds (Standard Reporting)
    if (millis() - lastRead > 2000) { 
      
      // --- READ SENSORS ---
      float t = dht.readTemperature();
      float h = dht.readHumidity();
      float dist = readUltrasonic();
      bool personNearby = (digitalRead(PIN_IR) == LOW);

      if (dist > 0) {
         float fillHeight = BIN_HEIGHT_CM - dist;
         currentBinLevel = (fillHeight / BIN_HEIGHT_CM) * 100.0;
         if (currentBinLevel < 0) currentBinLevel = 0;
         if (currentBinLevel > 100) currentBinLevel = 100;
      }

      // --- LOGIC FOR STATUS ---
      if (t > 32.0 && h > 70.0) envStatus = "WARNING: Smell Risk";
      else if (t > 40.0) envStatus = "ALERT: High Heat";
      else envStatus = "Good";

      if (currentBinLevel >= 90) binStatus = "FULL";
      else binStatus = "Normal";

      // ---------------------------------------------------------
      // PART A: SEND TO WEBSITE (HTTP)
      // ---------------------------------------------------------
      HTTPClient http;
      http.begin(serverUrl);
      http.addHeader("Content-Type", "application/json");

      String httpPayload = "{";
      httpPayload += "\"level\": " + String(currentBinLevel) + ",";
      httpPayload += "\"humidity\": " + String(isnan(h) ? 0 : h) + ",";
      httpPayload += "\"temp\": " + String(isnan(t) ? 0 : t) + ",";
      httpPayload += "\"lid_status\": \"" + lidStatus + "\","; 
      httpPayload += "\"person_nearby\": " + String(personNearby ? "true" : "false");
      httpPayload += "}";

      // Serial.print("Sending to Website... ");
      int httpCode = http.POST(httpPayload);
      // Serial.println(httpCode); 
      http.end();

      // ---------------------------------------------------------
      // PART B: SEND TO HIVEMQ (MQTT)
      // ---------------------------------------------------------
      if (!isnan(t) && !isnan(h)) {
        String mqttPayload = "{";
        mqttPayload += "\"temp\": " + String(t) + ",";
        mqttPayload += "\"hum\": " + String(h) + ",";
        mqttPayload += "\"level\": " + String(currentBinLevel) + ",";
        mqttPayload += "\"status\": \"" + binStatus + "\",";
        mqttPayload += "\"lid_status\": \"" + lidStatus + "\","; 
        mqttPayload += "\"env_alert\": \"" + envStatus + "\"";
        mqttPayload += "}";
        
        client.publish(mqtt_topic, mqttPayload.c_str());
        Serial.println("Data sent to HiveMQ");
      }

      lastRead = millis();
    }
  } else {
    setup_wifi();
  }

  // --- AUTOMATION (Lid Logic + Immediate Reporting) ---
  if (digitalRead(PIN_IR) == LOW) {
    if (currentBinLevel < 90.0) {
      
      // 1. UPDATE STATUS & PRINT
      lidStatus = "Open";
      Serial.println(">>> ACTION: LID OPENING! <<<");

      // 2. SEND IMMEDIATE MQTT UPDATE (So dashboard sees 'Open' instantly)
      String eventPayload = "{\"lid_status\": \"Open\", \"status\": \"Lid Open\"}";
      client.publish(mqtt_topic, eventPayload.c_str());

      // 3. MOVE SERVO
      myServo.write(SERVO_OPEN);
      
      // 4. WAIT
      delay(3000); // Code pauses here for 3 seconds
      
      // 5. CLOSE SERVO
      myServo.write(SERVO_CLOSE);
      lidStatus = "Closed";
      Serial.println(">>> ACTION: LID CLOSED <<<");

      // 6. SEND IMMEDIATE MQTT UPDATE (So dashboard resets)
      eventPayload = "{\"lid_status\": \"Closed\", \"status\": \"Normal\"}";
      client.publish(mqtt_topic, eventPayload.c_str());
    } 
  }
}
