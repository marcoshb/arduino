/**** Libraries *****/

// DHT-22 Sensor temperature and humidity sensor
#include <DHT.h>
#define DHT_PIN 2       // what pin we're connected to
#define DHT_TYPE DHT22  // DHT 22  (AM2302)
DHT TempSensor = DHT(DHT_PIN, DHT_TYPE);

// Json
#include <ArduinoJson.h>
StaticJsonDocument<256> jsonDoc;

// Relay
int PumpPIN = 3;

// LCD
#include <LiquidCrystal.h>
const int rs = 8, en = 9, d4 = 10, d5 = 11, d6 = 12, d7 = 13;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Ligth sensor
#define LIGHT_PIN 4  // Arduino Nano's pin connected to DO pin of the ldr module

/**** Main *****/
float temp;
float humidity;
int moisture_percent = 0;
float moisture_raw = 0;
int irrigation = 0;


void pump_on(boolean register_log = true) {
  digitalWrite(PumpPIN, LOW);  // Turn the pump on
  irrigation = 1;              // Set irrigation state to 1 (on)

  if (register_log) {                   // If logging is enabled
    addData("irrigation", irrigation);  // Log the irrigation state
  }
}
void pump_off(boolean register_log = true) {
  digitalWrite(PumpPIN, HIGH);  // Turn the pump off (assuming HIGH is off for this setup)
  irrigation = 0;               // Set irrigation state to 0 (off)

  if (register_log) {                   // Check if logging is enabled
    addData("irrigation", irrigation);  // Log the irrigation state
  }
}

void setup() {
  Serial.begin(115200);

  // Pump
  pinMode(PumpPIN, OUTPUT);
  pump_off(false);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  // Light sensor
  pinMode(LIGHT_PIN, INPUT);
}

void loop() {
  lcd.clear();
  delay(1000);
  get_temp();
  delay(1000);
  get_humidity();
    delay(1000);
  get_moisture();
  if (moisture_percent < 25) {
    pump_on();

  } else {
    pump_off();
  }
  get_light();

  printJson();
  display_on();
  delay(1000);
}

void get_temp() {

  temp = TempSensor.readTemperature();
  addData("temp", temp);
}

void get_humidity() {
  humidity = TempSensor.readHumidity();
  addData("humidity", humidity);
}

void get_moisture() {
  const int AirValue = 463;    //This is the sensor value when completely dry
  const int WaterValue = 210;  //This is the sensor value when submerged in water.
  int moisture_raw = 0;

  moisture_raw = analogRead(A0);  //put Sensor insert into soil
  addData("moisture_raw", moisture_raw);
  moisture_percent = map(moisture_raw, AirValue, WaterValue, 0, 100);
  if (moisture_percent > 99) {
    moisture_percent = 100;
  } else if (moisture_percent < 1) {
    moisture_percent = 0;
  }
  addData("moisture", moisture_percent);
}

void get_light() {
  int light_state = digitalRead(LIGHT_PIN);
   addData("light_state", light_state);
}

void display_on() {


  lcd.setCursor(1, 0);
  lcd.print("Naturalment TV");
  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  lcd.print(temp);
  lcd.print(" C       ");
  delay(3000);

  lcd.setCursor(0, 1);
  lcd.print("Humedad: ");
  lcd.print(humidity);
  lcd.print(" %       ");
  delay(3000);

  lcd.setCursor(0, 1);
  lcd.print("Suelo C/A: ");
  lcd.print(moisture_percent);
  lcd.print(" %          ");
  delay(3000);
}


void addData(const char* key, float value) {
  // Add or update data in JSON
  jsonDoc[key] = value;
}

void printJson() {
  // Convert JSON object to string and print it
  String jsonString;
  serializeJson(jsonDoc, jsonString);
  Serial.println(jsonString);
}
