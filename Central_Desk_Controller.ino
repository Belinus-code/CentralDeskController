#include <Arduino.h>
#include "secrets.h"
#include <WDT.h>
#include <DHT.h>
#include <FastLED.h>
#include "animations.h"
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
#define COMPUTER_TRESHHOLD 200
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

const long NTP_TIME_OFFSET = 3600;
const char* NTP_SERVER = "pool.ntp.org";

// ===== RGB PROGRAMMS =====

#define RGB_UNKOWN -1
#define RGB_OFF 0
#define RGB_SWITCH_BLINK 1
#define RGB_RED 2
#define RGB_GREEN 3
#define RGB_BLUE 4
#define RGB_FIRE 5

#define COOLING  80
#define SPARKING 170

// ===== PROGRAMM VARIABLES =====

bool pc_status = false;
bool last_pc_status = false;

float temperature = 0;
float last_temperature = 0;
float humidity = 0;
float last_humidity = 0;

IAnimation* animation = nullptr;
IAnimation* last_animation = nullptr;
IAnimation* last_user_animation = nullptr;

char rgb_brightness = 0xFF;
char last_rgb_brightness = 0xFF;
bool flushRGB = false;

unsigned long startEpoch = 0;

Preferences prefs;
FspTimer RGBTimer;
CRGB leds[RGB_COUNT];
WiFiClient wifiClient;
WiFiUDP udp;
DHT dht(dht_pin, DHTTYPE);
AnimationManager animationManager(leds, RGB_COUNT, prefs);
MqttClient mqttClient(wifiClient);
NTPClient timeClient(udp, NTP_SERVER, NTP_TIME_OFFSET, 60000);

// ===== METHOD-DEFINITION =====

void KeyChange();
void SwitchChange();
void ButtonChange();
bool BeginRGBTimer(float rate);
void RGBCallback(timer_callback_args_t __attribute((unused)) *p_args);
void UpdateRGB();
void ConnectWifi();
void ConnectMqtt();
void PublishData();
void UpdateMqtt();
void OnMqttMessage();
void handlePcCommand(String command);
void handleRgbCommand(String command);
void FireAnimation();
void SerialIncome();


void setup() {
  noInterrupts();
  Serial.begin(115200);
  delay(2000);
  Serial.println("Setup started");

  pinMode(key_pin, INPUT);
  pinMode(switch_pin, INPUT);
  pinMode(button_pin, INPUT);
  pinMode(pc_state_pin, INPUT);
  pinMode(relay_pin, OUTPUT);

  
  attachInterrupt(key_pin, KeyChange, CHANGE);
  attachInterrupt(switch_pin, SwitchChange, CHANGE);
  attachInterrupt(button_pin, ButtonChange, CHANGE);

  dht.begin();
  FastLED.addLeds<WS2812B, led_pin, GRB>(leds, RGB_COUNT).setCorrection(TypicalLEDStrip);

  //Ensure that Animations that are needed by programm do exsist
  Serial.println("Setting up animations");
  animationManager.begin();

  if (animationManager.getAnimationIndex("OFF") == -1) {
    AnimationSetting* newSettings = animationManager.createSettingsStaticColor(0, 255, "OFF");
    animationManager.createAnimation(newSettings);
    delete newSettings;
  }

  if (animationManager.getAnimationIndex("RED") == -1) {
    AnimationSetting* newSettings = animationManager.createSettingsStaticColor(0xFF0000, 255, "RED");
    animationManager.createAnimation(newSettings);
    delete newSettings;
  }
  
   if (animationManager.getAnimationIndex("SWITCH_BLINK") == -1) {
    AnimationSetting* newSettings = animationManager.createSettingsBlink(0xFF0000,0,8,255,"SWITCH_BLINK");
    animationManager.createAnimation(newSettings);
    delete newSettings;
  }
  animation = animationManager.getAnimationByName("OFF");
  last_user_animation = animationManager.getAnimationByName("OFF");
  Serial.println("Done with animations");

  ConnectWifi();
  ConnectMqtt();
  timeClient.begin();
  WDT.begin(5000);
  startEpoch = timeClient.getEpochTime();
  timeClient.update();
  BeginRGBTimer(10); //10 Hz

  interrupts();
  Serial.println("Finished Setup, starting loop...");
}

void loop()
{
  SerialIncome();
  UpdateRGB();
  UpdateMqtt();
  WDT.refresh();
}

void UpdateRGB()
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

void UpdateMqtt()
{
  if(!mqttClient.connected())
  {
    Serial.println("MQTT Connection lost. Reconnect.");
    ConnectMqtt();
  }
  mqttClient.poll();
  if (millis() - last_publish > publish_interval) {
    last_publish = millis();
    PublishData();
  }
}

