#pragma once
#include <FastLED.h>
#include <FspTimer.h>
#include <new>
#include "animation_manager.h"
#include <Preferences.h>
#include "system_io.h"
#include "Arduino.h"
#include "config.h"
#include "mqtt_manager.h"

namespace Desk
{

    class RGBController : public IMqttNode
    {
    public:
        // Use init() instead of Constructor, because Animation relys on other objects.
        void init(SystemIO *io, Preferences &prefs);

        void update();

        bool setUserAnimation(const String &str);
        bool setPriorityAnimation(const String &str);
        void clearPriorityAnimation();
        IAnimation *getUserAnimation() const { return user_animation_; }
        void setBrightness(int val);
        void adjustBrightness(int delta);
        void toggleUserAnimation();

        void publishCall(MqttManager *mqtt);
        void subscribeCall(MqttManager *mqtt);
        bool onMqttMessage(const String &topic, const String &payload);

        AnimationManager *getAnimMan() { return animation_manager_; }

    private:
        // Holds the currently displayed Animation
        IAnimation *active_animation_ = nullptr;
        // Holds the Animation the System needs to display, overwrites user animation.
        // If Null User animation is used
        IAnimation *priority_animation_ = nullptr;
        // Animation User wants to display now
        IAnimation *user_animation_ = nullptr;
        // Animation before User switched to another Animation
        IAnimation *last_user_animation_ = nullptr;

        // Holds the last published user animation
        IAnimation *last_published_animation_ = nullptr;

        int rgb_brightness_ = 0xFF;
        int last_rgb_brightness_ = 0xFF;
        bool flush_rgb_ = false;

        CRGB leds_[RGB_COUNT];
        alignas(AnimationManager) uint8_t animation_manager_buffer_[sizeof(AnimationManager)];
        AnimationManager *animation_manager_ = nullptr;
        SystemIO *io_ = nullptr;
        FspTimer rgb_timer_;
        static RGBController *instance_;

        static void RGBCallback(timer_callback_args_t __attribute((unused)) * p_args);
        void handleTimer();
        // Start timer for accurate animation updating
        bool BeginRGBTimer(float rate);
    };

}