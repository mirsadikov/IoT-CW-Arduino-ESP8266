#include <DHT.h>
#include <ArduinoJson.h>

#define DHTPIN 3
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

const int PHOTORESISTOR_PIN = A0;
const int RELAY_PIN = 6;
const int LED_PIN = 9;
const int MOTOR_PIN = 11;

String fan_mode = "auto";
String lamp_mode = "auto";
int fan_speed = 500;
int lamp_brightness = 255;
bool relay = false;

float temperature = 0;
float humidity = 0;
float light_intensity = 0;

String jsonString = "";

unsigned long previousSensorReadMillis = 0;
const long sensorReadInterval = 3000;

void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(DHTPIN, INPUT);
  pinMode(PHOTORESISTOR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(MOTOR_PIN, OUTPUT);
}

void loop() {
  unsigned long currentMillis = millis();
  readEspSerial();

  // Read every 2 seconds
  if (currentMillis - previousSensorReadMillis >= sensorReadInterval) {
    previousSensorReadMillis = currentMillis;
    readSensors();
  }
}

void readEspSerial() {
  if (Serial.available()) {
    String chunk = Serial.readStringUntil('\n');
    chunk.trim();

    
    if (chunk.startsWith("<DATA>")) {
      jsonString = chunk.substring(6);
    } 
  }

  if (jsonString != "") {
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, jsonString);

    if (error) {
      Serial.println("Failed to parse JSON from ESP8266!");
      jsonString = "";
      return;
    }

    fan_mode = doc["fan_mode"].as<String>();
    lamp_mode = doc["lamp_mode"].as<String>();
    lamp_brightness = doc["lamp_brightness"];
    fan_speed = doc["fan_speed"];
    relay = doc["relay"];
    jsonString = "";

    updateActuator();
  }
}

void updateActuator() {
  // LED
  if (lamp_mode == "manual") {
    analogWrite(LED_PIN, lamp_brightness);
  } else if (lamp_mode == "off") {
    digitalWrite(LED_PIN, LOW);
  }

  // fan
  if (fan_mode == "manual") {
    analogWrite(MOTOR_PIN, fan_speed);
  } else if (fan_mode == "off") {
    digitalWrite(MOTOR_PIN, LOW);
  }

  // relay
  if (relay)
    digitalWrite(RELAY_PIN, HIGH);
  else
    digitalWrite(RELAY_PIN, LOW);
}

void readSensors() {
  readTemperature();
  readHumidity();
  readLight();

  StaticJsonDocument<200> doc;
  doc["temperature"] = temperature;
  doc["light_intensity"] = light_intensity;
  doc["humidity"] = humidity;

  String jsonStr;
  serializeJson(doc, jsonStr);

  Serial.println("<DATA>" + jsonStr);
}

float readHumidity() {
  float newHumidity = dht.readHumidity();

  if (isnan(newHumidity)) {
    Serial.println("Failed to read humidity from DHT sensor!");
    return;
  }
  humidity = newHumidity;
}

float readTemperature() {
  float newTemperature = dht.readTemperature();

  if (isnan(newTemperature)) {
    Serial.println("Failed to read temperature from DHT sensor!");
    return;
  } else {
    temperature = newTemperature;
    const int THRESHOLD = 30;
    if (fan_mode == "auto") {
      if (temperature > THRESHOLD) {
        digitalWrite(MOTOR_PIN, HIGH);
      } else {
        digitalWrite(MOTOR_PIN, LOW);
      }
    }
  }
}

int readLight() {
  light_intensity = analogRead(PHOTORESISTOR_PIN);
  const int THRESHOLD = 300;

  if (lamp_mode == "auto") {
    if (light_intensity < THRESHOLD) {
      digitalWrite(LED_PIN, HIGH);
    } else {
      digitalWrite(LED_PIN, LOW);
    }
  }
}