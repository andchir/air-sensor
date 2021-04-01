
#include <Wire.h>
#include <WiFiManager.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_Sensor.h>

// AM2320
#include <Adafruit_AM2320.h>

// DS18B20
#include <OneWire.h> 
#include <DallasTemperature.h>

// BME280
#include <Adafruit_BME280.h>

// PMS5003
#include <SoftwareSerial.h>
#include "AirGradient.h"

// DHT
#include <DHT.h>

// OLED display
#include <SSD1306Wire.h>

AirGradient ag = AirGradient();

// OLED display
SSD1306Wire display(0x3c, SDA, SCL);

// Set sensors that you do not use to false
boolean hasPM = true;
boolean hasCO2 = false;
boolean hasSHT = false;
boolean hasDHT = false;
boolean hasAM2320 = false;
boolean hasDS18B20 = true;
boolean hasBME280 = true;
boolean connectWIFI = true;
boolean exportToNarodmon = true;
boolean exportToShopker = false;

boolean deepSleepEnabled = true;// Enable deep sleep mode. Connect D0 to RST pin.

// AM2320 sensor
Adafruit_AM2320 am2320 = Adafruit_AM2320();

// DHT sensor
#define DHTPIN D4 // What pin we're connected to
// Uncomment whatever type you're using!
//#define DHTTYPE DHT12 // DHT 11
#define DHTTYPE DHT22 // DHT 22  (AM2302)
//#define DHTTYPE DHT21 // DHT 21 (AM2301)
DHT dht(DHTPIN, DHTTYPE);

// DS18B20 sensor
#define ONE_WIRE_BUS D4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// BME280
Adafruit_BME280 bme;
Adafruit_Sensor *bme_temp = bme.getTemperatureSensor();
Adafruit_Sensor *bme_pressure = bme.getPressureSensor();
Adafruit_Sensor *bme_humidity = bme.getHumiditySensor();

const int buttonPin = D3;
int buttonState = 0;
int buttonPressedLoop = 0;
boolean buttonIsEnabled = false;

unsigned long timeMillis;
float valueTemp = 0;
float valueTempCorrection = 0;
int valueHum = 0;
float valuePr = 0;
int valuePm2 = 0;
int valuePm10 = 0;

// Intervals
int intervalDisplay = 3000;// 3 seconds
// int intervalSend = 5 * 60 * 1000;// 5 minutes
// For deep sleep mode:
int intervalSend = 30 * 1000;// 30 seconds
int sleepDurationSeconds = 4.5 * 60;// 4 minutes 30 seconds

// Narodmon.ru settings
char apiUrl[30] = "http://narodmon.ru/json";
char apiOwnerName[20] = "andchir";
char apiSensorName[20] = "AirSensor";
char apiSensorLat[10] = "61.784807";
char apiSensorLon[10] = "34.346085";
char apiSensorAlt[5] = "135";

// Shopker settings
char shopkerApiUrl[46] = "http://your-domain.com/api/ru/user_content/19";
char shopkerApiKey[121] = "xxxxxx";

void setup(){
  timeMillis = 0;
  Serial.begin(9600);
  Serial.println("Initializing");

  pinMode(buttonPin, INPUT);
  
  //Wire.begin(); // join I2C bus
  sensor_t sensor;

  display.init();
  display.flipScreenVertically();
  delay(2000);
  display.clear();
  
  showTextRectangle("Init", String(ESP.getChipId(), HEX), "", "", "", true);

  if (hasPM) {
    ag.PMS_Init();
    ag.wakeUp();
  }
  if (hasSHT) ag.TMP_RH_Init(0x45);
  if (hasDHT) dht.begin();
  if (hasAM2320) am2320.begin();
  if (hasDS18B20) sensors.begin();
  if (hasBME280) bme.begin(0x76);
  
  if (connectWIFI) connectToWifi();
  delay(2000);
}

