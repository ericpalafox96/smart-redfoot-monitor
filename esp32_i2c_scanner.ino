#include <Wire.h>

void setup() {
  Wire.begin(21, 22);   // ESP32 SDA, SCL
  Serial.begin(115200);

  Serial.println("\nI2C Scanner");
}

void loop() {
  byte error, address;
  int nDevices = 0;

  Serial.println("Scanning...");

  for (address = 1; address < 127; address++) {

    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {

      Serial.print("Found: 0x");

      if (address < 16)
        Serial.print("0");

      Serial.println(address, HEX);

      nDevices++;
    }
  }

  if (nDevices == 0)
    Serial.println("No devices found");

  Serial.println();
  delay(3000);
}
