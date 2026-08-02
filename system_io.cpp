#include "system_io.h"
#include <IRremote.hpp>

namespace Desk
{
  SystemIO *SystemIO::instance_ = nullptr;

  void SystemIO::init(IButtonListener *button_listener)
  {
    Serial.println("[SystemIO] Starting Init");
    instance_ = this;
    button_listener_ = button_listener;
    pinMode(key_pin, INPUT);
    pinMode(switch_pin, INPUT);
    pinMode(button_pin, INPUT);
    pinMode(pc_state_pin, INPUT);
    pinMode(relay_pin, OUTPUT);
    pinMode(water_sensor_pin, OUTPUT);
    pinMode(dht_power_pin, OUTPUT);

    IrSender.begin(ir_pin);

    dht_.begin();
    dht_ac1_.begin();
    dht_ac2_.begin();

    digitalWrite(water_sensor_pin, LOW);
    digitalWrite(dht_power_pin, HIGH);

    Serial.println("[SystemIO] Attaching Interrupts");

    instance_ = this;
    attachInterrupt(key_pin, key_isr_static, CHANGE);
    attachInterrupt(switch_pin, switch_isr_static, CHANGE);
    attachInterrupt(button_pin, button_isr_static, CHANGE);
    Serial.println("[SystemIO] Init finished");
  }

  void SystemIO::sendIRMessage(uint32_t value)
  {
    IrSender.sendNECRaw(value, 0);
  }

  bool SystemIO::isWaterFull()
  {
    pinMode(water_sensor_pin, INPUT_PULLUP);
    delay(2);
    bool acc = analogRead(water_sensor_pin) < WATERTANK_TRESHHOLD;
    pinMode(water_sensor_pin, OUTPUT);
    digitalWrite(water_sensor_pin, LOW);
    return acc;
  }

  void SystemIO::turnOffDHTSensors()
  {
    digitalWrite(dht_power_pin, LOW);
    pinMode(dht1_pin, OUTPUT);
    digitalWrite(dht1_pin, LOW);
    pinMode(dht2_pin, OUTPUT);
    digitalWrite(dht2_pin, LOW);
  }

  bool SystemIO::wasKeyInterrupt()
  {
    if (was_key_interrupt_)
    {
      was_key_interrupt_ = false;
      return true;
    }
    return false;
  }
  bool SystemIO::wasSwitchInterrupt()
  {
    if (was_switch_interrupt_)
    {
      was_switch_interrupt_ = false;
      return true;
    }
    return false;
  }
  bool SystemIO::wasButtonInterrupt()
  {
    if (was_button_interrupt_)
    {
      was_button_interrupt_ = false;
      return true;
    }
    return false;
  }

  // Button isr has two jobs. If switch is on (due to electrical reasons for this key has to be on two)
  // Then Button press directly controlls relay
  // Else, set a flag for later rgb controll
  void SystemIO::button_isr()
  {
    if (getSwitch())
      button_listener_->onButtonChange(getButton());
    else
      was_button_interrupt_ = true;
  }

  bool SystemIO::areAcSensorsFunc()
  {
    if (isnan(dht_ac1_.readTemperature()))
      return false;
    if (isnan(dht_ac2_.readTemperature()))
      return false;
    return true;
  }
}