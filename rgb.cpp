#include "rgb.h"

namespace Desk
{
    RGBController *RGBController::instance_ = nullptr;
    void RGBController::init(SystemIO *io, Preferences &prefs)
    {
        Serial.println("[RGB] Starting Init");
        io_ = io;
        instance_ = this;
        Serial.println("[RGB] Creating Animation Manager");
        animation_manager_ = new AnimationManager(leds_, RGB_COUNT, prefs);
        FastLED.addLeds<WS2812B, led_pin, GRB>(leds_, RGB_COUNT).setCorrection(TypicalLEDStrip);
        animation_manager_->begin();

        Serial.println("[RGB] Creating Animations");

        // Ensure that Animations needed by System are existing
        if (animation_manager_->getAnimationIndex("OFF") == -1)
        {
            AnimationSetting *newSettings = animation_manager_->createSettingsStaticColor(0, 255, "OFF");
            animation_manager_->createAnimation(newSettings);
            delete newSettings;
        }

        if (animation_manager_->getAnimationIndex("RED") == -1)
        {
            AnimationSetting *newSettings = animation_manager_->createSettingsStaticColor(0xFF0000, 255, "RED");
            animation_manager_->createAnimation(newSettings);
            delete newSettings;
        }

        if (animation_manager_->getAnimationIndex("SWITCH_BLINK") == -1)
        {
            AnimationSetting *newSettings = animation_manager_->createSettingsBlink(0xFF0000, 0, 16, 255, "SWITCH_BLINK");
            animation_manager_->createAnimation(newSettings);
            delete newSettings;
        }

        if (animation_manager_->getAnimationIndex("RAINBOW") == -1)
        {
            AnimationSetting *newSettings = animation_manager_->createSettingsPalette(0, 3, 0, 255, "RAINBOW");
            animation_manager_->createAnimation(newSettings);
            delete newSettings;
        }
        user_animation_ = animation_manager_->getAnimationByName("OFF");
        BeginRGBTimer(20);
        Serial.println("[RGB] Init finished");
    }

    void RGBController::update()
    {
        // Check if any Interrupts appeared:
        if (io_->wasKeyInterrupt())
        {
            if (io_->getKey())
                priority_animation_ = animation_manager_->getAnimationByName("RED");
            else
                priority_animation_ = nullptr;
        }
        if (io_->wasSwitchInterrupt())
        {
            if (io_->getSwitch())
                priority_animation_ = animation_manager_->getAnimationByName("SWITCH_BLINK");
            else
                priority_animation_ = io_->getKey() ? animation_manager_->getAnimationByName("RED") : nullptr;
        }
        if (io_->wasButtonInterrupt())
        {
            if (io_->getButton())
            {
                if (user_animation_ == animation_manager_->getAnimationByName("OFF"))
                {
                    if (last_user_animation_ == nullptr || last_user_animation_ == animation_manager_->getAnimationByName("OFF"))
                        user_animation_ = animation_manager_->getAnimationByName(DEFAULT_RGB_PRG);
                    else
                        user_animation_ = last_user_animation_;
                }
                else
                {
                    last_user_animation_ = user_animation_;
                    user_animation_ = animation_manager_->getAnimationByName("OFF");
                }
            }
        }

        // Now go on with updating animations
        if (rgb_brightness_ != last_rgb_brightness_)
        {
            constrain(rgb_brightness_, 0, 255);
            FastLED.setBrightness(rgb_brightness_);
            last_rgb_brightness_ = rgb_brightness_;
            flush_rgb_ = true;
        }
        if (flush_rgb_)
        {
            flush_rgb_ = false;
            FastLED.show();
        }
    }

    void RGBController::setBrightness(int val)
    { 
        rgb_brightness_ = constrain(val, 0, 255); 
    }

    void RGBController::adjustBrightness(int delta)
    { 
        setBrightness(rgb_brightness_ + delta); 
    }

