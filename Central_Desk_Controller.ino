#include "secrets.h"
#include <Arduino.h>
#include <WDT.h>
#include <DHT.h>
#include <FastLED.h>
#include "FspTimer.h"
#include <WiFiS3.h>
#include <WiFiUdp.h>
#include <Time.h>
#include <RTClib.h>
#include <NTPClient.h>
#include <MqttClient.h>

// ===== PROGRAM DEFINES =====

#define DHTTYPE DHT22
#define RGB_COUNT 211
#define COMPUTER_TRESHHOLD 250
#define BLINKING_SPEED 250

// ===== PIN DEFINITION =====

const int key_pin = 12;
const int switch_pin = 3;
const int button_pin = 2;
const int relay_pin = 9;
const int pc_state_pin = A0;
const int led_pin = 11;
const int dht_pin = 7;

// ===== MQTT DEFINITIONS =====

const char broker[] = BROKER_HOST_ADRESS;
int        port   = BROKER_HOST_PORT;
const char mqtt_user[] = BROKER_USER;
const char mqtt_pass[] = BROKER_PASSWORD;

const char TOPIC_TEMP[]   = "linus/haydn17/kellerzimmer/temperature";
const char TOPIC_PC_HUMIDITY[] = "linus/haydn17/kellerzimmer/humidity";
const char TOPIC_PC_CMD[]    = "linus/haydn17/kellerzimmer/pc/command";
const char TOPIC_PC_STATUS[] = "linus/haydn17/kellerzimmer/pc/status";
const char TOPIC_RGB_CMD[]   = "linus/haydn17/kellerzimmer/rgb/command";
const char TOPIC_RGB_STATUS[] = "linus/haydn17/kellerzimmer/rgb/status";

const long publish_interval = 1000;
unsigned long last_publish = 0;

// ===== NTP DEFINITIONS =====

const long NTP_TIME_OFFSET = 7200;
const char* NTP_SERVER = "pool.ntp.org";

// ===== RGB PROGRAMMS =====

#define RGB_UNKOWN -1
#define RGB_OFF 0
#define RGB_SWITCH_BLINK 1
#define RGB_RED 2
#define RGB_GREEN 3
#define RGB_BLUE 4

// ===== PROGRAMM VARIABLES =====

bool pc_status = false;
bool last_pc_status = false;

bool temperature = 0;
bool last_temperature = 0;
bool humidity = 0;
bool last_humidity = 0;

int rgb_programm = RGB_OFF;
int last_rgb_programm = RGB_UNKOWN;

int user_rgb_programm = RGB_OFF;
char rgb_brightness = 0xFF;
char last_rgb_brightness = 0xFF;
bool flushRGB = false;

FspTimer RGBTimer;
CRGB leds[RGB_COUNT];
DHT dht(dht_pin, DHTTYPE);
WiFiClient wifiClient;
WiFiUDP udp;
MqttClient mqttClient(wifiClient);
NTPClient timeClient(udp, NTP_SERVER, NTP_TIME_OFFSET, 60000);

// ===== METHOD-DEFINITION =====

void KeyChange();
void SwitchChange();
void ButtonChange();
bool BeginRGBTimer(float rate);
void RGBCallback(timer_callback_args_t __attribute((unused)) *p_args);
void updateRGB();


void setup() {
  Serial.begin(115200);
  Serial.println("Setup started");

  pinMode(key_pin, INPUT);
  pinMode(switch_pin, INPUT);
  pinMode(button_pin, INPUT);
  pinMode(pc_state_pin, INPUT);
  pinMode(relay_pin, OUTPUT);

  
  attachInterrupt(key_pin, KeyChange, CHANGE);
  attachInterrupt(switch_pin, SwitchChange, CHANGE);
  attachInterrupt(button_pin, ButtonChange, CHANGE);

  if(!BeginRGBTimer(20))Serial.println("Failed starting RGBTimer"); //20 Hz
  FastLED.addLeds<WS2812B, led_pin, GRB>(leds, RGB_COUNT);

}

