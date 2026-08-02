#pragma once
#include <Preferences.h>
#include "animations.h"

namespace Desk
{
    class AnimationManager
    {
    public:
        AnimationManager(struct CRGB *targetArray_, int RGBCount_, Preferences &storage);
        void begin();
        ~AnimationManager();

        int getAnimationIndex(const String &name);
        IAnimation *getAnimation(int index);
        IAnimation *getAnimationByName(const String &name);

        int createAnimation(AnimationSetting *settings, bool save);
        int createAnimation(AnimationSetting *settings);
        void saveAnimation(AnimationSetting *settings);

        bool saveAnimationIndex(int id);
        void deleteAnimation(int id);
        int createAnimationsFromStorage();

        AnimationSetting *createSettingsStaticColor(unsigned long color, uint8_t brightness, const String &name);
        AnimationSetting *createSettingsBlink(unsigned long color_on, unsigned long color_off, uint8_t cycle_ticks, uint8_t brightness, const String &name);
        AnimationSetting *createSettingsPalette(uint8_t paletteID, uint8_t speed, uint8_t delta, uint8_t brightness, const String &name);

        int getAnimationCount();

    private:
        IAnimation *animations_[100];
        CRGB *leds_;
        int rgb_count_ = 0;
        Preferences storage_;
        int animation_count_ = 0;
    };
}
