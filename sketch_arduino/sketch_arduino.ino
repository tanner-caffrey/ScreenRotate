#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// WiFi Credentials
const char* ssid = "x";
const char* password = "x";

// Web Server running on port 80
ESP8266WebServer server(80);
Adafruit_MPU6050 mpu;

void setup() {
    Serial.begin(115200);
    
    // Connect to WiFi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi");
    Serial.print("Device IP: ");
    Serial.println(WiFi.localIP()); // Should show 192.168.1.28

    // Initialize MPU6050
    Wire.begin();
    if (!mpu.begin()) {
        Serial.println("Failed to initialize MPU6050!");
        while (1);
    }

    // Set MPU6050 configurations
    mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
    mpu.setGyroRange(MPU6050_RANGE_250_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

    // Serve the HTML page at "/"
    server.on("/", handleRoot);

    // Serve JSON data at "/data"
    server.on("/data", handleData);

    // Start the server
    server.begin();
    Serial.println("Web server started!");
}

void loop() {
    server.handleClient(); // Handle incoming HTTP requests
}

// Function to get MPU6050 sensor values
void getSensorData(float &pitch, float &roll) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    pitch = atan2(a.acceleration.y, a.acceleration.z) * 180 / PI;
    roll = atan2(-a.acceleration.x, sqrt(a.acceleration.y * a.acceleration.y + a.acceleration.z * a.acceleration.z)) * 180 / PI;
}

// Serve live HTML page
void handleRoot() {
    String html = R"rawliteral(
        <!DOCTYPE html>
        <html>
        <head>
            <title>Mongyro Sensor Data</title>
            <style>
                body { font-family: Arial, sans-serif; text-align: center; }
                h1 { color: #333; }
                .data { font-size: 24px; }
            </style>
            <script>
                function updateData() {
                    fetch('/data')
                        .then(response => response.json())
                        .then(data => {
                            document.getElementById('pitch').innerText = data.pitch.toFixed(2);
                            document.getElementById('roll').innerText = data.roll.toFixed(2);
                        });
                }
                setInterval(updateData, 100);
                window.onload = updateData;
            </script>
        </head>
        <body>
            <h1>Mongyro Sensor Data</h1>
            <p class="data">Pitch: <span id="pitch">0</span>°</p>
            <p class="data">Roll: <span id="roll">0</span>°</p>
        </body>
        </html>
    )rawliteral";

    server.send(200, "text/html", html);
}

// Serve JSON data at "/data"
void handleData() {
    float pitch, roll;
    getSensorData(pitch, roll);

    String json = "{";
    json += "\"pitch\": " + String(pitch) + ",";
    json += "\"roll\": " + String(roll);
    json += "}";

    server.send(200, "application/json", json);
}