void loop()
{
  updateRGB();
  if(Serial.available())
  {
    while(Serial.available())Serial.read();
    Serial.print("UserProg: ");
    Serial.println(user_rgb_programm);
  }
}

void updateRGB()
{
  if(rgb_brightness!=last_rgb_brightness)
  {
    FastLED.setBrightness(rgb_brightness);
    last_rgb_brightness=rgb_brightness;
    flushRGB=true;
  }
  if(flushRGB)
  {
    flushRGB = false;
    FastLED.show();
  }
}

void KeyChange()
{
  if(digitalRead(key_pin))
  {
    rgb_programm = RGB_RED;
  }
  else
  {
    rgb_programm = RGB_OFF;
    
  }
}

void SwitchChange()
{
  if(digitalRead(switch_pin))
  {
    rgb_programm = RGB_SWITCH_BLINK;
  }
  else
  {
    rgb_programm = (digitalRead(key_pin))?RGB_RED:user_rgb_programm;
  }
}

void ButtonChange()
{
  if(digitalRead(switch_pin))
  {
    if(digitalRead(button_pin)) digitalWrite(relay_pin, HIGH);
    else digitalWrite(relay_pin, LOW);
  }
  /*
  else
  {
    if(digitalRead(button_pin))
    {
      if(rgb_programm!=RGB_OFF)
      {
        user_rgb_programm = rgb_programm;
        rgb_programm = RGB_OFF;
      }
      else
      {
        if(user_rgb_programm==RGB_OFF)user_rgb_programm=RGB_BLUE;
        rgb_programm = user_rgb_programm;
      }
    }
  }
  */
}

void RGBCallback(timer_callback_args_t __attribute((unused)) *p_args)
{
  static unsigned long cycle_counter = 0;
  static int local_last_rgb_programm = RGB_UNKOWN;
  switch(rgb_programm)
  {
    case RGB_OFF:
      if(local_last_rgb_programm!=rgb_programm)
      {
        fill_solid(leds, RGB_COUNT, CRGB::Black);
        flushRGB = true;
      }
      break;
    case RGB_SWITCH_BLINK:
      if(rgb_programm!=local_last_rgb_programm)cycle_counter=0;
      if(!(cycle_counter%14))
      {
        fill_solid(leds, RGB_COUNT, CRGB::Red);
        flushRGB = true;
      }
      else if(!((cycle_counter+7)%14))
      {
        fill_solid(leds, RGB_COUNT, CRGB::Black);
        flushRGB = true;
      }
      break;
    case RGB_RED:
      if(local_last_rgb_programm!=rgb_programm)
        {
          fill_solid(leds, RGB_COUNT, CRGB::Red);
          flushRGB = true;
        }
        break;
    case RGB_GREEN:
      if(local_last_rgb_programm!=rgb_programm)
        {
          fill_solid(leds, RGB_COUNT, CRGB::Green);
          flushRGB = true;
        }
        break;
    case RGB_BLUE:
      if(local_last_rgb_programm!=rgb_programm)
        {
          fill_solid(leds, RGB_COUNT, CRGB::Blue);
          flushRGB = true;
        }
        break;
  }
  local_last_rgb_programm=rgb_programm;
  cycle_counter++;
}

bool BeginRGBTimer(float rate) {
  uint8_t timer_type = GPT_TIMER;
  int8_t tindex = FspTimer::get_available_timer(timer_type);
  if (tindex < 0){
    tindex = FspTimer::get_available_timer(timer_type, true);
  }
  if (tindex < 0){
    return false;
  }

  FspTimer::force_use_of_pwm_reserved_timer();

  if(!RGBTimer.begin(TIMER_MODE_PERIODIC, timer_type, tindex, rate, 0.0f, RGBCallback)){
    return false;
  }

  if (!RGBTimer.setup_overflow_irq()){
    return false;
  }

  if (!RGBTimer.open()){
    return false;
  }

  if (!RGBTimer.start()){
    return false;
  }
  return true;
}
