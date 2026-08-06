#pragma once
#include "secrets.h"
#include "Arduino.h"
#include <stdint.h>
#include <DHT.h>

// ===== PROGRAM DEFINES =====
#define DHTTYPE DHT22
#define RGB_COUNT 211
#define COMPUTER_TRESHHOLD 200
#define PC_STATE_SWITCH_TIME 2000
#define WATERTANK_TRESHHOLD 500
#define BLINKING_SPEED 250
#define PC_TOGGLE_TIME 1000
#define PC_RESET_TIME 6000;
#define WIFI_CHECK_INTERVAL 5000

// ===== RGB PROGRAMMS =====

#define COOLING 80
#define SPARKING 170
#define DEFAULT_RGB_PRG "RAINBOW"

// ===== AC DEFINES =====

#define AC_SRESET_DURATION 10000
#define AC_SOFFLINE_TIME 10000
#define AC_WATER_FULL_TIME 10000
#define AC_SENSOR_POLL 2500

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
constexpr long PUBLISH_INTERVALL = 2000;
constexpr long MQTT_RECONNECT_INTERVALL = 700;

#define TOPIC_TEMP "linus/sundgau74/desk/temperature"
#define TOPIC_HUMIDITY "linus/sundgau74/desk/humidity"
#define TOPIC_AC_TEMP1 "linus/sundgau74/ac/temperature_before"
#define TOPIC_AC_TEMP2 "linus/sundgau74/ac/temperature_after"
#define TOPIC_AC_HUMID1 "linus/sundgau74/ac/humidity_before"
#define TOPIC_AC_HUMID2 "linus/sundgau74/ac/humidity_after"
#define TOPIC_PC_CMD "linus/sundgau74/pc/command"
#define TOPIC_PC_STATUS "linus/sundgau74/pc/status"
#define TOPIC_RGB_CMD "linus/sundgau74/desk_rgb/command"
#define TOPIC_RGB_STATUS "linus/sundgau74/desk_rgb/status"
#define TOPIC_RGB_STATUS_DIG "linus/sundgau74/desk_rgb/status_dig"
#define TOPIC_AC_CMD "linus/sundgau74/ac/command"
#define TOPIC_AC_WATER "linus/sundgau74/ac/water_full"
#define TOPIC_AC_FEEDBACK "linus/sundgau74/ac/feedback"

// ===== NTP DEFINITIONS =====

const long NTP_TIME_OFFSET = 3600;
#define NTP_SERVER "pool.ntp.org"
