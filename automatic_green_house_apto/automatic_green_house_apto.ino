/**** Libraries *****/

// DHT-22 Sensor for temperature and humidity
#include <DHT.h>
#define DHT_PIN 7       // Pin connected to the DHT-22 sensor
#define DHT_TYPE DHT11  // DHT 11  (AM2302)
DHT TempSensor(DHT_PIN, DHT_TYPE);

// JSON library for data logging
#include <ArduinoJson.h>
StaticJsonDocument<256> jsonDoc;

// Relay for pump control
const int PumpPIN = 3;

// LCD display
#include <LiquidCrystal.h>
const int rs = 8, en = 9, d4 = 10, d5 = 11, d6 = 12, d7 = 13;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Light sensor
#define LIGHT_PIN 4  // Pin connected to DO pin of the LDR module

/**** Constants *****/
const int AirValue = 463;    // Sensor value when soil is completely dry
const int WaterValue = 210;  // Sensor value when submerged in water

/**** Global Variables *****/
float temp = 0.0;
float humidity = 0.0;
int moisturePercent = 0;
int irrigation = 0;
int lightState;

/**** Function Declarations *****/
void pumpOn(bool registerLog = true);
void pumpOff(bool registerLog = true);
void getTemperature();
void getHumidity();
void getMoisture();
void getLight();
void displayData();
void addData(const char* key, float value);
void printJson();

void setup() {
  Serial.begin(115200);
  //
  TempSensor.begin();
  delay(1000);
  // Initialize pump
  pinMode(PumpPIN, OUTPUT);
  pumpOff(false);  // Ensure pump is off initially

  // Initialize LCD
  lcd.begin(16, 2);

  // Initialize light sensor
  pinMode(LIGHT_PIN, INPUT);
}

void loop() {
  lcd.clear();

  getTemperature();
  delay(1000);
  getHumidity();
  getMoisture();

  if (moisturePercent < 25) {
    pumpOn();
  } else {
    pumpOff();
  }

  getLight();
  printJson();
  displayData();

  delay(1000);  // Add a delay to avoid overwhelming the loop
}

/**** Function Definitions *****/
void pumpOn(bool registerLog) {
  digitalWrite(PumpPIN, LOW);  // Turn the pump on
  irrigation = 1;              // Update irrigation state

  if (registerLog) {
    addData("irrigation", irrigation);  // Log the irrigation state
  }
}

void pumpOff(bool registerLog) {
  digitalWrite(PumpPIN, HIGH);  // Turn the pump off
  irrigation = 0;               // Update irrigation state

  if (registerLog) {
    addData("irrigation", irrigation);  // Log the irrigation state
  }
}

void getTemperature() {
  temp = TempSensor.readTemperature();
  addData("temp", temp);
}

void getHumidity() {
  humidity = TempSensor.readHumidity();
  addData("humidity", humidity);
}

void getMoisture() {
  int moistureRaw = analogRead(A0);  // Read moisture sensor value
  addData("moistureraw", moistureRaw);

  moisturePercent = map(moistureRaw, AirValue, WaterValue, 0, 100);
  moisturePercent = constrain(moisturePercent, 0, 100);
  addData("moisture", moisturePercent);
}

void getLight() {
  lightState = digitalRead(LIGHT_PIN);
  if (lightState == 1) {
    lightState = 0;
  } else {
    lightState = 1;
  }
  addData("lightstate", lightState);
}

void displayData() {
  lcd.setCursor(1, 0);
  lcd.print("Naturalment TV");

  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  lcd.print(temp);
  lcd.print(" C       ");
  delay(4000);

  lcd.setCursor(0, 1);
  lcd.print("Humedad: ");
  lcd.print(humidity);
  lcd.print(" %       ");
  delay(4000);

  lcd.setCursor(0, 1);
  lcd.print("Suelo C/A: ");
  lcd.print(moisturePercent);
  lcd.print(" %          ");
  delay(4000);

  lcd.setCursor(0, 1);
  lcd.print("Sol: ");
  if (lightState == 0)
    lcd.print("Ausente      ");
  else
    lcd.print("Presente     ");

  delay(4000);
}

void addData(const char* key, float value) {
  jsonDoc[key] = value;
}

void printJson() {
  String jsonString;
  serializeJson(jsonDoc, jsonString);
  Serial.println(jsonString);
}
