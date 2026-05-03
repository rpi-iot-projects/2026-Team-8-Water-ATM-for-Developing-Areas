#include <SPI.h>
#include <MFRC522.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "secret.h"
#include <cstdio>
#include <utility>

#define SS_PIN 5
#define RST_PIN 22
#define VALVE_PIN 4
#define STATUS_PIN 2

MFRC522 rfid(SS_PIN, RST_PIN);

WiFiClient espClient;
PubSubClient mqttClient;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  SPI.begin();
  rfid.PCD_Init();

  init_mqtt();

  pinMode(VALVE_PIN, OUTPUT);
  digitalWrite(VALVE_PIN, LOW); 
  pinMode(STATUS_PIN, OUTPUT);
  digitalWrite(STATUS_PIN, LOW);
  pinMode(WATER_FLOW_PIN, INPUT);

  Serial.println("Starting...");
  Serial.println("Please tap RFID card...");
}

void init_mqtt() {
  delay(10);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Initializing MQTT publisher...");
  mqttClient.setClient(espClient);
  mqttClient.setServer(broker_ip_address, 1883);
  Serial.println("Connected to broker ");
  Serial.print(broker_ip_address);
  Serial.print(" port 1883");
  Serial.println();
}

std::pair<bool, char*> rfid_read() {

  static char uid[32] = {};
  if (!rfid.PICC_IsNewCardPresent()) {
    return std::make_pair(0, uid);
  }

  if (!rfid.PICC_ReadCardSerial()) {
    return std::make_pair(0, uid);
  }
 
  Serial.print("Card detected! UID: ");

  uint8_t offset = 0;
  for (uint8_t i = 0; i < rfid.uid.size; i++) {

    if (i < rfid.uid.size - 1) {
      offset += std::snprintf(uid + offset, sizeof(uid) - offset, "%02X ", rfid.uid.uidByte[i]);
    } else {
      offset += std::snprintf(uid + offset, sizeof(uid) - offset, "%02X", rfid.uid.uidByte[i]);
    }

  }

  Serial.print(uid);
  Serial.println();

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  return std::make_pair(1, uid);
}

void open_valve() {
  Serial.println("Opening valve...");
  digitalWrite(VALVE_PIN, HIGH);
  delay(15000);
  digitalWrite(VALVE_PIN, LOW);
  Serial.println("Valve closed. waiting for next user");
}

void mqtt_send(const char* uid) {
  StaticJsonDocument<200> doc;

  doc["success"] = "success";
  doc["uid"] = uid;

  char payload_buffer[200];
  serializeJson(doc, payload_buffer);
  
  mqttClient.publish("esp32/wateratm", payload_buffer);
}

void reconnect() {

  while (!mqttClient.connected()) {
    digitalWrite(STATUS_PIN, LOW);
    Serial.print("Attempting MQTT connection...");

    if (mqttClient.connect("wateratm01")) {
      Serial.println("connected");
      digitalWrite(STATUS_PIN, HIGH);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");

      delay(5000);
    }
  }


}

void loop() {
  // put your main code here, to run repeatedly:
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();
  
  
  auto [read_success, uid] = rfid_read();
  if (read_success) {
    open_valve();
    mqtt_send(uid);
  }
  
}
