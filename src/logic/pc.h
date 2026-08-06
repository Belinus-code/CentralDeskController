#pragma once
#include "../io/system_io.h"
#include "../network/mqtt_manager.h"

namespace Desk
{

    class PC : public IButtonListener, public IMqttNode
    {
    public:
        void init(SystemIO *io);

        // Update Method checks everything
        void update();

        // toggles pc (non-blocking)
        void togglePC();

        // hard-resets pc (non-blocking)
        void resetPC();

        // returns status of pc given of by its LED-Light
        bool getPCState() const {return current_pc_state_;}

        // handels button change isr
        bool onButtonChange(bool button_state);

        void publishCall(MqttManager *mqtt);
        void subscribeCall(MqttManager *mqtt);
        bool onMqttMessage(const String &topic, const String &payload);

    private:
        SystemIO *io_;
        bool button_overwrite_ = false;
        uint32_t doing_start_ = 0;    // millis-time of starting relay action
        uint32_t doing_duration_ = 0; // Duration of relay action. Zero means no active action
        uint32_t last_pc_state_differ = 0;

        bool current_pc_state_ = false;
        bool last_published_pc_state_ = false;
    };
}