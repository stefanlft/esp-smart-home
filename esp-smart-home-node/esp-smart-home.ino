#include <DHT11.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <WebServer.h>
#include <esp_sleep.h>

#define PIR_PIN (16)
#define DHT_PIN (17)
#define LDR_PIN (34)
#define LED_PIN (23)
#define BUZ_PIN (18)

#define SLEEP_TIMER (60)

#define BUZZER_CHANNEL (0)

DHT11 dht11(17);
HTTPClient http;
int pirPrevState = LOW;

Preferences preferences;
bool configured = true;

WebServer server(80);


// Replace with your network credentials
String ssid = "SSID";
String psk = "PSK";
String serverURL = "http://192.168.1.136:8080";
String room = "living";
bool loud = false;
String node = "alpha";

unsigned blinkCount = 0;
long lastBlink = 0;

// Values
bool motion = false;
int temperature = 0;
int humidity = 0;
double lightness = 0;

void initWiFiSTA() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, psk);
  Serial.println(ssid + ":" + psk);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(String("\nIP: ")+WiFi.localIP());
}

void initWiFiAP() {
  WiFi.mode(WIFI_AP);  // Set WiFi mode to AP (Access Point)
  WiFi.softAPConfig(IPAddress(192, 168, 1, 1), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));
  WiFi.softAP("ESP Smart Home Node", "password123");  // SSID and Password for the AP
  Serial.print("Access Point Started. IP: ");
  Serial.println(WiFi.softAPIP());  // Get and print the IP address of the Access Point
}

void initPreferences() {
  preferences.begin("esp-smart-home", true);

  configured = preferences.getBool("configured", false);

  room = preferences.getString("room", "");
  ssid = preferences.getString("ssid", "");
  psk = preferences.getString("psk", "");
  serverURL = preferences.getString("hub", "");
  loud = preferences.getBool("loud", false);
  node = preferences.getString("node", "alpha");

  preferences.end();
}

void handleRoot() {
  // Basic HTML form
  String html = R"rawliteral(
    <html>
      <head>
        <title>ESP Smart Home Node Config</title>
        <style>
          body {
            font-family: Arial, sans-serif;
            background-color: #f5f5f5;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
          }
          .container {
            background-color: white;
            padding: 30px 40px;
            border-radius: 10px;
            box-shadow: 0 4px 8px rgba(0,0,0,0.1);
            max-width: 400px;
            width: 100%;
          }
          h2 {
            text-align: center;
            color: #333;
          }
          label {
            display: block;
            margin-bottom: 8px;
            font-weight: bold;
          }
          input[type="text"], input[type="password"] {
            width: 100%;
            padding: 10px;
            margin-bottom: 15px;
            border: 1px solid #ccc;
            border-radius: 5px;
          }
          input[type="submit"] {
            width: 100%;
            padding: 12px;
            background-color: #4CAF50;
            color: white;
            border: none;
            border-radius: 5px;
            font-size: 16px;
            cursor: pointer;
          }
          input[type="submit"]:hover {
            background-color: #45a049;
          }
          .checkbox-inline {
            display: inline-block;
            margin-left: 5px;
            font-weight: normal;
          }
          input[type="checkbox"] {
            margin-right: 10px; /* Space between checkbox and label */
          }
        </style>
        <script>
          function confirmSubmission() {
            return confirm("Are you sure all your data is correct before submitting?");
          }
        </script>
      </head>
      <body>
        <div class="container">
          <h2>Configure Node</h2>
          <form action="/submit" method="POST" onsubmit="return confirmSubmission();">
            <label for="ssid">SSID</label>
            <input type="text" id="ssid" name="ssid" required>

            <label for="psk">Password</label>
            <input type="text" id="psk" name="psk" required>

            <label for="room">Room</label>
            <input type="text" id="room" name="room" required>

            <label for="node">Node</label>
            <input type="text" id="node" name="node" required>

            <label for="hub">Hub Address</label>
            <input type="text" id="hub" name="hub" required>

            <label for="loud" class="checkbox-inline">
              <input type="checkbox" id="loud" name="loud" checked /> Use buzzer
            </label>

            <input type="submit" value="Submit">
          </form>
        </div>
      </body>
    </html>
  )rawliteral";

  
  server.send(200, "text/html", html);
}

