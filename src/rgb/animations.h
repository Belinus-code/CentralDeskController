#pragma once
#include <Preferences.h>
#include <FastLED.h>
#include "../../config.h"

#define STATIC_COLOR 1
#define BLINK 2
#define PALETTE 3

namespace Desk
{
    struct AnimationSetting
    {
        uint8_t id;
        uint8_t type;
        char name[ANIMATION_NAME_MAX_LEN + 1];
        uint8_t data[16];
    };

    class IAnimation
    {
    public:
        virtual ~IAnimation() = default;
        virtual void resetSettings() = 0;
        virtual void restartAnimation() = 0;
        virtual bool update(unsigned long tick) = 0;
        virtual String getAvailableSettings();
        virtual bool updateSetting(int index, unsigned long value);
        virtual int getSetting(int index) = 0;
        virtual String getName() = 0;
        virtual void getAnimationSetting(AnimationSetting *settings) = 0;
        virtual void applyAnimationSetting(AnimationSetting *settings) = 0;
    };

    class StaticColorAnimation : public IAnimation
    {
    public:
        StaticColorAnimation(struct CRGB *targetArray, int rgbCount);
        void resetSettings() override;
        void restartAnimation() override;
        bool update(unsigned long tick) override;
        bool updateSetting(int index, unsigned long value) override;
        int getSetting(int index) override;
        String getAvailableSettings() override;
        String getName() override;
        void getAnimationSetting(AnimationSetting *settings) override;
        void applyAnimationSetting(AnimationSetting *settings) override;

    private:
        uint8_t brightness_ = 0;
        unsigned long color_ = 0xFFFFFF;
        String name_ = "";
        CRGB *leds_;
        int rgb_count_ = 0;
        bool update_needed_ = false;
        uint8_t id_ = 0;
    };

    class BlinkAnimation : public IAnimation
    {
    public:
        BlinkAnimation(struct CRGB *targetArray, int rgbCount);
        void resetSettings() override;
        void restartAnimation() override;
        bool update(unsigned long tick) override;
        bool updateSetting(int index, unsigned long value) override;
        int getSetting(int index) override;
        String getAvailableSettings() override;
        String getName() override;
        void getAnimationSetting(AnimationSetting *settings) override;
        void applyAnimationSetting(AnimationSetting *settings) override;

    private:
        int id_ = 0;
        int brightness_ = 0;
        unsigned long color_on_ = 0xFFFFFF;
        unsigned long color_off_ = 0;
        uint8_t cycle_ticks_ = 10;
        String name_ = "";
        CRGB *leds_;
        int rgb_count_ = 0;
        bool update_needed_ = false;
    };

    class PaletteAnimation : public IAnimation
    {
    public:
        PaletteAnimation(struct CRGB *targetArray, int rgbCount);
        void resetSettings() override;
        void restartAnimation() override;
        bool update(unsigned long tick) override;
        void changePalette(uint8_t id);
        bool updateSetting(int index, unsigned long value) override;
        int getSetting(int index) override;
        String getAvailableSettings() override;
        String getName() override;
        void getAnimationSetting(AnimationSetting *settings) override;
        void applyAnimationSetting(AnimationSetting *settings) override;

    private:
        uint8_t id_ = 0;
        String name_ = "";
        CRGB *leds_;
        int rgb_count_;

        CRGBPalette16 current_palette_;

        uint8_t brightness_;
        uint8_t palette_id_;
        uint8_t speed_;
        uint8_t delta_;

        bool update_needed_ = false;
    };
}
