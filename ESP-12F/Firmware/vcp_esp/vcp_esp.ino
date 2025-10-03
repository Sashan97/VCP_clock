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

bool isInOnOffInterval(int curMin, int offMin, int onMin) {
  if (offMin == onMin) return false;
  if (offMin < onMin) {
    return (curMin >= offMin && curMin < onMin);
  } else {
    // for cases involving the transition through midnight
    return (curMin >= offMin) || (curMin < onMin);
  }
}

void setHourPixels(int hours) {
  hourStrip.clear();
  int hourLedIndex = hours * 3;

  if (hours >= 12) 
    hourLedIndex -= 36;

  for (int i = hourLedIndex; i < (hourLedIndex + 3); i++) {
    hourStrip.setPixelColor(i, color);
  }
}

void setMinutePixels(int minutes) {
  if (minutes == 0) {
    minuteStrip.clear();
  }

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

  int minutesSinceMidnight = hours * 60 + minutes;
  int offMin = offHour * 60 + offMinute;
  int onMin = onHour * 60 + onMinute;

  bool shouldBeActive = true;
  if (autoOnOffEnabled) {
    if (isInOnOffInterval(minutesSinceMidnight, offMin, onMin)) {
      shouldBeActive = false;
    }
    else {
      shouldBeActive = true;
    }
  }

  if (shouldBeActive != clockActive) {
    clockActive = shouldBeActive;
    if (!clockActive) {
      hourStrip.clear();
      minuteStrip.clear();
      hourStrip.show();
      minuteStrip.show();
    }
    else {
      setHourPixels(hours);
      hourStrip.show();
      setMinutePixels(minutes);
      minuteStrip.show();
    }
  }

  if (!clockActive) return;

  // Display hours
  setHourPixels(hours);
  hourStrip.show();

  // Display minutes
  setMinutePixels(minutes);
  minuteStrip.show();
}

void handleRoot() {
  int tzHours = utcOffsetInSeconds / 3600;

  uint32_t c = color;
  uint8_t r = (c >> 16) & 0xFF;
  uint8_t g = (c >> 8) & 0xFF;
  uint8_t b = c & 0xFF;
  char colorHex[8];
  sprintf(colorHex, "#%02X%02X%02X", r, g, b);

  char offBuf[6], onBuf[6];
  sprintf(offBuf, "%02d:%02d", offHour, offMinute);
  sprintf(onBuf, "%02d:%02d", onHour, onMinute);

  String checked = autoOnOffEnabled ? "checked" : "";

  String page = String(R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <meta charset="UTF-8">
    <title>Clock Settings</title>
    <style>
      body { font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5; }
      h1 { color: #222; }
      form { background: #fff; padding: 20px; border-radius: 8px; box-shadow: 0 0 5px rgba(0,0,0,0.2); max-width: 420px; }
      label { display: block; margin-top: 12px; }
      input[type=number], input[type=color], input[type=time] {
        padding: 6px; width: 100%; box-sizing: border-box; margin-top: 6px;
      }
      input[type=submit] {
        margin-top: 18px; padding: 10px 18px;
        background: #4CAF50; color: white; border: none;
        border-radius: 4px; cursor: pointer;
      }
      input[type=submit]:hover { background: #45a049; }
    </style>
  </head>
  <body>
    <h1>Clock Settings</h1>
    <form action="/save" method="POST">
  )rawliteral");

  page += String("<label>Timezone offset (hours):<input type=\"number\" name=\"tz\" value=\"") + String(tzHours) + String("\" step=\"1\" min=\"-12\" max=\"12\"></label>\n");

  page += String("<label>Brightness (0-255):<input type=\"number\" name=\"brightness\" value=\"") + String(brightness) + String("\" min=\"0\" max=\"255\"></label>\n");

  page += String("<label>LED color:<input type=\"color\" name=\"color\" value=\"") + String(colorHex) + String("\"></label>\n");
  
  page += "<label><input type=\"checkbox\" name=\"auto_onoff\" " + checked + "> Enable automatic on/off</label>\n";

  page += String("<label>Auto-off start time:<input type=\"time\" name=\"off_time\" value=\"") + String(offBuf) + String("\"></label>\n");

  page += String("<label>Auto-on start time:<input type=\"time\" name=\"on_time\" value=\"") + String(onBuf) + String("\"></label>\n");

  page += R"rawliteral(
      <input type="submit" value="Save settings">
    </form>
  </body>
  </html>
  )rawliteral";

  server.send(200, "text/html", page);
}

void handleSave() {
  if (server.hasArg("tz")) {
    int tz = server.arg("tz").toInt();
    utcOffsetInSeconds = tz * 3600;
    timeClient.setTimeOffset(utcOffsetInSeconds);
  }

  if (server.hasArg("brightness")) {
    int b = server.arg("brightness").toInt();
    if (b < 0) b = 0;
    if (b > 255) b = 255;
    brightness = b;

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

  if (server.hasArg("auto_onoff")) {
    autoOnOffEnabled = true;
  } else {
    autoOnOffEnabled = false;
  }

  if (server.hasArg("off_time")) {
    String offTime = server.arg("off_time");
    if (offTime.length() >= 5) {
      offHour = offTime.substring(0,2).toInt();
      offMinute = offTime.substring(3,2+3).toInt();
    }
  }

  if (server.hasArg("on_time")) {
    String onTime = server.arg("on_time");
    if (onTime.length() >= 5) {
      onHour = onTime.substring(0,2).toInt();
      onMinute = onTime.substring(3,2+3).toInt();
    }
  }

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