void handleFormSubmit() {
  String rssid = server.arg("ssid");
  String rpsk = server.arg("psk");

  String rroom = server.arg("room");
  String rhub = server.arg("hub");
  String rnode = server.arg("node");

  bool rbuzz = server.hasArg("loud");

  Serial.println("Form Submitted");
  Serial.println("SSID: " + rssid);
  Serial.println("PSK: " + rpsk);
  Serial.println("ROOM: " + rroom);
  Serial.println("HUB: " + rhub);
  Serial.println("LOUD: " + rbuzz ? "true" : "false");
  Serial.println("NODE: " + rnode);

  preferences.begin("esp-smart-home", false);

  preferences.putBool("configured", true);

  preferences.putString("room", rroom);
  preferences.putString("ssid", rssid);
  preferences.putString("psk", rpsk);
  preferences.putString("hub", rhub);
  preferences.putString("node", rnode);
  preferences.putBool("loud", rbuzz);

  preferences.end();

  String response = R"rawliteral(
    <html>
      <head>
        <title>Submission Received</title>
        <style>
          body {
            font-family: Arial, sans-serif;
            background-color: #f0f0f0;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
          }
          .container {
            background-color: white;
            padding: 30px 40px;
            border-radius: 10px;
            box-shadow: 0 4px 8px rgba(0,0,0,0.1);
            text-align: center;
          }
          h2 {
            color: #4CAF50;
          }
          p {
            font-size: 16px;
            color: #333;
          }
        </style>
      </head>
      <body>
        <div class="container">
          <h2>Thank you!</h2>
  )rawliteral";

  response += "<p>SSID: " + rssid + "</p>";
  response += "<p>PSK: " + rpsk + "</p>";
  response += "<p>Room: " + rroom + "</p>";
  response += "<p>Hub: " + rhub + "</p>";
  response += "<p>Node: " + rnode + "</p><p>Using buzzer: ";
  response += rbuzz ? String("yes") : String("no");
  response += "</p><p>The Node will now reboot!</p>";
  response += "</div></body></html>";


  server.send(200, "text/html", response);
  delay(5000);
  ESP.restart();
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(3000);
  Serial.println("ESP SMART HOME NODE");

  pinMode(LED_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);

  initPreferences();

  if (configured) {
    initWiFiSTA();
  } else {
    initWiFiAP();

    // Define the root URL ("/") handler
    server.on("/", HTTP_GET, handleRoot);
    server.on("/submit", HTTP_POST, handleFormSubmit);

    // Start the HTTP server
    server.begin();

  }

  Serial.println("~~~~~~~~~~~~~~~~~");

  esp_sleep_enable_ext0_wakeup(GPIO_NUM_16, HIGH); // pir
  esp_sleep_enable_timer_wakeup(SLEEP_TIMER * 1000000);
}

void loop() {
  if (configured) {
    pirHandler();
    dhtHandler();
    ldrHandler();

    update();

    ++blinkCount;
    digitalWrite(LED_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);
    lastBlink = millis();

    if (blinkCount == 3) {
      blinkCount = 0;
      if (loud) {
        tone(BUZ_PIN, 440, 500);
      }
    }

    esp_deep_sleep_start();

  } else {
    server.handleClient();
  }
}

void pirHandler() {
  int pirState = digitalRead(PIR_PIN);

  if (pirState != pirPrevState && pirState == HIGH) {
    Serial.println("Motion detected!");
    motion = true;
  } else {
    Serial.println("No motion!");
    motion = false;
  }

  pirPrevState = pirState;
}

void dhtHandler() {
  // Attempt to read the temperature and humidity values from the DHT11 sensor.
  int result = dht11.readTemperatureHumidity(temperature, humidity);

  // Check the results of the readings.
  // If the reading is successful, print the temperature and humidity values.
  // If there are errors, print the appropriate error messages.
  if (result == 0) {
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print(" °C\tHumidity: ");
    Serial.print(humidity);
    Serial.println(" %");

  } else {
    // Print error message based on the error code.
    Serial.println(DHT11::getErrorString(result));
  }
}

void ldrHandler() {
  unsigned value = analogRead(LDR_PIN);
  double percent = value / 40.96;

  lightness = percent;
}

void update() {
  http.begin(serverURL + String("/update"));
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  int httpCode = http.POST(String("room=")+room+String("&node=")+node+String("&lightness=")+String(lightness)+
                           String("%&motion=")+String(motion)+String("&temperature=")+String(temperature)+
                           String("°C&humidity=")+String(humidity)+String("%"));

  // httpCode will be negative on error
  if (httpCode > 0) {
    // file found at server
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      // Serial.println(payload);
    } else {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET/POST... code: %d\n", httpCode);
    }
  } else {
    Serial.printf("[HTTP] GET/POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}