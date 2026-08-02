#include "animation_manager.h"

namespace Desk
{
    AnimationManager::AnimationManager(struct CRGB *targetArray_, int RGBCount_, Preferences &storage)
    {
        leds = targetArray_;
        rgb_count = RGBCount_;
        _storage = storage;
    }

    void AnimationManager::begin()
    {
        memset(animations, 0, sizeof(animations));
        animation_count = createAnimationsFromStorage();
    }

    AnimationManager::~AnimationManager() {}

    int AnimationManager::getAnimationIndex(String name)
    {
        int i = 0;
        while (i < 100)
        {
            if (animations[i] != nullptr && name == animations[i]->GetName())
                break;
            i++;
        }
        if (i < 100)
            return i;
        else
            return -1;
    }

    IAnimation *AnimationManager::getAnimation(int index)
    {
        if (index < 0 || index >= 100)
            return nullptr;
        else
            return animations[index];
    }

    IAnimation *AnimationManager::getAnimationByName(String name)
    {
        int id = getAnimationIndex(name);
        if (id == -1)
            return nullptr;
        else
            return animations[id];
    }

    int AnimationManager::createAnimation(AnimationSetting *settings, bool save)
    {
        if (settings == nullptr)
            return -1;

        int i = 0;
        while (i < 100 && animations[i] != nullptr)
            i++;
        if (i >= 100)
            return -2;

        IAnimation *animation = nullptr;
        if (settings->type == STATIC_COLOR)
        {
            animation = new StaticColorAnimation(leds, rgb_count);
        }
        else if (settings->type == BLINK)
        {
            animation = new BlinkAnimation(leds, rgb_count);
        }
        else if (settings->type == PALETTE)
        {
            animation = new PaletteAnimation(leds, rgb_count);
        }
        else
            return -3;

        settings->id = i;
        animation->applyAnimationSetting(settings);
        if (save)
            saveAnimation(settings);
        animations[i] = animation;
        animation_count++;
        return i;
    }

    int AnimationManager::createAnimation(AnimationSetting *settings)
    {
        return createAnimation(settings, true);
    }

    void AnimationManager::saveAnimation(AnimationSetting *settings)
    {
        _storage.begin("anim_data");
        String key = "a" + String(settings->id);
        _storage.putBytes(key.c_str(), settings, sizeof(AnimationSetting));
        _storage.end();
    }

    bool AnimationManager::saveAnimationIndex(int id)
    {
        if (id < 0 || id >= 100)
            return false;
        if (animations[id] == nullptr)
            return false;

        AnimationSetting settings;
        animations[id]->getAnimationSetting(&settings);
        saveAnimation(&settings);
        return true;
    }

    void AnimationManager::deleteAnimation(int id)
    {
        if (id < 0 || id >= 100)
            return;

        String key = "a" + String(id);
        _storage.begin("anim_data", false);
        _storage.remove(key.c_str());
        _storage.end();
        delete animations[id];
        animations[id] = nullptr;
        animation_count--;
    }

    int AnimationManager::createAnimationsFromStorage()
    {
        String key = "";
        int found = 0;
        for (int i = 0; i < 100; i++)
        {
            key = "a" + String(i);
            AnimationSetting tempSettings;
            _storage.begin("anim_data", false);
            size_t len = _storage.getBytes(key.c_str(), &tempSettings, sizeof(AnimationSetting));
            if (len == sizeof(AnimationSetting))
            {
                createAnimation(&tempSettings, false);
                found++;
            }
            _storage.end();
        }
        return found;
    }

    AnimationSetting *AnimationManager::createSettingsStaticColor(unsigned long color, uint8_t brightness, String name)
    {
        if (name.length() > 13)
            return nullptr;
        AnimationSetting *settings = new AnimationSetting();
        settings->type = STATIC_COLOR;
        memcpy(settings->name, name.c_str(), name.length());
        settings->data[0] = brightness;
        settings->data[1] = (uint8_t)(color & 0xFF);
        settings->data[2] = (uint8_t)((color >> 8) & 0xFF);
        settings->data[3] = (uint8_t)((color >> 16) & 0xFF);
        return settings;
    }

    AnimationSetting *AnimationManager::createSettingsBlink(unsigned long color_on, unsigned long color_off, uint8_t cycle_ticks, uint8_t brightness, String name)
    {
        if (name.length() > 13)
            return nullptr;
        AnimationSetting *settings = new AnimationSetting();
        settings->type = BLINK;
        memcpy(settings->name, name.c_str(), name.length());
        settings->data[0] = brightness;
        settings->data[1] = (uint8_t)(color_on & 0xFF);
        settings->data[2] = (uint8_t)((color_on >> 8) & 0xFF);
        settings->data[3] = (uint8_t)((color_on >> 16) & 0xFF);
        settings->data[4] = (uint8_t)(color_off & 0xFF);
        settings->data[5] = (uint8_t)((color_off >> 8) & 0xFF);
        settings->data[6] = (uint8_t)((color_off >> 16) & 0xFF);
        settings->data[7] = (uint8_t)cycle_ticks;
        return settings;
    }

    AnimationSetting *AnimationManager::createSettingsPalette(uint8_t paletteID, uint8_t speed, uint8_t delta, uint8_t brightness, String name)
    {
        if (name.length() > 13)
            return nullptr;

        AnimationSetting *settings = new AnimationSetting();
        settings->type = PALETTE;
        memset(settings->name, 0, sizeof(settings->name));
        memcpy(settings->name, name.c_str(), name.length());
        settings->data[0] = brightness;
        settings->data[1] = paletteID;
        settings->data[2] = speed;
        settings->data[3] = delta;
        return settings;
    }

    int AnimationManager::getAnimationCount()
    {
        return animation_count;
    }
}
