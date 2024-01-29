/**** Libraries *****/

// SPI Scrren
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>

#define TFT_RST    8  // you can also connect this to the Arduino reset
#define TFT_DC     9
#define TFT_CS     10
#define TFT_SCLK 13   // set these to be whatever pins you like!
#define TFT_MOSI 11   // set these to be whatever pins you like!

// color definitions
const uint16_t  Display_Color_Black        = 0x0000;
const uint16_t  Display_Color_Blue         = 0x001F;
const uint16_t  Display_Color_Red          = 0xF800;
const uint16_t  Display_Color_Green        = 0x07E0;
const uint16_t  Display_Color_Cyan         = 0x07FF;
const uint16_t  Display_Color_Magenta      = 0xF81F;
const uint16_t  Display_Color_Yellow       = 0xFFE0;
const uint16_t  Display_Color_White        = 0xFFFF;

// The colors we actually want to use
uint16_t        Display_Text_Color         = Display_Color_Black;
uint16_t        Display_Backround_Color    = Display_Color_Blue;

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// AM2320 I2C temperature and humidity sensor
#include <Adafruit_Sensor.h>
#include <Adafruit_AM2320.h>

Adafruit_AM2320 TempSensor = Adafruit_AM2320();

//
#include <SoftwareSerial.h>
SoftwareSerial SIMmodule(5, 4); // tX,rX

//
int PumpPIN = 2;

/**** Main *****/

float temp;
float humidity;
int soil_moisture_raw = 0;
int soil_moisture_percent = 0;
int soil_moisture_percent_calc = 0;
float sunlight;
char sunlightperiod;
int irrigation;

int counter = 0;
char buf[100];

String apn = "3internet.ie";                    //APN
String apn_u = "";                     //APN-Username
String apn_p = "";                     //APN-Password
String url = " https://api.thingspeak.com/update?api_key=VE8ZMHII1G58ZGE6";  //URL of Server


void setup()
{
  TempSensor.begin();
  // Relay
  pinMode(PumpPIN, OUTPUT);
  // Screen
  tft.initR(INITR_MINI160x80);  // Init ST7735S mini display
  tft.setRotation(3);
  tft.setFont();

  Serial.begin(9600);
  //SIMmodule.begin(9600);
  /*
     delay(15000);
    while (SIMmodule.available()) {
      Serial.write(SIMmodule.read());
    }*/
  delay(2000);
  //gsm_config_gprs();
}

void loop()
{
  print_on_screen("SetHUB", "AGH", Display_Color_Black);

  // Input
  get_temperature();
  get_humidity();
  get_moisture();
  get_light();
  if (soil_moisture_percent < 70)
  {
    pump_on();

  }
  else {
    pump_off();
  }


  if (counter == 60 || irrigation == 1) {
    //put_data();
    counter = 0;
  }
  delay(2000);
  counter++;
  irrigation = 0;

  // gsm_http_post("field1=100");
  // delay(30000);
  //put_data();
  //send_sms();
}
void get_moisture()
{
  const int AirValue = 810;   //This is the sensor value when completely dry
  const int WaterValue = 440; //This is the sensor value when submerged in water.

  soil_moisture_raw = analogRead(A0);  //put Sensor insert into soil
  dtostrf(soil_moisture_raw, 3, 1, buf);
  print_on_serial("Moisture Raw", buf);

  soil_moisture_percent_calc = map(soil_moisture_raw, AirValue, WaterValue, 0, 100);
  Serial.println(soil_moisture_percent_calc);
  if (soil_moisture_percent_calc >= 100)
  {
    soil_moisture_percent = 100;
  }
  else if (soil_moisture_percent_calc <= 0)
  {
    soil_moisture_percent = 0 ;
  }
  else if (soil_moisture_percent_calc > 0 && soil_moisture_percent_calc < 100)
  {
    soil_moisture_percent = soil_moisture_percent_calc;
  }
  dtostrf(soil_moisture_percent, 3, 1, buf);
  print_on_screen("Moisture", buf, Display_Color_Black);
  print_on_serial("Moisture percentaje", buf);// the raw analog reading




}

void get_temperature()
{
  temp = TempSensor.readTemperature();
  dtostrf(temp, 3, 1, buf);
  print_on_screen("Temp:", buf, Display_Color_Black);
}

void get_humidity()
{
  humidity = TempSensor.readHumidity();
  dtostrf(humidity, 3, 1, buf);
  print_on_screen("Humidity", buf, Display_Color_Black);
}

void get_light()
{
  sunlight = analogRead(A1);

  // We'll have a few threshholds, qualitatively determined
  if (sunlight < 25) {
    print_on_screen("Sunlight", "Dark", Display_Color_Black);

    sunlightperiod = "Dark";
  } else if (sunlight < 200) {
    sunlightperiod =  "Dim";
    print_on_screen("Sunlight", "Dim", Display_Color_Black);

  } else if (sunlight < 500) {
    sunlightperiod = "Light";
    print_on_screen("Sunlight", "Light", Display_Color_Black);

  } else if (sunlight < 800) {
    sunlightperiod = "Bright";
    print_on_screen("Sunlight", "Bright", Display_Color_Black);

  } else {
    sunlightperiod = "Very bright";
    print_on_screen("Sunlight", "Very bright", Display_Color_Black);

  }
  Serial.println(sunlightperiod);

  //print_on_serial("Sunlight raw reading", sunlight)// the raw analog reading
  dtostrf(sunlight, 3, 1, buf);
  print_on_serial("Sunlight raw", buf);// the raw analog reading
  dtostrf(sunlightperiod, 3, 1, buf);

  print_on_serial("Sunlight period", buf);// the raw analog reading
}

