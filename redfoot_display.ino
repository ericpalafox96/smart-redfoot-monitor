#include <WiFi.h>
#include <WebServer.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BME280.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "RTClib.h"

#include "config.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define SDA_PIN 21
#define SCL_PIN 22
#define ONE_WIRE_BUS 4

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_BME280 bme;
RTC_DS3231 rtc;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);

WebServer server(80);

float airF = 0;
float humidity = 0;
float spotF = 0;
String timeString = "";

void updateReadings() {
  float airC = bme.readTemperature();
  airF = airC * 9.0 / 5.0 + 32.0;
  humidity = bme.readHumidity();

  ds18b20.requestTemperatures();
  spotF = ds18b20.getTempFByIndex(0);

  DateTime now = rtc.now();

  int hour12 = now.hour() % 12;
  if (hour12 == 0) hour12 = 12;

  const char* ampm = now.hour() >= 12 ? "PM" : "AM";

  timeString = String(hour12) + ":";
  if (now.minute() < 10) timeString += "0";
  timeString += String(now.minute()) + " " + ampm;
}

String buildDashboard() {
  String html = "";

  html += "<!DOCTYPE html><html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<meta http-equiv='refresh' content='5'>";
  html += "<title>Rafa's Redfoot Monitor</title>";

  html += "<style>";
  html += "body{font-family:Arial;background:#111;color:white;text-align:center;margin:0;padding:20px;}";
  html += "h1{font-size:26px;margin-bottom:20px;}";
  html += ".card{background:#222;border-radius:16px;padding:20px;margin:15px auto;max-width:350px;box-shadow:0 4px 12px #000;}";
  html += ".label{font-size:18px;color:#aaa;}";
  html += ".value{font-size:42px;font-weight:bold;margin-top:8px;}";
  html += ".small{font-size:18px;color:#aaa;margin-top:20px;}";
  html += "</style>";

  html += "</head><body>";
  html += "<h1>🐢 Rafa's Redfoot Monitor</h1>";

  html += "<div class='card'>";
  html += "<div class='label'>Air Temperature</div>";
  html += "<div class='value'>" + String(airF, 1) + "&deg;F</div>";
  html += "</div>";

  html += "<div class='card'>";
  html += "<div class='label'>Humidity</div>";
  html += "<div class='value'>" + String(humidity, 0) + "%</div>";
  html += "</div>";

  html += "<div class='card'>";
  html += "<div class='label'>Spot Temperature</div>";

  if (spotF < -100) {
    html += "<div class='value'>ERR</div>";
  } else {
    html += "<div class='value'>" + String(spotF, 1) + "&deg;F</div>";
  }

  html += "</div>";

  html += "<div class='small'>Updated: " + timeString + "</div>";
  html += "<div class='small'>Auto-refreshes every 5 seconds</div>";

  html += "</body></html>";

  return html;
}

void handleRoot() {
  updateReadings();
  server.send(200, "text/html", buildDashboard());
}

void handleData() {
  updateReadings();

  String json = "{";
  json += "\"air_f\":" + String(airF, 1) + ",";
  json += "\"humidity\":" + String(humidity, 1) + ",";
  json += "\"spot_f\":" + String(spotF, 1) + ",";
  json += "\"time\":\"" + timeString + "\"";
  json += "}";

  server.send(200, "application/json", json);
}

void updateOLED() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print(airF, 1);
  display.print("F");

  display.setTextSize(1);
  display.setCursor(0, 25);
  display.print("Hum: ");
  display.print(humidity, 0);
  display.print("%");

  display.setCursor(0, 39);
  display.print("Spot: ");

  if (spotF < -100) {
    display.print("ERR");
  } else {
    display.print(spotF, 1);
    display.print("F");
  }

  display.drawLine(0, 52, 127, 52, SSD1306_WHITE);

  display.setCursor(0, 56);
  display.print("IP:");
  display.print(WiFi.localIP());

  display.display();
}

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  bme.begin(0x76);
  rtc.begin();
  ds18b20.begin();

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.println("Connecting WiFi...");
  display.display();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();

  Serial.println("Web server started");
}

void loop() {
  server.handleClient();

  updateReadings();
  updateOLED();

  delay(1000);
}
