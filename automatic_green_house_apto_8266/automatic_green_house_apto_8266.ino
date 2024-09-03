#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

// Replace these with your network credentials
const char* ssid = "GoTravelClub.net";
const char* password = "roger_123";

// ThingSpeak API URL (base)
String serverName = "https://api.thingspeak.com/update?api_key=ZJ0WUU6IYY8UDHWV";

// Variables for dynamic data
int temperature = 26;  // Replace with your sensor reading
int humidity = 55;     // Replace with your sensor reading

WiFiClientSecure wifiClient;

// Structure to define mapping between JSON keys and URL fields
struct FieldMapping {
  const char* jsonKey;
  const char* urlField;
};

// Array of field mappings
FieldMapping fieldMappings[] = {
  { "temp", "field1" },
  { "humidity", "field2" },
   { "moisture", "field3" },
  { "irrigation", "field4" },
};

// Function to construct URL from JSON document
String constructURL(const StaticJsonDocument<256>& jsonDoc) {
  String fullURL = serverName;

  // Iterate over field mappings and construct URL
  for (FieldMapping mapping : fieldMappings) {
    if (jsonDoc.containsKey(mapping.jsonKey)) {
      String value = jsonDoc[mapping.jsonKey].as<String>();
      fullURL += "&" + String(mapping.urlField) + "=" + value;
    }
  }

  return fullURL;
}

void setup() {
  // Start the Serial communication to debug the results
  Serial.begin(115200);
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected to WiFi");

  // Optional: Disable SSL certificate verification (not recommended for production)
  wifiClient.setInsecure();

  //Serial.println("{\"temperature\":\"25\",\"humidity\":\"56\"}");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    delay(35000);
    HTTPClient http;

    String jsonData = Serial.readStringUntil('\n');  // Read JSON string from Arduino

    // Print received JSON string for debugging
    Serial.println("Received JSON: " + jsonData);

    StaticJsonDocument<256> jsonDoc;

    // Parse JSON data
    DeserializationError error = deserializeJson(jsonDoc, jsonData);

    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }

    // Construct the URL using the separate function
    String fullURL = constructURL(jsonDoc);

    Serial.println(fullURL);  // Print HTTP return code
    // Initialize the HTTPS request with WiFiClientSecure and the full URL
    http.begin(wifiClient, fullURL);  // Updated to use WiFiClientSecure
    int httpCode = http.GET();        // Send the request

    // Check the returning code
    if (httpCode > 0) {
      String payload = http.getString();  // Get the request response payload
      Serial.println(httpCode);           // Print HTTP return code
      Serial.println(payload);            // Print request response payload
    } else {
      Serial.println("Error on HTTP request");
    }

    http.end();  // Close connection
  } else {
    Serial.println("WiFi not connected");
  }
}
