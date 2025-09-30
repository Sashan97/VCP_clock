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

const long utcOffsetInSeconds = 3 * 3600;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds, 3600000);

Ticker displayTicker;

WiFiManager wifiManager;

Adafruit_NeoPixel hourStrip = Adafruit_NeoPixel(HOUR_LED_AMOUNT, HOURS_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel minuteStrip = Adafruit_NeoPixel(MINUTE_LED_AMOUNT, MINUTES_PIN, NEO_GRB + NEO_KHZ800);

uint32_t color = minuteStrip.Color(252, 165, 3);

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
  server.send(200, "text/html", "<h1>Clock settings</h1>");
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
  timeClient.update();

  displayTicker.attach(1, DisplayTime);

  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();
  timeClient.update();
}