void pump_on()
{
  digitalWrite(PumpPIN, LOW);
  irrigation = 1;
}

void pump_off()
{
  digitalWrite(PumpPIN, HIGH);
  irrigation = 0;
}

void put_data()
{
  if (SIMmodule.available())
  {
    Serial.write(SIMmodule.read());
  }
  SIMmodule.println("AT");
  delay(1000);
  print_SIM_response();
  SIMmodule.println("AT+CPIN?");
  delay(1000);
  print_SIM_response();
  SIMmodule.println("AT+CREG?");
  delay(1000);
  print_SIM_response();
  SIMmodule.println("AT+CIPSHUT");
  delay(1000);
  SIMmodule.println("AT+CGATT=1");
  delay(1000);
  print_SIM_response();

  SIMmodule.println("AT+CIPMUX=0");
  delay(2000);
  print_SIM_response();



  SIMmodule.println("AT+CSTT=\"airtelgprs.com\"");//start task and setting the APN,
  delay(1000);

  print_SIM_response();

  SIMmodule.println("AT+CIICR");//bring up wireless connection
  delay(3000);

  print_SIM_response();

  SIMmodule.println("AT+CIFSR");//get local IP adress
  delay(2000);

  print_SIM_response();

  SIMmodule.println("AT+CIPSPRT=0");
  delay(3000);

  print_SIM_response();

  SIMmodule.println("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",\"80\"");//start up the connection
  delay(6000);

  print_SIM_response();

  SIMmodule.println("AT+CIPSEND");//begin send data to remote server
  delay(4000);
  print_SIM_response();

  String str = "GET https://api.thingspeak.com/update?api_key=O13AOCHYYNU2LQ19&field1=" + String(0) + "&field2=" + String(0);
  Serial.println(str);
  SIMmodule.println(str);//begin send data to remote server

  delay(4000);
  print_SIM_response();

  SIMmodule.println((char)26);//sending
  delay(5000);//waitting for reply, important! the time is base on the condition of internet
  SIMmodule.println();

  print_SIM_response();

  SIMmodule.println("AT+CIPSHUT");//close the connection
  delay(100);
  print_SIM_response();
}

void send_sms()
{
  SIMmodule.println("AT"); //Once the handshake test is successful, it will back to OK
  print_SIM_response();

  SIMmodule.println("AT+CMGF=1"); // Configuring TEXT mode
  print_SIM_response();
  SIMmodule.println("AT+CMGS=\"+35385128543\"");//change ZZ with country code and xxxxxxxxxxx with phone number to sms
  print_SIM_response();
  SIMmodule.println("Hola Antonio!"); //text content
  print_SIM_response();
  SIMmodule.println((char)26);//sending

}
void print_on_screen(
  char *title,
  char *text,
  uint16_t color)
{
  tft.setCursor(5, 5);
  tft.fillScreen(ST77XX_WHITE);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.setTextSize(2);
  tft.print(title);
  tft.setCursor(5, 55);
  tft.setTextSize(3);
  tft.print(text);
  delay(4000);
}

void print_on_serial(char *title, char *value) {
  Serial.println(title);
  Serial.println(value);
}

void print_SIM_response()
{
  while (SIMmodule.available() != 0)
  {
    Serial.write(SIMmodule.read());
  }
  delay(5000);
}

void gsm_http_post( String postdata) {
  Serial.println(" --- Start GPRS & HTTP --- ");
  gsm_send_serial("AT+SAPBR=1,1");
  gsm_send_serial("AT+SAPBR=2,1");
  gsm_send_serial("AT+HTTPINIT");
  gsm_send_serial("AT+HTTPPARA=CID,1");
  gsm_send_serial("AT+HTTPPARA=URL," + url);
  gsm_send_serial("AT+HTTPPARA=CONTENT,application/x-www-form-urlencoded");
  gsm_send_serial("AT+HTTPDATA=192,5000");
  gsm_send_serial(postdata);
  gsm_send_serial("AT+HTTPACTION=1");
  gsm_send_serial("AT+HTTPREAD");
  gsm_send_serial("AT+HTTPTERM");
  gsm_send_serial("AT+SAPBR=0,1");
}

void gsm_config_gprs() {
  Serial.println(" --- CONFIG GPRS --- ");
  gsm_send_serial("AT+SAPBR=3,1,CONTYPE,GPRS");
  gsm_send_serial("AT+SAPBR=3,1,APN," + apn);
  if (apn_u != "") {
    gsm_send_serial("AT+SAPBR=3,1,USER," + apn_u);
  }
  if (apn_p != "") {
    gsm_send_serial("AT+SAPBR=3,1,PWD," + apn_p);
  }
}

void gsm_send_serial(String command) {
  Serial.println("Send ->: " + command);
  SIMmodule.println(command);
  long wtimer = millis();
  while (wtimer + 3000 > millis()) {
    while (SIMmodule.available()) {
      Serial.write(SIMmodule.read());
    }
  }
  Serial.println();
}
