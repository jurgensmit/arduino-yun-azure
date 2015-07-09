/*
 * Arduino Yun Azure - Connect the Arduino Yun to Azure
 *
 * Circuit: 
 *
 * - A temperature sensor connected to analog port A1
 * - A light sensor connected to analog port A1
 * - A humidity sensor connected to digital pin 8
 * - A red led connected to digital pin 11
 * - A green led connected to digital pin 10
 * - A yellow led connected to digital pin 9
 *
 * (c) 2015 Jurgen Smit. All rights reserved.
 */

#include <Process.h>
#include <Wire.h>
#include <DHT.h>
#include "rgb_lcd.h"

#define TEMPERATURE_PIN      1     // Arduino (analog) pin tied to the temperature sensor
#define PHOTORESISTOR_PIN    2     // Arduino (analog) pin tied to the photoresistor
#define DHT_PIN              8     // Arduino pin tied to the DHT temperature and humidity sensor
#define DHT_TYPE             DHT11 // The type of DHT sensor we are using (DHT11, DHT21 or DHT22)
#define RED_LED_PIN          11    // Arduino pin tied to the anode of the red led
#define GREEN_LED_PIN        10    // Arduino pin tied to the anode of the green led
#define YELLOW_LED_PIN       9     // Arduino pin tied to the anode of the yellow led

DHT dht(DHT_PIN, DHT_TYPE);
rgb_lcd lcd;

void setup() {
  lcd.begin(16, 2);   // Set up the LCD's number of columns and rows:
  lcd.setColorAll();  // Turn off the backlight
  lcd.clear();
  lcd.print(F("Arduino Azure"));

  Serial.begin(115200);

  Bridge.begin();  

  delay(2000);  // wait 2 seconds
}

/*
 * Print a message to the 2nd line of the LCD display
 */
void printMessage(const String& message) {
  lcd.setCursor(0, 1);
  lcd.print(message + String(F("                ")));
}

/*
 * Measure and return the value of the photoresistor
 */
unsigned int getLight() {
  return analogRead(PHOTORESISTOR_PIN);
}

/*
 * Measure and return the current temperature
 */
float getTemperature() {
  int a = analogRead(TEMPERATURE_PIN);
  float resistance = (float)(1023 - a) * 10000 / a; //get the resistance of the sensor
  int B = 3975; 
  float temperature = 1 / (log(resistance/10000) / B + 1 / 298.15) - 273.15; //convert to temperature via datasheet
  return temperature;
}

/*
 * Measure and return the humidity
 */
float getHumidity() {
  float humidity = dht.readHumidity();
  return humidity;
}

/*
 * Send the give sensor values to Azure 
 */
void sendSensorValues(float temperature, float humidity, unsigned int light) {
  Process p;

  printMessage(F("Sending..."));

  String sensorValues = "temperature:" + String(temperature) + ",humidity:" + String(humidity) + ",light:" + String(light);

  p.begin(F("python"));
  p.addParameter(F("/root/sendeventhub.py"));
  p.addParameter(sensorValues);
  p.addParameter(F("arduino-yun-jurgen"));
  p.addParameter(F("&2>1")); // pipe error output to stdout
  p.run();

  char resultCode[4] = "\0\0\0";

  int i = 0;
  while (p.available()) {
    char c = p.read();
    if(i < 3) {
      resultCode[i++] = c;
    }
  }

  printMessage(resultCode);
  printMessage(String(temperature));
}

void loop() {
  // Get the current sensor values
  float temperature = getTemperature();
  float humidity = getHumidity();
  unsigned light = getLight();

  // Send the sensor values to Azure
  sendSensorValues(temperature, humidity, light);

  // Wait a while
  delay(10000);
}