    void RGBController::toggleUserAnimation()
    {
        if (user_animation_ == animation_manager_->getAnimationByName("OFF"))
        {
            if (last_user_animation_ == nullptr || last_user_animation_ == animation_manager_->getAnimationByName("OFF"))
            {
                Serial.println("[RGB] No Previous Animation! Switching to Default Animation");
                setUserAnimation(DEFAULT_RGB_PRG);
            }
            else
            {
                Serial.println("[RGB] Switching to Previous Animation");
                user_animation_ = last_user_animation_;
            }
        }
        else
        {
            last_user_animation_ = user_animation_;
            setUserAnimation("OFF");
        }
    }

    void RGBController::publishCall(MqttManager *mqtt)
    {
        if (last_published_animation_ != user_animation_)
        {
            last_published_animation_ = user_animation_;
            mqtt->publish(TOPIC_RGB_STATUS, user_animation_->GetName(), true, 1);
            mqtt->publish(TOPIC_RGB_STATUS_DIG, (user_animation_->GetName() == "OFF" ? "0" : "1"), true, 1);
        }
    }

    void RGBController::subscribeCall(MqttManager *mqtt)
    {
        mqtt->subscribe(TOPIC_RGB_CMD);
    }

    bool RGBController::onMqttMessage(const String &topic, const String &payload)
    {
        if (topic == TOPIC_RGB_CMD)
        {
            int index = animation_manager_->getAnimationIndex(payload);
            if (index != -1)
            {
                last_user_animation_ = user_animation_;
                user_animation_ = animation_manager_->getAnimation(index);
                return true;
            }
            else if(payload == "toggle")
            {
                toggleUserAnimation();
                return true;
            }
            return false;
        }
        return false;
    }

    bool RGBController::setUserAnimation(const String &str)
    {
        int index = animation_manager_->getAnimationIndex(str);
        if (index == -1)
            return false;
        last_user_animation_ = user_animation_;
        user_animation_ = animation_manager_->getAnimation(index);
        return true;
    }

    bool RGBController::setPriorityAnimation(const String &str)
    {
        int index = animation_manager_->getAnimationIndex(str);
        if (index == -1)
            return false;
        priority_animation_ = animation_manager_->getAnimation(index);
        return true;
    }

    void RGBController::clearPriorityAnimation()
    {
        if (io_->getSwitch())
            priority_animation_ = animation_manager_->getAnimationByName("SWITCH_BLINK");
        else
            priority_animation_ = io_->getKey() ? animation_manager_->getAnimationByName("RED") : nullptr;
    }
    void RGBController::handleTimer()
    {
        static unsigned long cycle_counter = 0;
        static IAnimation *local_last_animation = nullptr;

        noInterrupts();
        active_animation_ = (priority_animation_ == nullptr) ? user_animation_ : priority_animation_;
        if (active_animation_ == nullptr)
            return;
        if (active_animation_ != local_last_animation)
        {
            active_animation_->RestartAnimation();
            flush_rgb_ = true;
        }
        flush_rgb_ |= active_animation_->Update(cycle_counter);

        local_last_animation = active_animation_;
        cycle_counter++;
        interrupts();
    }

    void RGBController::RGBCallback(timer_callback_args_t __attribute((unused)) * p_args)
    {
        if (instance_ != nullptr)
            instance_->handleTimer();
    }

    bool RGBController::BeginRGBTimer(float rate)
    {
        uint8_t timer_type = GPT_TIMER;
  int8_t tindex = FspTimer::get_available_timer(timer_type);
  if (tindex < 0) {
    tindex = FspTimer::get_available_timer(timer_type, true);
  }
  if (tindex < 0) {
    return false;
  }

  FspTimer::force_use_of_pwm_reserved_timer();

  if (!rgb_timer_.begin(TIMER_MODE_PERIODIC, timer_type, tindex, rate, 0.0f, RGBCallback)) {
    return false;
  }

  if (!rgb_timer_.setup_overflow_irq()) {
    return false;
  }

  if (!rgb_timer_.open()) {
    return false;
  }

  if (!rgb_timer_.start()) {
    return false;
  }
  return true;
}

}