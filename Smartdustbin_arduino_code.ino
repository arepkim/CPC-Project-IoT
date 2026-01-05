/*
 * Project: Smart Dustbin (GCP VM HTTP Version)
 * Hardware: Maker Feather S3 (ESP32)
 * Protocol: HTTP POST
 */

#include <WiFi.h>
#include <HTTPClient.h> // REQUIRED for GCP VM
#include <ESP32Servo.h>
#include <DHT.h>

// --- WIFI SETTINGS ---
const char* ssid = "iPhone";
const char* password = "tenagamuda";

// --- GCP VM SETTINGS ---
// IMPORTANT: Replace this with your VM Public IP Address
const char* serverUrl = "http://x.x.x.x:5000/api/update-sensor";

// --- PIN DEFINITIONS ---
#define PIN_SERVO   47
#define PIN_DHT     38
#define PIN_IR      9
#define PIN_TRIG    5
#define PIN_ECHO    6

#define DHTTYPE DHT11
DHT dht(PIN_DHT, DHTTYPE);
Servo myServo;

// --- VARIABLES ---
const int SERVO_OPEN = 90;
const int SERVO_CLOSE = 0;
const float BIN_HEIGHT_CM = 30.0; 

float currentBinLevel = 0; 

void setup() {
  Serial.begin(115200);

  // 1. Hardware Init
  dht.begin();
  myServo.setPeriodHertz(50); 
  myServo.attach(PIN_SERVO, 500, 2400);
  myServo.write(SERVO_CLOSE); 
  pinMode(PIN_IR, INPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  // 2. WiFi Connect
  setup_wifi();
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
  if (WiFi.status() == WL_CONNECTED) {
    // --- DATA COLLECTION ---
    static unsigned long lastRead = 0;
    if (millis() - lastRead > 2000) { // Send data every 2 seconds
      float t = dht.readTemperature();
      float h = dht.readHumidity();
      float dist = readUltrasonic();
      bool personNearby = (digitalRead(PIN_IR) == LOW); // Detects person/hand

      if (dist > 0) {
         float fillHeight = BIN_HEIGHT_CM - dist;
         currentBinLevel = (fillHeight / BIN_HEIGHT_CM) * 100.0;
         if (currentBinLevel < 0) currentBinLevel = 0;
         if (currentBinLevel > 100) currentBinLevel = 100;
      }

      // --- HTTP POST TO VM ---
      HTTPClient http;
      http.begin(serverUrl);
      http.addHeader("Content-Type", "application/json");

      // Construct JSON payload to match Flask expectations
      String payload = "{";
      payload += "\"level\": " + String(currentBinLevel) + ",";
      payload += "\"humidity\": " + String(isnan(h) ? 0 : h) + ",";
      payload += "\"temp\": " + String(isnan(t) ? 0 : t) + ",";
      payload += "\"person_nearby\": " + String(personNearby ? "true" : "false");
      payload += "}";

      Serial.print("Sending to VM: ");
      Serial.println(payload);

      int httpResponseCode = http.POST(payload);
      
      if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
      } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      http.end();
      
      lastRead = millis();
    }
  } else {
    setup_wifi(); // Reconnect if WiFi is lost
  }

  // --- AUTOMATION (Lid Logic) ---
  if (digitalRead(PIN_IR) == LOW) {
    if (currentBinLevel < 90.0) {
      myServo.write(SERVO_OPEN);
      delay(3000); // Keep open for 3 seconds
      myServo.write(SERVO_CLOSE);
    } 
  }
}
