#pragma once
#include "../../config.h"
#include "Arduino.h"
#include <DHT.h>
#include "vector"

namespace Desk
{

  class IButtonListener
  {
  public:
    virtual bool onButtonChange(bool button_state) = 0;
  };

  class SystemIO
  {
  public:
    // Inits all Sensors and sets Pinmodes
    void init();
    void addButtonListener(IButtonListener *button_listener);

    bool getKey() const { return digitalRead(key_pin); }

    bool getSwitch() const { return digitalRead(switch_pin); }
    bool getButton() const { return digitalRead(button_pin); }

    void setRelais(bool state) { digitalWrite(relay_pin, state); }
    bool getPcState() const { return analogRead(pc_state_pin) > COMPUTER_TRESHHOLD; }
    int getPcRaw() { return analogRead(pc_state_pin); }

    void sendIRMessage(uint32_t value);

    bool wasKeyInterrupt();
    bool wasSwitchInterrupt();
    bool wasButtonInterrupt();

    float getIncaseTemp() { return dht_.readTemperature(); }
    float getIncaseHumid() { return dht_.readHumidity(); }
    float getBeforeAcTemp() { return dht_ac1_.readTemperature(); }
    float getAfterAcTemp() { return dht_ac2_.readTemperature(); }
    float getBeforeAcHumid() { return dht_ac1_.readHumidity(); }
    float getAfterAcHumid() { return dht_ac2_.readHumidity(); }
    bool areAcSensorsFunc();
    bool isWaterFull();
    int getWaterLevel() { return analogRead(water_sensor_pin); }
    void turnOffDHTSensors();
    void turnOnDTHSensors() { digitalWrite(dht_power_pin, HIGH); }

  private:
    std::vector<IButtonListener *> button_listeners_;
    static SystemIO *instance_;

    DHT dht_{dht_pin, DHTTYPE};
    DHT dht_ac1_{dht1_pin, DHTTYPE};
    DHT dht_ac2_{dht2_pin, DHTTYPE};

    volatile bool was_key_interrupt_ = false;
    volatile bool was_switch_interrupt_ = false;
    volatile bool was_button_interrupt_ = false;

    void key_isr() { was_key_interrupt_ = true; }
    void switch_isr() { was_switch_interrupt_ = true; }
    void button_isr();

    static void key_isr_static()
    {
      if (instance_)
        instance_->key_isr();
    }
    static void switch_isr_static()
    {
      if (instance_)
        instance_->switch_isr();
    }
    static void button_isr_static()
    {
      if (instance_)
        instance_->button_isr();
    }
  };
}