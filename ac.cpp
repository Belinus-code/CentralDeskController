#include "ac.h"

namespace Desk
{

    void AC::init(SystemIO *io, RGBController *rgb)
    {
        io_ = io;
        rgb_ = rgb;
    }

    void AC::update()
    {
        uint32_t time = millis();
        current_water_ = io_->isWaterFull();
        if (!current_water_)
        {
            last_water_empty_ = time;
            if (is_alarming_)
            {
                is_alarming_ = false;
                rgb_->clearPriorityAnimation();
            }
        }
        if (!is_reseting_)
        {
            if (time - last_sensor_poll_ > AC_SENSOR_POLL)
            {
                last_sensor_poll_ = time;
                float temp1 = io_->getBeforeAcTemp();
                float temp2 = io_->getAfterAcTemp();
                float humid1 = io_->getBeforeAcHumid();
                float humid2 = io_->getAfterAcHumid();
                incase_temp_ = io_->getIncaseTemp();
                incase_humid_ = io_->getIncaseHumid();
                if (!isnan(temp1) && !isnan(temp2) && !isnan(humid1) && !isnan(humid2))
                {
                    ac1_temp_ = temp1;
                    ac2_temp_ = temp2;
                    ac1_humid_ = humid1;
                    ac2_humid_ = humid2;
                    last_poll_failed = false;
                }
                else
                    last_poll_failed = true;
            }
            if (!last_poll_failed)
            {
                last_sensor_functioning_ = time;
                float tempdiff = ac1_temp_ - ac2_temp_;
                if (tempdiff > 3 && current_water_)
                {
                    if (!is_alarming_)
                    {
                        is_alarming_ = true;
                        rgb_->setPriorityAnimation("SWITCH_BLINK");
                    }
                    else if (is_alarming_ && time - last_water_empty_ > AC_WATER_FULL_TIME)
                    {
                        toggleAc(false);
                        lock_system_toggle_ = true;
                    }
                }
            }
            else
            {
                if (time - last_sensor_functioning_ > AC_SOFFLINE_TIME)
                {
                    io_->turnOffDHTSensors();
                    is_reseting_ = true;
                    last_sensor_maintainance_ = time;
                }
            }
        }
        else if (time - last_sensor_maintainance_ > AC_SRESET_DURATION)
        {
            io_->turnOnDTHSensors();
            is_reseting_ = false;
        }
    }

    void AC::toggleAc(bool user_triggered)
    {
        if (user_triggered)
        {
            io_->sendIRMessage(getRcCodeFromString("toggle"));
            lock_system_toggle_ = false;
        }
        else if (!lock_system_toggle_)
        {
            io_->sendIRMessage(getRcCodeFromString("toggle"));
        }
    }

    void AC::publishCall(MqttManager *mqtt)
    {
        if (incase_temp_ != incase_temp_before_)
        {
            incase_temp_before_ = incase_temp_;
            mqtt->publish(TOPIC_TEMP, String(incase_temp_), true, 0);
        }
        if (incase_humid_ != incase_humid_before_)
        {
            incase_humid_before_ = incase_humid_;
            mqtt->publish(TOPIC_HUMIDITY, String(incase_humid_), true, 0);
        }
        if (ac1_temp_ != ac1_temp_before_)
        {
            ac1_temp_before_ = ac1_temp_;
            mqtt->publish(TOPIC_AC_TEMP1, String(ac1_temp_), true, 0);
        }
        if (ac2_temp_ != ac2_temp_before_)
        {
            ac2_temp_before_ = ac2_temp_;
            mqtt->publish(TOPIC_AC_TEMP2, String(ac2_temp_), true, 0);
        }
        if (ac1_humid_ != ac1_humid_before_)
        {
            ac1_humid_before_ = ac1_humid_;
            mqtt->publish(TOPIC_AC_HUMID1, String(ac1_humid_), true, 0);
        }
        if (ac2_humid_ != ac2_humid_before_)
        {
            ac2_humid_before_ = ac2_humid_;
            mqtt->publish(TOPIC_AC_HUMID2, String(ac2_humid_), true, 0);
        }
        if (current_water_ != last_water_)
        {
            last_water_ = current_water_;
            mqtt->publish(TOPIC_AC_WATER, String(current_water_), true, 1);
        }
    }

    void AC::subscribeCall(MqttManager *mqtt)
    {
        mqtt->subscribe(TOPIC_AC_CMD);
    }

    bool AC::onMqttMessage(const String &topic, const String &payload)
    {
        if (topic == TOPIC_AC_CMD)
        {
            processCommand(payload);
            return true;
        }
        return false;
    }

    // This is ONLY to be called from user action directly!
    void AC::processCommand(const String &cmd)
    {
        uint32_t acc = getRcCodeFromString(cmd);
        if (acc == 0)
            return;
        if (acc == getRcCodeFromString("toggle"))
            lock_system_toggle_ = false;
        io_->sendIRMessage(acc);
    }

    uint32_t AC::getRcCodeFromString(const String &cmd)
    {
        String lowerCmd = cmd;
        lowerCmd.toLowerCase();

        if (lowerCmd == "on")
            return 0xFF00E710;
        if (lowerCmd == "toggle")
            return 0xFF00E710;
        if (lowerCmd == "off")
            return 0xFF00E710;
        if (lowerCmd == "cool")
            return 0xEB14E710;
        if (lowerCmd == "dry")
            return 0xF30CE710;
        if (lowerCmd == "fan")
            return 0xF708E710;
        if (lowerCmd == "sleep")
            return 0xFA05E710;
        if (lowerCmd == "up")
            return 0xEA15E710;
        if (lowerCmd == "down")
            return 0xF20DE710;
        if (lowerCmd == "high")
            return 0xE916E710;
        if (lowerCmd == "low")
            return 0xF50AE710;

        return 0;
    }

}