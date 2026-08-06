#include "../logic/ac.h"

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
        if (is_timer_active_ && (time - timer_start_ >= timer_duration_) && is_ac_on_)
        {
            Serial.println("[AC] Timer reached & AC ON! Turning AC off.");
            is_timer_active_ = false;
            toggleAc(false);
        }
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
                is_ac_on_ = tempdiff > 3;
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
                    Serial.println("[AC] DHT Sensors failed for 10 Seconds. Turning them Off...");
                }
            }
        }
        else if (time - last_sensor_maintainance_ > AC_SRESET_DURATION)
        {
            io_->turnOnDTHSensors();
            is_reseting_ = false;
            last_sensor_poll_ = time; // This way the sensor has a full poll cycle time to wake up again.
            last_sensor_functioning_ = time;
            Serial.println("[AC] DHT Sensors turned for 10 Seconds. Turning them On Again...");
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

    bool AC::onButtonChange(bool button_state)
    {
        if(io_->getSwitch())return false;
        if(button_state)toggleAc();
        return true;
    }

    void AC::publishCall(MqttManager *mqtt)
    {
        if (abs(incase_temp_ - incase_temp_before_) >= 0.2)
        {
            incase_temp_before_ = incase_temp_;
            mqtt->publish(TOPIC_TEMP, String(incase_temp_), true, 0);
        }
        if (abs(incase_humid_ - incase_humid_before_) >= 0.2)
        {
            incase_humid_before_ = incase_humid_;
            mqtt->publish(TOPIC_HUMIDITY, String(incase_humid_), true, 0);
        }
        if abs((ac1_temp_ - ac1_temp_before_) >= 0.2)
        {
            ac1_temp_before_ = ac1_temp_;
            mqtt->publish(TOPIC_AC_TEMP1, String(ac1_temp_), true, 0);
        }
        if (abs(ac2_temp_ - ac2_temp_before_) >= 0.2)
        {
            ac2_temp_before_ = ac2_temp_;
            mqtt->publish(TOPIC_AC_TEMP2, String(ac2_temp_), true, 0);
        }
        if (abs(ac1_humid_ - ac1_humid_before_) >= 0.2)
        {
            ac1_humid_before_ = ac1_humid_;
            mqtt->publish(TOPIC_AC_HUMID1, String(ac1_humid_), true, 0);
        }
        if (abs(ac2_humid_ - ac2_humid_before_) >= 0.2)
        {
            ac2_humid_before_ = ac2_humid_;
            mqtt->publish(TOPIC_AC_HUMID2, String(ac2_humid_), true, 0);
        }
        if (current_water_ != last_water_)
        {
            last_water_ = current_water_;
            mqtt->publish(TOPIC_AC_WATER, String(current_water_), true, 1);
        }

        if (feedback_string_ != "")
        {
            mqtt->publish(TOPIC_AC_FEEDBACK, feedback_string_, true, 2);
            feedback_string_ = "";
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
        String upperCmd = cmd;
        upperCmd.toUpperCase();

        if (upperCmd.startsWith("TIMER "))
        {
            int minutes = upperCmd.substring(6).toInt();
            if (minutes > 0)
            {
                is_timer_active_ = true;
                timer_start_ = millis();
                timer_duration_ = minutes * 60000UL;
                Serial.println("[AC] Timer activated: Automatic deactivation in " + String(minutes) + " minutes.");
                feedback_string_ = ("[AC] Timer activated: Automatic deactivation in " + String(minutes) + " minutes.");
            }
            else
            {
                is_timer_active_ = false;
                Serial.println("[AC] Timer was cancelled.");
                feedback_string_ = ("[AC] Timer was cancelled.");
            }
            return;
        }
        uint32_t acc = getRcCodeFromString(cmd);
        if (acc == 0)
            return;
        if (acc == getRcCodeFromString("toggle"))
            lock_system_toggle_ = false;
        if (is_timer_active_)
        {
            is_timer_active_ = false;
            Serial.println("[AC] Timer was abborted.");
            feedback_string_ = "[AC] Timer was abborted.";
        }
        io_->sendIRMessage(acc);
    }

    uint32_t AC::getRcCodeFromString(const String &cmd)
    {
        String lowerCmd = cmd;
        lowerCmd.toLowerCase();

        if (lowerCmd == "on/off")
            return 0xFF00E710;
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