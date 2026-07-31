#pragma once
#include "secrets.h"
#include "Arduino.h"
#include <stdint.h>
#include <DHT.h>

// ===== PROGRAM DEFINES =====
#define DHTTYPE DHT22
#define RGB_COUNT 211
#define COMPUTER_TRESHHOLD 200
#define WATERTANK_TRESHHOLD 500
#define BLINKING_SPEED 250
#define PC_TOGGLE_TIME 1000
#define PC_RESET_TIME 6000;

// ===== PIN DEFINITION =====

constexpr int key_pin = 12;
constexpr int switch_pin = 3;
constexpr int button_pin = 2;
constexpr int relay_pin = 9;
constexpr int pc_state_pin = A0;
constexpr int led_pin = 11;
constexpr int dht_pin = 8; // Incase-DHT
constexpr int ir_pin = A1;

constexpr int dht_power_pin = 4;
constexpr int dht1_pin = 7; // DHT AC 1
constexpr int dht2_pin = 6; // DHT AC 2
constexpr int water_sensor_pin = A2;

// ===== MQTT DEFINITIONS =====

const char broker[] = BROKER_HOST_ADRESS;
constexpr int port = BROKER_HOST_PORT;
const char mqtt_user[] = BROKER_USER;
const char mqtt_pass[] = BROKER_PASSWORD;
constexpr long publish_interval = 1000;

const char TOPIC_TEMP[] = "linus/sundgau74/desk/temperature";
const char TOPIC_HUMIDITY[] = "linus/sundgau74/desk/humidity";
const char TOPIC_AC_TEMP1[] = "linus/sundgau74/ac/temperature_before";
const char TOPIC_AC_TEMP2[] = "linus/sundgau74/ac/temperature_after";
const char TOPIC_AC_HUMID1[] = "linus/sundgau74/ac/humidity_before";
const char TOPIC_AC_HUMID2[] = "linus/sundgau74/ac/humidity_after";
const char TOPIC_PC_CMD[] = "linus/sundgau74/pc/command";
const char TOPIC_PC_STATUS[] = "linus/sundgau74/pc/status";
const char TOPIC_RGB_CMD[] = "linus/sundgau74/desk_rgb/command";
const char TOPIC_RGB_STATUS[] = "linus/sundgau74/desk_rgb/status";
const char TOPIC_RGB_STATUS_DIG[] = "linus/sundgau74/desk_rgb/status_dig";
const char TOPIC_AC_CMD[] = "linus/sundgau74/ac/command";
const char TOPIC_AC_WATER[] = "linus/sundgau74/ac/water_full";

// ===== NTP DEFINITIONS =====

const long NTP_TIME_OFFSET = 3600;
const char *NTP_SERVER = "pool.ntp.org";