void loop(){

  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH) {
    buttonPressedLoop = 0;
  } else {
    buttonPressedLoop++;
    if (buttonPressedLoop == 1) {
      buttonIsEnabled = !buttonIsEnabled;
    }
  }

  if (buttonIsEnabled) {
    display.displayOn();
    showTextRectangle(
      "T: " + String(valueTemp),
      "H: " + String(valueHum),
      "Pr: " + String(valuePr),
      "PM2.5: " + String(valuePm2),
      "PM10: " + String(valuePm10),
      true
    );
  } else {
    display.clear();
    display.displayOff();
  }
  if (buttonPressedLoop == 8) {
    buttonIsEnabled = true;
    Serial.println("Reset WIFI settings");
    display.displayOn();
    showTextRectangle("RESET", "WIFI", "CONFIG", "", "", true);
    WiFi.disconnect();
    delay(3000);
    ESP.restart();
    delay(5000);
  }

  if (timeMillis % intervalDisplay == 0) {

    // Read values
    if (hasPM) {
      PMS_DATA data;
      if (ag.getPM_Data(data)) {
        valuePm2 = data.PM_AE_UG_2_5;
        valuePm10 = data.PM_AE_UG_10_0;
      } else {
        Serial.println("Could not read from PM sensor");
        //showTextRectangle("No PM data", "", "", "", "", true);
      }
    }
    if (hasSHT) {
      TMP_RH result = ag.periodicFetchData();
      if (result.error == SHT3XD_NO_ERROR) {
        valueTemp = result.t - valueTempCorrection;
        valueHum = result.rh;
      } else {
        Serial.println("Could not read from SHT sensor");
        Serial.println("Error code: " + String(result.error));
      }
    }
    if (hasDHT) {
      float h = dht.readHumidity();
      float t = dht.readTemperature();
      if (isnan(h) || isnan(t)) {
        Serial.println("Failed to read from DHT sensor!");
      } else {
        valueTemp = t - valueTempCorrection;
        valueHum = h;
      }
    }
    if (hasAM2320) {
      valueTemp = am2320.readTemperature() - valueTempCorrection;
      valueHum = am2320.readHumidity();
    }
    if (hasDS18B20) {
      sensors.requestTemperatures();
      valueTemp = sensors.getTempCByIndex(0);
    }
    if (hasBME280) {
      sensors_event_t temp_event, pressure_event, humidity_event;
      bme_temp->getEvent(&temp_event);
      bme_pressure->getEvent(&pressure_event);
      bme_humidity->getEvent(&humidity_event);
      valuePr = pressure_event.pressure / 1.333;// mmHg (мм.рт.ст.)
      valueHum = humidity_event.relative_humidity;
      if (!hasDS18B20) {
        valueTemp = temp_event.temperature;
      }
    }
    
    Serial.println("Temperature: " + String(valueTemp));
    Serial.println("Humidity: " + String(valueHum));
    Serial.println("Pressure: " + String(valuePr));
    Serial.println("PM2.5: " + String(valuePm2));
    Serial.println("PM10: " + String(valuePm10));
    
    // Display data on OLED
    if (buttonIsEnabled) {
      Serial.println("Display data update");
      showTextRectangle(
        "T: " + String(valueTemp),
        "H: " + String(valueHum),
        "Pr: " + String(valuePr),
        "PM2.5: " + String(valuePm2),
        "PM10: " + String(valuePm10),
        true
      );
    }
  }

  // Send data by API
  if (timeMillis > 0 && timeMillis % intervalSend == 0) {
    if (connectWIFI){
      Serial.println("Send data by API");
      if (exportToNarodmon) {
        sendDataToNarodmon(valueTemp, valueHum, valuePm2, valuePm10, valuePr);
      }
      if (exportToShopker) {
        sendDataToShopker(valueTemp, valueHum, valuePm2, valuePm10, valuePr);
      }
      if (deepSleepEnabled && !buttonIsEnabled) {
        Serial.println("Go to deep sleep");
        if (hasPM) ag.sleep();
        ESP.deepSleep(sleepDurationSeconds * 1000000);
      }
    }
    timeMillis = 0;
  }
  
  delay(500);
  timeMillis += 500;
}

