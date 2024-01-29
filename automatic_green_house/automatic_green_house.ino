/**** Libraries *****/

// LCD
#include <LiquidCrystal.h>
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;


// AM2320 I2C temperature and humidity sensor
#include <Adafruit_Sensor.h>
#include <Adafruit_AM2320.h>

Adafruit_AM2320 TempSensor = Adafruit_AM2320();

//
#include <SoftwareSerial.h>
SoftwareSerial gprsSerial(9, 10); // tX,rX

// IR sensor
int IRSensorPIN = 13;
//
int PumpPIN = 6;

/**** Main *****/

float temp;
float humidity;
float moisture;
float moisture_raw;
float sunlight;
int irrigation;

int counter = 0;

// Mouiter



void setup() {
  // set up the LCD's number of columns and rows:
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

  TempSensor.begin();

  // sensor pin INPUT
  pinMode (IRSensorPIN, INPUT);

  // Relay
  pinMode(PumpPIN, OUTPUT);
  digitalWrite(PumpPIN, HIGH);

  // put your setup code here, to run once:
  Serial.begin(9600);
  gprsSerial.begin(9600);
}

void loop() {
  digitalWrite(PumpPIN, LOW );

  // Input
  get_temp();
  get_moisture();

  if (moisture < 20)
  {
    pump_on();

  }
  else{
    pump_off();
    }

  // Output
  int IRStatus = digitalRead (IRSensorPIN);

  Serial.println("IRStatus");
  Serial.println(IRStatus);

  //if (IRStatus == 0)
    display_on();
  
    // Sent data API
    Serial.println("Counter");
    Serial.println(counter);
    if (counter == 45) {
      put_data();
      counter = 0;
    }
    delay(1000);
  counter++;
  irrigation = 0;
}
void get_moisture() {
  const int AirValue = 790;   //This is the sensor value when completely dry
  const int WaterValue = 390; //This is the sensor value when submerged in water.
  int soil_moisture = 0;
  int soil_moisture_percent = 0;
  soil_moisture = analogRead(A0);  //put Sensor insert into soil

   moisture_raw = soil_moisture;
  Serial.println(soil_moisture);
  soil_moisture_percent = map(soil_moisture, AirValue, WaterValue, 0, 100);
    Serial.println("mouiter %");

  Serial.println(soil_moisture_percent);
  if (soil_moisture_percent >= 100)
  {
    moisture = 100;
  }
  else if (soil_moisture_percent <= 0)
  {
    moisture = 0 ;
  }
  else if (soil_moisture_percent > 0 && soil_moisture_percent < 100)
  {
    moisture = soil_moisture_percent;
  }

}

void get_temp() {

  temp = TempSensor.readTemperature();
  delay(1000);
  humidity = TempSensor.readHumidity();
}

void display_on() {
  LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
  lcd.begin(16, 2);
  delay(1000);
  lcd.setCursor(0, 0);
  lcd.print("SEThub - AGH");
  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  lcd.print(temp);
  lcd.print(" C");
  delay(3000);
  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  lcd.print(humidity);
  lcd.print(" %");
  delay(3000);
  lcd.setCursor(0, 1);
  lcd.print("Soil M/C: ");
  lcd.print(moisture);
  lcd.print(" %");
  delay(3000);

  lcd.clear();
}

void pump_on() {
  digitalWrite(PumpPIN, LOW );
  irrigation = 1;

}

void pump_off() {
  digitalWrite(PumpPIN, HIGH  );
  irrigation = 0;

}

void put_data() {
  if (gprsSerial.available())
  {
    Serial.write(gprsSerial.read());

  }

  gprsSerial.println("AT");
  delay(1000);
  ShowSerialData();
/*
  gprsSerial.println("AT+CPIN?");
  delay(1000);

  gprsSerial.println("AT+CREG?");
  delay(1000);

  gprsSerial.println("AT+CGATT?");
  delay(1000);

  gprsSerial.println("AT+CIPSHUT");
  delay(1000);

  gprsSerial.println("AT+CIPSTATUS");
  delay(2000);*/

  gprsSerial.println("AT+CIPMUX=0");
  delay(2000);

  ShowSerialData();

  gprsSerial.println("AT+CSTT=\"data.lycamobile.ie\",\"lmie\",\"plus\"");//start task and setting the APN,
      Serial.println(gprsSerial.readString());

  delay(1000);  

  ShowSerialData();

  gprsSerial.println("AT+CIICR");//bring up wireless connection
  delay(3000);

  ShowSerialData();

  gprsSerial.println("AT+CIFSR");//get local IP adress
  delay(2000);

  ShowSerialData();

  gprsSerial.println("AT+CIPSPRT=0");
  delay(3000);

  ShowSerialData();

  gprsSerial.println("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",\"80\"");//start up the connection
  delay(6000);

  ShowSerialData();

  gprsSerial.println("AT+CIPSEND");//begin send data to remote server
  delay(4000);
  ShowSerialData();

  String str = "GET https://api.thingspeak.com/update?api_key=VE8ZMHII1G58ZGE6&field1=" + String(temp) + "&field2=" + String(humidity) + "&field3=" + String(moisture) + "&field4=" + String(sunlight) + "&field5=" + String(irrigation)+ "&field6=" + String(moisture_raw);
  Serial.println(str);
  gprsSerial.println(str);//begin send data to remote server

  delay(4000);
  ShowSerialData();

  gprsSerial.println((char)26);//sending
  delay(5000);//waitting for reply, important! the time is base on the condition of internet
  gprsSerial.println();

  ShowSerialData();

  gprsSerial.println("AT+CIPSHUT");//close the connection
  delay(100);
  ShowSerialData();

}
void ShowSerialData()
{
  Serial.println("data");
  while (gprsSerial.available() != 0)
    Serial.write(gprsSerial.read());
  delay(5000);

}
