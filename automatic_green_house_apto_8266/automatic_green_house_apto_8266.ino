#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

// Replace these with your network credentials
const char* ssid = "GoTravelClub.net";
const char* password = "roger_123";

// ThingSpeak API URL (base)
String serverName = "https://api.thingspeak.com/update?api_key=ZJ0WUU6IYY8UDHWV";

WiFiClientSecure wifiClient;

// Structure to define mapping between JSON keys and URL fields
struct FieldMapping {
  const char* jsonKey;
  const char* urlField;
};

// Array of field mappings
FieldMapping fieldMappings[] = {
  { "t", "field1" },
  { "h", "field2" },
  { "mr", "field3" },
  { "m", "field4" },
  { "i", "field5" },
  { "ls", "field6" },
};

// Function to construct URL from JSON document
String constructURL(const JsonDocument& jsonDoc) {
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

void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
}

void setup() {
  // Start the Serial communication to debug the results
  Serial.begin(115200);
  connectToWiFi();

  // Optional: Disable SSL certificate verification (not recommended for production)
  wifiClient.setInsecure();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Read all available data from Serial input
    String jsonData = "";
    while (Serial.available() > 0) {
      jsonData += Serial.readStringUntil('\n');
    }

    if (jsonData.length() > 0) {
      // Split the received data into individual JSON objects
      int startIndex = 0;
      while (startIndex < jsonData.length()) {
        int endIndex = jsonData.indexOf('\n', startIndex);
        if (endIndex == -1) {
          endIndex = jsonData.length();
        }

        String singleJsonData = jsonData.substring(startIndex, endIndex);
        startIndex = endIndex + 1;

        // Print received JSON string for debugging
        Serial.println("Received JSON: " + singleJsonData);

        DynamicJsonDocument jsonDoc(256);

        // Parse JSON data
        DeserializationError error = deserializeJson(jsonDoc, singleJsonData);

        if (error) {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.f_str());
          continue;
        }

        // Construct the URL using the separate function
        String fullURL = constructURL(jsonDoc);

        Serial.println(fullURL);  // Print constructed URL
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
      }
    } else {
      Serial.println("No data available on Serial");
    }
  } else {
    Serial.println("WiFi not connected");
    connectToWiFi();
  }

  delay(10000);  // Adjust delay as needed
}