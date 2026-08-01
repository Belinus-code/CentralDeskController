#include "pc.h"

namespace Desk
{

    void PC::togglePC()
    {
        doing_start_ = millis();
        doing_duration_ = PC_TOGGLE_TIME;
        io_->setRelais(true);
    }

    void PC::resetPC()
    {
        doing_start_ = millis();
        doing_duration_ = PC_RESET_TIME;
        io_->setRelais(true);
    }

    void PC::onButtonChange(bool button_state)
    {
        if (button_state)
        {
            button_overwrite_ = true;
            io_->setRelais(true);
        }
        else
        {
            button_overwrite_ = false;
            io_->setRelais(false);
        }
    }

    void PC::update()
    {
        if (doing_duration_ != 0 && !button_overwrite_ && millis() - doing_start_ > doing_duration_)
        {
            doing_duration_ = 0;
            io_->setRelais(false);
        }
        current_pc_state_ = getPCState();
    }

    void PC::publishCall(MqttManager *mqtt)
    {
        if (current_pc_state_ != last_published_pc_state_)
        {
            mqtt->publish(TOPIC_PC_STATUS, current_pc_state_ ? "ON" : "OFF", true, 1);
            last_published_pc_state_ = current_pc_state_;
        }
    }

    void PC::subscribeCall(MqttManager *mqtt)
    {
        mqtt->subscribe(TOPIC_PC_CMD);
    }

    bool PC::onMqttMessage(const String &topic, const String &payload)
    {
        if (topic == TOPIC_AC_CMD)
        {
            if (payload == "TOGGLE")
                togglePC();
            else if (payload == "RESET")
                resetPC();
            return true;
        }
        return false;
    }

}