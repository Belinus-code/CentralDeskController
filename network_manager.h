#pragma once
#include "secrets.h"
#include <WiFiS3.h>
#include <WiFiUdp.h>
#include <Time.h>
#include <RTClib.h>
#include <NTPClient.h>
#include <MqttClient.h>

// ===== MQTT DEFINITIONS =====

const char broker[] = BROKER_HOST_ADRESS;
int port = BROKER_HOST_PORT;
const char mqtt_user[] = BROKER_USER;
const char mqtt_pass[] = BROKER_PASSWORD;

const char TOPIC_TEMP[] = "linus/haydn17/kellerzimmer/temperature";
const char TOPIC_PC_HUMIDITY[] = "linus/haydn17/kellerzimmer/humidity";
const char TOPIC_PC_CMD[] = "linus/haydn17/kellerzimmer/pc/command";
const char TOPIC_PC_STATUS[] = "linus/haydn17/kellerzimmer/pc/status";
const char TOPIC_RGB_CMD[] = "linus/haydn17/kellerzimmer/rgb/command";
const char TOPIC_RGB_STATUS[] = "linus/haydn17/kellerzimmer/rgb/status";

const long publish_interval = 1000;
unsigned long last_publish = 0;

// ===== NTP DEFINITIONS =====

const long NTP_TIME_OFFSET = 3600;
const char* NTP_SERVER = "pool.ntp.org";

void ConnectMqtt() {
  mqttClient.onMessage(OnMqttMessage);
  mqttClient.setUsernamePassword(mqtt_user, mqtt_pass);

  String clientId = "arduino-uno-r4-" + String(random(0xffff), HEX);
  mqttClient.setId(clientId);

  Serial.print("Connecting to MQTT broker '");
  Serial.print(broker);
  Serial.print("'...");
  while (!mqttClient.connect(broker, port)) {
    Serial.print(".");
    delay(700);
    WDT.refresh();
  }
  Serial.println("\nMQTT connected!");

  mqttClient.subscribe(TOPIC_PC_CMD, 2);
  mqttClient.subscribe(TOPIC_RGB_CMD, 2);
}

void ConnectWifi() {
  IPAddress dns(8, 8, 8, 8);
  WiFi.setDNS(dns);

  Serial.print("Connecting to Wifi '");
  Serial.print(WIFI_SSID);
  Serial.print("'...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    WDT.refresh();
  }
  Serial.println("\nConnected! IP-Adress: " + WiFi.localIP().toString());
}

void PublishData() {
  static IAnimation* local_last_animation = nullptr;

  // ===== Collect Data =====

  pc_status = (analogRead(pc_state_pin) > COMPUTER_TRESHHOLD) ? true : false;
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  // ===== Publish Data =====

  if (pc_status != last_pc_status) {
    last_pc_status = pc_status;
    mqttClient.beginMessage(TOPIC_PC_STATUS, true, 1);  // topic, retained, qos
    mqttClient.print(pc_status);
    mqttClient.endMessage();
  }

  if (user_animation != local_last_animation) {
    local_last_animation = user_animation;
    mqttClient.beginMessage(TOPIC_RGB_STATUS, true, 1);  // topic, retained, qos
    mqttClient.print(user_animation->GetName());
    mqttClient.endMessage();
  }

  if (isnan(temperature) || isnan(humidity)) {
    //Failed to read DHT Data, dont publish garbage data
    return;
  }

  if (temperature != last_temperature) {
    last_temperature = temperature;
    mqttClient.beginMessage(TOPIC_TEMP, false, 0);  // topic, retained, qos
    mqttClient.print(temperature);
    mqttClient.endMessage();
  }

  if (abs(humidity - last_humidity) != 0) {
    last_humidity = humidity;
    mqttClient.beginMessage(TOPIC_PC_HUMIDITY, false, 0);
    mqttClient.print(humidity);
    mqttClient.endMessage();
  }
}

void OnMqttMessage(int messageSize) {
  String topic = mqttClient.messageTopic();
  String payload = "";
  while (mqttClient.available()) {
    payload += (char)mqttClient.read();
  }

  Serial.print("Received message on topic: ");
  Serial.println(topic);
  Serial.print("Payload: ");
  Serial.println(payload);

  if (topic == TOPIC_PC_CMD) {
    handlePcCommand(payload);
  }

  if (topic == TOPIC_RGB_CMD) {
    handleRgbCommand(payload);
  }
}


/*
class NetworkManager
{
  public:
    NetworkManager(){}
    void begin()
    {

    }
}
*/
