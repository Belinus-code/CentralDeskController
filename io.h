#pragma once
#include "config.h"
#include "Arduino.h"
#include <IRremote.hpp>
#include <DHT.h>

class IButtonListener
{
public:
  virtual void onButtonChange(bool button_state) = 0;
};

class IO
{
public:
  // Inits all Sensors and sets Pinmodes
  void init(IButtonListener *button_listener);

  bool getKey() const { return digitalRead(key_pin); }
  bool getSwitch() const { return digitalRead(switch_pin); }
  bool getButton() const { return digitalRead(button_pin); }

  void setRelais(bool state) { digitalWrite(relay_pin, state); }
  bool getPcState() const { return analogRead(pc_state_pin) > COMPUTER_TRESHHOLD; }

  void sendIRMessage(uint32_t value);

  bool wasKeyInterrupt();
  bool wasSwitchInterrupt();
  bool wasButtonInterrupt();

  float getIncaseTemp() const { return dht_.readTemperature(); }
  float getIncaseHumid() const { return dht_.readHumidity(); }
  float getBeforeAcTemp() const { return dht_ac1_.readTemperature(); }
  float getAfterAcTemp() const { return dht_ac2_.readTemperature(); }
  float getBeforeAcHumid() const { return dht_ac1_.readHumidity(); }
  float getAfterAcHumd() const { return dht_ac2_.readHumidity(); }
  bool isWaterFull();
  void turnOffDHTSensors();
  void turnOnDTHSensors() { digitalWrite(dht_power_pin, HIGH); }

private:
  IButtonListener *button_listener_ = nullptr;

  DHT dht_{dht_pin, DHTTYPE};
  DHT dht_ac1_{dht1_pin, DHTTYPE};
  DHT dht_ac2_{dht2_pin, DHTTYPE};

  volatile bool was_key_interrupt_ = false;
  volatile bool was_switch_interrupt_ = false;
  volatile bool was_button_interrupt_ = false;

  void key_isr() { was_key_interrupt_ = true; }
  void switch_isr() { was_switch_interrupt_ = true; }
  void button_isr();
};