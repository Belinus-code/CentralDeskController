#pragma once
#include <stdint.h>
#include "Arduino.h"
#include "system_io.h"
#include "rgb.h"
#include "mqtt_manager.h"

namespace Desk
{

    class AC : public IMqttNode
    {
    public:
        void init(SystemIO *io, RGBController *rgb);
        void toggleAc(bool user_triggered = true);
        void processCommand(const String &cmd);
        void update();

        void publishCall(MqttManager *mqtt);
        void subscribeCall(MqttManager *mqtt);
        bool onMqttMessage(const String &topic, const String &payload);

        // Getter for all float sensor values
        float getIncaseTemp() const { return incase_temp_; }
        float getIncaseHumid() const { return incase_humid_; }
        float getBeforeAcTemp() const { return ac1_temp_; }
        float getAfterAcTemp() const { return ac2_temp_; }
        float getBeforeAcHumid() const { return ac1_humid_; }
        float getAfterAcHumid() const { return ac2_humid_; }

    private:
        SystemIO *io_ = nullptr;
        RGBController *rgb_ = nullptr;
        uint32_t last_sensor_maintainance_ = 0;
        uint32_t last_sensor_functioning_ = 0;
        bool is_reseting_ = false;
        bool is_alarming_ = false;
        uint32_t last_water_empty_ = 0;
        uint32_t last_sensor_poll_ = 0;
        bool lock_system_toggle_ = false;
        bool last_poll_failed = false;
        bool is_ac_on_ = false;

        bool current_water_ = false;
        bool last_water_ = false;

        float incase_temp_ = 0;
        float ac1_temp_ = 0;
        float ac2_temp_ = 0;
        float incase_humid_ = 0;
        float ac1_humid_ = 0;
        float ac2_humid_ = 0;

        float incase_temp_before_ = 0;
        float ac1_temp_before_ = 0;
        float ac2_temp_before_ = 0;
        float incase_humid_before_ = 0;
        float ac1_humid_before_ = 0;
        float ac2_humid_before_ = 0;

        // Timer for automatic AC Turn-Off
        bool is_timer_active_ = false;
        uint32_t timer_start_ = 0;
        uint32_t timer_duration_ = 0;

        uint32_t getRcCodeFromString(const String &cmd);
    };

}