void KeyChange()
{
  if(digitalRead(key_pin))
  {
    last_user_animation=animation;
    animation = animationManager.getAnimationByName("RED");
  }
  else
  {
    animation = last_user_animation;
  }
}

void SwitchChange()
{
  if(digitalRead(switch_pin))
  {
    animation = animationManager.getAnimationByName("SWITCH_BLINK");
  }
  else
  {
    animation = (digitalRead(key_pin))?animationManager.getAnimationByName("RED"):last_user_animation;
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
  static IAnimation* local_last_animation = nullptr;
  
  if(animation==nullptr)return;
  if(animation!=local_last_animation)
  {
    animation->RestartAnimation();
    flushRGB = true;
  }
  flushRGB |= animation->Update(cycle_counter);

  local_last_animation=animation;
  cycle_counter++;
}

void SerialIncome() {
  static String application = "";
  String input="";
  while (Serial.available()>0) {
    delayMicroseconds(90);  
    char c = Serial.read();
    input += c;
  }
  if(input.length()==0)return;
  input.trim();
  if(input.startsWith("rgb") || application=="rgb")
  {
    application = "rgb";
    if(input == "rgb")
    {
      Serial.println("rgb started");
      return;
    }
    else if (input.startsWith("rgb "))input = input.substring(4);

    if(input=="exit")application="";
    else handleRgbCommand(input);
  }
  else if(input.startsWith("dump"))
  {
    unsigned long runtime = timeClient.getEpochTime() - startEpoch; // in Sekunden
    unsigned long hours = runtime / 3600;
    unsigned long minutes = (runtime % 3600) / 60;
    unsigned long seconds = runtime % 60;
    String runtimeStr = String(hours) + ":" +
                    (minutes < 10 ? "0" : "") + minutes + ":" +
                    (seconds < 10 ? "0" : "") + seconds;
    String pc_state = (pc_state)?"ON":"OFF";
    Serial.println(runtimeStr);
    Serial.print("Time: ");
    Serial.println(timeClient.getFormattedTime());
    Serial.println("IP-Adress: " + WiFi.localIP().toString());
    Serial.print("Temperature: ");
    Serial.println(temperature);
    Serial.print("Humidity: ");
    Serial.println(humidity);
    Serial.println("PC State: " + pc_state);
    Serial.print("PC Raw Value: ");
    Serial.println(analogRead(pc_state_pin));
    Serial.print("PC Treshold: ");
    Serial.println(COMPUTER_TRESHHOLD);
    Serial.print("RGB Programm: ");
    if(animation == nullptr) Serial.println("<nullptr>");
    else Serial.println(animation->GetName());
  }
  else if(input.startsWith("help"))
  {
    Serial.println("help - list of commands");
    Serial.println("dump - dump status and sensor data");
    Serial.println("rgb - start rgb application");
  }
  else
  {
    Serial.println("Unkown Command. Type 'help' for a list of commands");
  }
}

void handlePcCommand(String command) {
  if (command == "TOGGLE") {
    digitalWrite(relay_pin,HIGH);
    delay(1000);
    digitalWrite(relay_pin,LOW);
  }
  else if (command == "RESET")
  {
    digitalWrite(relay_pin,HIGH);
    WDT.refresh();
    delay(3000);
    WDT.refresh();
    delay(3000);
    WDT.refresh();
    digitalWrite(relay_pin,LOW);
  }
  else {
    Serial.print("Unknown PC command: ");
    Serial.println(command);
  }
}

void handleRgbCommand(String command) {
  Serial.println(command);
  if(command.startsWith("set") && !command.startsWith("setting"))
  {
    if(command=="set")
    {
      Serial.println("'set' can be used to set an animation. \nUsage: 'set COLOR'");
      return;
    }
    String color = command.substring(4);
    int index = animationManager.getAnimationIndex(color);
    if(index==-1)
    {
      Serial.println("Color not found");
    }
    else
    {
      last_user_animation=animation;
      animation = animationManager.getAnimation(index);
      Serial.print("Switched to ");
      Serial.println(color);
    } 
  }
  else if(command.startsWith("new"))
  {
    if (command.startsWith("new static "))
    {
        command = command.substring(11);
        int spacePos = command.indexOf(' ');
        if (spacePos == -1 || spacePos == 0 || spacePos >= command.length() - 1) {
            Serial.println("Error: Format is 'new static NAME COLOR'");
            return;
        }
        String name = command.substring(0, spacePos);
        String hex = command.substring(spacePos + 1);
        uint32_t rgb = (uint32_t)strtoul(hex.c_str(), NULL, 16);

        AnimationSetting* newSettings = animationManager.createSettingsStaticColor(rgb, 255, name);
        animationManager.createAnimation(newSettings);
        delete newSettings;
        Serial.println("New static color created!");
    }
    else if (command.startsWith("new blink "))
    {
      command = command.substring(10);
      int firstSpace = command.indexOf(' ');
      if (firstSpace == -1) {
          Serial.println("Error: Missing arguments. Usage: 'new blink NAME COLOR_ON COLOR_OFF TICKS'");
          return;
      }
      int secondSpace = command.indexOf(' ', firstSpace + 1);
      if (secondSpace == -1) {
          Serial.println("Error: Missing Color Off/Ticks.");
          return;
      }
      int thirdSpace = command.indexOf(' ', secondSpace + 1);
      if (thirdSpace == -1) {
          Serial.println("Error: Missing Ticks.");
          return;
      }

      String name = command.substring(0, firstSpace);
      
      String hexOn = command.substring(firstSpace + 1, secondSpace);
      uint32_t color_on = (uint32_t)strtoul(hexOn.c_str(), NULL, 16);

      String hexOff = command.substring(secondSpace + 1, thirdSpace);
      uint32_t color_off = (uint32_t)strtoul(hexOff.c_str(), NULL, 16);

      String ticksStr = command.substring(thirdSpace + 1);
      uint8_t cycle_ticks = (uint8_t)ticksStr.toInt();

      AnimationSetting* newSettings = animationManager.createSettingsBlink(color_on, color_off, cycle_ticks, 255, name);
      
      animationManager.createAnimation(newSettings);
      delete newSettings;
      Serial.println("New blink animation created!");
    }
    else if (command.startsWith("new fade"))
    {
      if(command == "new fade" || command == "new fade help")
      {
        Serial.println("'new fade NAME PALETTE SPEED DELTA'\nPalette: 0: Rainbow, 1: Party, 2: Ocean, 3: Forest, 4: Heat, 5: Lava, 6: Matrix\nDelta: Width");
        return;
      }
      command = command.substring(9);
      int firstSpace = command.indexOf(' ');
      if (firstSpace == -1) {
          Serial.println("Error: Missing arguments. Usage: 'new fade NAME PALETTE SPEED DELTA'");
          return;
      }
      int secondSpace = command.indexOf(' ', firstSpace + 1);
      if (secondSpace == -1) {
          Serial.println("Error: Missing Speed/Delta.");
          return;
      }
      int thirdSpace = command.indexOf(' ', secondSpace + 1);
      if (thirdSpace == -1) {
          Serial.println("Error: Missing Delta.");
          return;
      }

      String name = command.substring(0, firstSpace);
      
      String paletteStr = command.substring(firstSpace + 1, secondSpace);
      uint8_t palette = (uint8_t)paletteStr.toInt();

      String speedStr = command.substring(secondSpace + 1, thirdSpace);
      uint8_t speed = (uint8_t)speedStr.toInt();

      String deltaStr = command.substring(thirdSpace + 1);
      uint8_t delta = (uint8_t)deltaStr.toInt();

      AnimationSetting* newSettings = animationManager.createSettingsPalette(palette, speed, delta, 255,name);
      
      animationManager.createAnimation(newSettings);
      delete newSettings;
      Serial.println("New fade animation created!");
    }
    else
    {
        Serial.println("'new' can be used to create an animation. \nUsage:\n'new static NAME COLOR'\n'new blink NAME COLOR_ON COLOR_OFF TICKS'\n'new fade NAME PALETTE SPEED DELTA'");
        return;
    }
  }
  else if(command.startsWith("setting"))
  {
    String params = command.substring(7);
    params.trim();

    // Suche nach dem ersten Leerzeichen (Trennung Befehl / Name)
    int firstSpace = params.indexOf(' ');
    
    // Falls kein Parameter da ist
    if (params.length() == -1) {
      Serial.println("Usage:\nsetting set NAME INDEX DATA\nsetting show NAME INDEX\nsetting list NAME");
      return;
    }
    
    // Unterbefehl extrahieren (list, show, set)
    // Wenn kein Leerzeichen, ist der ganze String der Befehl (z.B. "list" fehlt der Name -> Fehlerabfang später)
    String subCommand = (firstSpace == -1) ? params : params.substring(0, firstSpace);
    
    // Rest des Strings (Argumente nach dem Unterbefehl)
    String args = (firstSpace == -1) ? "" : params.substring(firstSpace + 1);
    args.trim();

    if (subCommand == "list")
    {
      // Syntax: setting list NAME
      if(args.length() == 0) {
        Serial.println("Error: Missing Name. Usage: setting list NAME");
        return;
      }
      
      String animName = args;
      IAnimation* anim = animationManager.getAnimationByName(animName);
      
      if (anim != nullptr) {
        Serial.println("Available Settings for " + animName + ":");
        Serial.println(anim->GetAvailableSettings());
      } else {
        Serial.println("Animation '" + animName + "' not found.");
      }
    }
    else if (subCommand == "show")
    {
       // Syntax: setting show NAME INDEX
       int nameSpace = args.indexOf(' ');
       if (nameSpace == -1) {
         Serial.println("Error: Missing Index. Usage: setting show NAME INDEX");
         return;
       }
       
       String animName = args.substring(0, nameSpace);
       String indexStr = args.substring(nameSpace + 1);
       int index = indexStr.toInt();

       IAnimation* anim = animationManager.getAnimationByName(animName);
       if (anim != nullptr) {
         int value = anim->GetSetting(index);
         Serial.print("Setting ");
         Serial.print(index);
         Serial.print(" is: ");
         Serial.print(value);
         Serial.print(" (Hex: 0x");
         Serial.print(value, HEX);
         Serial.println(")");
       } else {
         Serial.println("Animation '" + animName + "' not found.");
       }
    }
    else if (subCommand == "set")
    {
      // Syntax: setting set NAME INDEX DATA
      int nameSpace = args.indexOf(' ');
      if (nameSpace == -1) {
        Serial.println("Error: Missing Index/Data. Usage: setting set NAME INDEX DATA");
        return;
      }
      
      String animName = args.substring(0, nameSpace);
      String rest = args.substring(nameSpace + 1);
      
      int indexSpace = rest.indexOf(' ');
      if (indexSpace == -1) {
        Serial.println("Error: Missing Data. Usage: setting set NAME INDEX DATA");
        return;
      }
      
      int index = rest.substring(0, indexSpace).toInt();
      String dataStr = rest.substring(indexSpace + 1);
      
      unsigned long value = strtoul(dataStr.c_str(), NULL, 0);

      IAnimation* anim = animationManager.getAnimationByName(animName);
      if (anim != nullptr) {
        if(anim->UpdateSetting(index, value)) {
          Serial.println("Setting updated.");
          int id = animationManager.getAnimationIndex(animName);
          animationManager.saveAnimationIndex(id);
          Serial.println("Saved to storage.");
        } else {
          Serial.println("Failed to update setting. Invalid Index or Value?");
        }
      } else {
        Serial.println("Animation '" + animName + "' not found.");
      }
    }
    else
    {
      Serial.println("Unknown command. Usage:\nsetting set NAME INDEX DATA\nsetting show NAME INDEX\nsetting list NAME");
    }
  }
  else if(command == "list")
  {
    int amount = animationManager.getAnimationCount();
    int i = 0;
    while(i < 100 && amount > 0)
    {
      IAnimation* ani = animationManager.getAnimation(i);
      i++;
      if(ani == nullptr)return;
      Serial.println(ani->GetName());
      amount--;
    }
  }
  else if(command == "toggle")
  {
    if(animation==animationManager.getAnimationByName("OFF"))
    {
      if(last_user_animation==nullptr || last_user_animation == animationManager.getAnimationByName("OFF"))animation=animationManager.getAnimationByName("WHITE");
      else animation = last_user_animation;
    }
    else
    {
      last_user_animation=animation;
      animation=animationManager.getAnimationByName("OFF");
    }
  }
  else if(command == "help")
  {
    Serial.print("help - list of commands\nset - set an Animation\nnew - create new animation\nlist - list all Animations\ntoggle - Turn light on/off\n");
  }
  else
  {
    Serial.println("Unkown Command. Type 'help' for a list of commands");
  }
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

void PublishData() {

  // ===== Collect Data =====

  pc_status = (analogRead(pc_state_pin)>COMPUTER_TRESHHOLD)?true:false;
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  // ===== Publish Data =====

  if (pc_status != last_pc_status) {
    last_pc_status = pc_status;
    mqttClient.beginMessage(TOPIC_PC_STATUS, true, 1); // topic, retained, qos
    mqttClient.print(pc_status);
    mqttClient.endMessage();
  }

  if(animation != last_animation)
  {
    last_animation = animation;
    mqttClient.beginMessage(TOPIC_RGB_STATUS, true, 1); // topic, retained, qos
    mqttClient.print(animation->GetName());
    mqttClient.endMessage();
  }

  if (isnan(temperature) || isnan(humidity)) {
    //Failed to read DHT Data, dont publish garbage data
    return;
  }

  if (temperature != last_temperature) {
    last_temperature = temperature;
    mqttClient.beginMessage(TOPIC_TEMP, false, 0); // topic, retained, qos
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

void FireAnimation()
{
  bool fireReverseDirection = false;
  random16_add_entropy( random16());
  // Array of temperature readings at each simulation cell
  static uint8_t heat[RGB_COUNT];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < RGB_COUNT; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / RGB_COUNT) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= RGB_COUNT - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < RGB_COUNT; j++) {
      CRGB color = HeatColor( heat[j]);
      int pixelnumber;
      if( fireReverseDirection ) {
        pixelnumber = (RGB_COUNT-1) - j;
      } else {
        pixelnumber = j;
      leds[pixelnumber] = color;
      }
    }
}