#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BME280.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define ONE_WIRE_BUS 2

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_BME280 bme;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);

void setup() {
  Serial.begin(115200);

  // OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED FAILED");
    while (1);
  }

  // BME280
  if (!bme.begin(0x76)) {
    Serial.println("BME280 FAILED");
    while (1);
  }

  // DS18B20
  ds18b20.begin();

  Serial.print("Found DS18B20 sensors: ");
  Serial.println(ds18b20.getDeviceCount());

  display.clearDisplay();
  display.display();
}

void loop() {

  // ---------- BME280 ----------
  float airC = bme.readTemperature();
  float airF = airC * 9.0 / 5.0 + 32.0;
  float humidity = bme.readHumidity();

  // ---------- DS18B20 ----------
  ds18b20.requestTemperatures();
  float probeC = ds18b20.getTempCByIndex(0);

  float probeF = -999;

  if (probeC != DEVICE_DISCONNECTED_C) {
    probeF = probeC * 9.0 / 5.0 + 32.0;
  }

  // ---------- SERIAL ----------
  Serial.print("Air: ");
  Serial.print(airF, 1);

  Serial.print("F  Hum: ");
  Serial.print(humidity, 1);

  Serial.print("%  Spot: ");

  if (probeF > -900)
    Serial.print(probeF, 1);
  else
    Serial.print("ERROR");

  Serial.println("F");

  // ---------- OLED ----------
  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(20, 0);
  display.println("Redfoot Monitor");

  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  // Air Temp
  display.setCursor(0, 18);
  display.print("Air:");
  display.setCursor(55, 18);
  display.print(airF, 1);
  display.print("F");

  // Humidity
  display.setCursor(0, 34);
  display.print("Hum:");
  display.setCursor(55, 34);
  display.print(humidity, 1);
  display.print("%");

  // Probe Temp
  display.setCursor(0, 50);
  display.print("Spot:");

  display.setCursor(55, 50);

  if (probeF > -900) {
    display.print(probeF, 1);
    display.print("F");
  } else {
    display.print("ERR");
  }

  display.display();

  delay(2000);
}
