#include <WiFiManager.h>
#include <ESP8266Wifi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <Ticker.h>
#include <NTPClient.h>
#include <Adafruit_NeoPixel.h>

#define HOUR_LED_AMOUNT 36
#define MINUTE_LED_AMOUNT 72
#define HOURS_PIN 5
#define MINUTES_PIN 4

ESP8266WebServer server(80);

long utcOffsetInSeconds = 3 * 3600;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 3600000);

Ticker displayTicker;

WiFiManager wifiManager;

Adafruit_NeoPixel hourStrip = Adafruit_NeoPixel(HOUR_LED_AMOUNT, HOURS_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel minuteStrip = Adafruit_NeoPixel(MINUTE_LED_AMOUNT, MINUTES_PIN, NEO_GRB + NEO_KHZ800);

int brightness = 50;
uint32_t color = 0;

bool autoOnOffEnabled = true;
bool clockActive = true;
int offHour = 23;
int offMinute = 0;
int onHour = 7;
int onMinute = 0;

void setHourPixels(int hours) {
  int hourLedIndex = hours * 3;

  if (hours >= 12) 
    hourLedIndex -= 36;

  for (int i = hourLedIndex; i < (hourLedIndex + 3); i++) {
    hourStrip.setPixelColor(i, color);
  }
}

void setMinutePixels(int minutes) {
  int minutesUntilTile = 5;
  int tileAmount = minutes + minutes / 5;

  for (int i = 0; i < tileAmount; i++) {
    if (minutesUntilTile > 0) {
      minuteStrip.setPixelColor(i, color);
      minutesUntilTile--;
    } 
    else {
      minuteStrip.setPixelColor(i, 0x000000);
      minutesUntilTile = 5;
    }
  }
}

void DisplayTime() {
  Serial.println(timeClient.getFormattedTime());

  int hours = timeClient.getHours();
  int minutes = timeClient.getMinutes();

  if (autoOnOffEnabled) {
    
  }

  // Display hours
  hourStrip.clear();
  setHourPixels(hours);
  hourStrip.show();

  // Display minutes
  if (minutes == 0) {
    minuteStrip.clear();
  } 
  else {
    setMinutePixels(minutes);
  }
  minuteStrip.show();
}

void handleRoot() {
  String page = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <meta charset="UTF-8">
    <title>Clock Settings</title>
    <style>
      body { font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5; }
      h1 { color: #222; }
      form { background: #fff; padding: 20px; border-radius: 8px; box-shadow: 0 0 5px rgba(0,0,0,0.2); max-width: 400px; }
      label { display: block; margin-top: 15px; }
      input[type=number], input[type=color], input[type=time] {
        padding: 5px; width: 100%; box-sizing: border-box; margin-top: 5px;
      }
      input[type=submit] {
        margin-top: 20px; padding: 10px 20px;
        background: #4CAF50; color: white; border: none;
        border-radius: 4px; cursor: pointer;
      }
      input[type=submit]:hover {
        background: #45a049;
      }
    </style>
  </head>
  <body>
    <h1>Clock Settings</h1>
    <form action="/save" method="POST">
      <label>Timezone offset (hours):
        <input type="number" name="tz" value="3" step="1" min="-12" max="12">
      </label>

      <label>Brightness (0-255):
        <input type="number" name="brightness" value="128" min="0" max="255">
      </label>

      <label>LED color:
        <input type="color" name="color" value="#fca503">
      </label>

      <label>Auto-off start time:
        <input type="time" name="off_time" value="23:00">
      </label>

      <label>Auto-on start time:
        <input type="time" name="on_time" value="07:00">
      </label>

      <input type="submit" value="Save settings">
    </form>
  </body>
  </html>
  )rawliteral";

  server.send(200, "text/html", page);
}

void handleSave() {
  Serial.println("=== Received settings ===");

  if (server.hasArg("tz")) {
    int tz = server.arg("tz").toInt();
    utcOffsetInSeconds = tz * 3600;
    timeClient.setTimeOffset(utcOffsetInSeconds);
  }

  if (server.hasArg("brightness")) {
    int brightness = server.arg("brightness").toInt();
    if (brightness < 0) brightness = 0;
    if (brightness > 255) brightness = 255;

    hourStrip.setBrightness(brightness);
    minuteStrip.setBrightness(brightness);
  }

  if (server.hasArg("color")) {
    String hex = server.arg("color"); // например, "#fca503"
    long number = strtol(hex.substring(1).c_str(), NULL, 16);

    int r = (number >> 16) & 0xFF;
    int g = (number >> 8) & 0xFF;
    int b = number & 0xFF;

    color = minuteStrip.Color(r, g, b);
  }

  if (server.hasArg("off_time")) {
    String offTime = server.arg("off_time");
    offHour = offTime.substring(0,2).toInt();
    offMinute = offTime.substring(3).toInt();

    Serial.printf("Auto-off: %02d:%02d\n", offHour, offMinute);
  }

  if (server.hasArg("on_time")) {
    String onTime = server.arg("on_time");
    onHour = onTime.substring(0,2).toInt();
    onMinute = onTime.substring(3).toInt();

    Serial.printf("Auto-on: %02d:%02d\n", onHour, onMinute);
  }

  Serial.println("=========================");

  server.send(200, "text/html",
              "<h1>Settings saved!</h1><p><a href='/'>Back to settings</a></p>");
}

void handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

void setup() {
  pinMode(HOURS_PIN, OUTPUT);
  pinMode(MINUTES_PIN, OUTPUT);

  Serial.begin(9600);

  wifiManager.autoConnect("VCP-Clock");
  
  timeClient.begin();
  timeClient.setTimeOffset(utcOffsetInSeconds);
  timeClient.update();

  hourStrip.begin();
  minuteStrip.begin();

  hourStrip.setBrightness(brightness);
  minuteStrip.setBrightness(brightness);
  color = minuteStrip.Color(252, 165, 3);

  displayTicker.attach(1, DisplayTime);

  server.on("/", handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.onNotFound(handleNotFound);
  server.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();
  timeClient.update();
}
