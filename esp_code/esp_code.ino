#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <FirebaseESP8266.h>

#define ssid "Home"
#define password "mmmxn2023"

#define FIREBASE_HOST "nodemcu-zeyd-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "AIzaSyBiTl6S6NO7o1VEzgA_Y4VDfUoVAsMb_l0"

FirebaseData firebaseData;
String jsonString = "";

unsigned long previousDbReadMillis = 0;
const long dbReadInterval = 3000;

void setup() {
  Serial.begin(115200);
  connectWifi();
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void connectWifi() {
  WiFi.begin(ssid, password); // connect to Wi-Fi network
  while (WiFi.status() != WL_CONNECTED) { // wait for connection
    delay(1000);
    Serial.println("Connecting to Wi-Fi network...");
  }
  Serial.println("Connected to Wi-Fi network");
}

void loop() { 
  unsigned long currentMillis = millis();  
  readArduinoSerial();

  // Read every 2 secods
  if (currentMillis - previousDbReadMillis >= dbReadInterval) {
    previousDbReadMillis = currentMillis;
    readFromDatabase();
  }
}

void readFromDatabase() {
  FirebaseData dataJson;
  Firebase.getJSON(dataJson, "/");

  Serial.println("<DATA>" + dataJson.jsonString());
}

void readArduinoSerial() {
  if (Serial.available()) {
    String chunk = Serial.readStringUntil('\n');
    chunk.trim();

    if (chunk.startsWith("<DATA>")) {
      jsonString = chunk.substring(6);
    } 
  }

  if (jsonString != "") {
    FirebaseJson json;
    json.setJsonData(jsonString);
    Firebase.updateNodeSilent(firebaseData, "/", json);
    jsonString = "";
  }
}