void sendDataToNarodmon(float temp, float hum, int pm2, int pm10, float pr) {
  Serial.println("API URL: " + String(apiUrl));
  Serial.println("API owner: " + String(apiOwnerName));

  String jsonString = "{\"devices\":[{\"mac\":\"" + WiFi.macAddress() + "\", \"name\":\"" + String(apiSensorName) + "\", ";
  jsonString += "\"owner\":\"" + String(apiOwnerName) + "\", ";
  //jsonString += "\"lat\":" + String(apiSensorLat) + ", ";
  //jsonString += "\"lon\":" + String(apiSensorLon) + ", ";
  //jsonString += "\"alt\":" + String(apiSensorAlt) + ", ";
  jsonString += "\"sensors\": [";
  
  jsonString += "{\"id\":\"T1\", \"name\":\"Температура\", \"value\":" + String(temp) + ", \"unit\":\"C\"},";
  jsonString += "{\"id\":\"H1\", \"name\":\"Влажность\", \"value\":" + String(hum) + ", \"unit\":\"%\"},";
  jsonString += "{\"id\":\"P1\", \"name\":\"Микрочастицы PM2.5\", \"value\":" + String(pm2) + ", \"unit\":\"мкг/м3\"},";
  jsonString += "{\"id\":\"P2\", \"name\":\"Пыль PM10\", \"value\":" + String(pm10) + ", \"unit\":\"мкг/м3\"},";
  jsonString += "{\"id\":\"Pr1\", \"name\":\"Давление\", \"value\":" + String(pr) + ", \"unit\":\"мм.рт.ст.\"}";
  
  jsonString += "]}]}";
  Serial.println(jsonString);

  HTTPClient http;
  http.begin(apiUrl);
  http.addHeader("Host", "narodmon.ru");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.addHeader("Content-Length", String(jsonString.length()));
  int httpCode = http.POST(jsonString);
  String response = http.getString();
  Serial.println("Response code: " + String(httpCode));
  Serial.println("Response: " + response);
  http.end();
}

void sendDataToShopker(float temp, float hum, int pm2, int pm10, float pr) {
  Serial.println("API URL: " + String(shopkerApiUrl));
  
  String jsonString = "{";
  jsonString += "\"temperature\":" + String(temp) + ", ";
  jsonString += "\"humidity\":" + String(hum) + ", ";
  jsonString += "\"pm25\":" + String(pm2) + ", ";
  jsonString += "\"pm10\":" + String(pm10) + ", ";
  jsonString += "\"pr\":" + String(pr);
  jsonString += "}";

  Serial.println(jsonString);

  HTTPClient http;
  http.begin(shopkerApiUrl);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Accept", "application/json");
  http.addHeader("Content-Length", String(jsonString.length()));
  http.addHeader("X-AUTH-TOKEN", String(shopkerApiKey));
  int httpCode = http.POST(jsonString);
  String response = http.getString();
  Serial.println("Response code: " + String(httpCode));
  Serial.println("Response: " + response);
  http.end();
}

// DISPLAY
void showTextRectangle(String ln1, String ln2, String ln3, String ln4, String ln5, boolean small) {
  //display.clearDisplay();
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  int fontSize;
  if (small) {
    fontSize = 10;
    display.setFont(ArialMT_Plain_10);
  } else {
    fontSize = 16;
    display.setFont(ArialMT_Plain_16);
  }

  display.drawString(32, 14, ln1);
  display.drawString(32, 14 + fontSize, ln2);
  display.drawString(32, 14 + fontSize * 2, ln3);
  display.drawString(32, 14 + fontSize * 3, ln4);
  display.drawString(32, 14 + fontSize * 4, ln5);
  
  display.display();
}

// Wifi Manager
void connectToWifi(){
  Serial.println("WIFI connection...");
  WiFiManager wifiManager;
  // Custom fields
  WiFiManagerParameter custom_api_server("api_server", "API server", apiUrl, 30);
  WiFiManagerParameter custom_api_owner("api_owner", "API user", apiOwnerName, 30);
  wifiManager.addParameter(&custom_api_server);
  wifiManager.addParameter(&custom_api_owner);
  
  String HOTSPOT = "AIR-SENSOR-" + String(ESP.getChipId(), HEX);
  wifiManager.setTimeout(120);
  if(!wifiManager.autoConnect((const char*)HOTSPOT.c_str())) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      ESP.restart();
      delay(5000);
  } else {
    strcpy(apiUrl, custom_api_server.getValue());
    strcpy(apiOwnerName, custom_api_owner.getValue());
    Serial.println("MAC address: " + WiFi.macAddress());
    Serial.println("API URL: " + String(apiUrl));
    Serial.println("API user: " + String(apiOwnerName));
  }
}
