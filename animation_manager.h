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

        int getAnimationIndex(String name);
        IAnimation *getAnimation(int index);
        IAnimation *getAnimationByName(String name);

        int createAnimation(AnimationSetting *settings, bool save);
        int createAnimation(AnimationSetting *settings);
        void saveAnimation(AnimationSetting *settings);

        bool saveAnimationIndex(int id);
        void deleteAnimation(int id);
        int createAnimationsFromStorage();

        AnimationSetting *createSettingsStaticColor(unsigned long color, uint8_t brightness, String name);
        AnimationSetting *createSettingsBlink(unsigned long color_on, unsigned long color_off, uint8_t cycle_ticks, uint8_t brightness, String name);
        AnimationSetting *createSettingsPalette(uint8_t paletteID, uint8_t speed, uint8_t delta, uint8_t brightness, String name);

        int getAnimationCount();

    private:
        IAnimation *animations[100];
        CRGB *leds;
        int rgb_count = 0;
        Preferences _storage;
        int animation_count = 0;
    };
}
