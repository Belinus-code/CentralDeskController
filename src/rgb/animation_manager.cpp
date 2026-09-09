#include "../rgb/animation_manager.h"

namespace Desk
{
    AnimationManager::AnimationManager(struct CRGB *targetArray_, int rgbCount_, Preferences &storage)
        : leds_(targetArray_), rgb_count_(rgbCount_), storage_(storage)
    {
        // ...existing code...
    }

    void AnimationManager::begin()
    {
        memset(animations_, 0, sizeof(animations_));
        animation_count_ = createAnimationsFromStorage();
    }

    AnimationManager::~AnimationManager() {}

    int AnimationManager::getAnimationIndex(const String &name)
    {
        int i = 0;
        while (i < MAX_ANIMATIONS)
        {
            if (animations_[i] != nullptr && name == animations_[i]->getName())
                break;
            i++;
        }
        if (i < MAX_ANIMATIONS)
            return i;
        else
            return -1;
    }

    IAnimation *AnimationManager::getAnimation(int index)
    {
        if (index < 0 || index >= MAX_ANIMATIONS)
            return nullptr;
        else
            return animations_[index];
    }

    IAnimation *AnimationManager::getAnimationByName(const String &name)
    {
        int id = getAnimationIndex(name);
        if (id == -1)
            return nullptr;
        else
            return animations_[id];
    }

    int AnimationManager::createAnimation(AnimationSetting *settings, bool save)
    {
        if (settings == nullptr)
            return -1;

        int i = 0;
        while (i < MAX_ANIMATIONS && animations_[i] != nullptr)
            i++;
        if (i >= MAX_ANIMATIONS)
            return -2;

        IAnimation *animation = nullptr;
        if (settings->type == STATIC_COLOR)
        {
            animation = new StaticColorAnimation(leds_, rgb_count_);
        }
        else if (settings->type == BLINK)
        {
            animation = new BlinkAnimation(leds_, rgb_count_);
        }
        else if (settings->type == PALETTE)
        {
            animation = new PaletteAnimation(leds_, rgb_count_);
        }
        else
            return -3;

        settings->id = i;
        animation->applyAnimationSetting(settings);
        if (save)
            saveAnimation(settings);
        animations_[i] = animation;
        animation_count_++;
        return i;
    }

    int AnimationManager::createAnimation(AnimationSetting *settings)
    {
        return createAnimation(settings, true);
    }

    void AnimationManager::saveAnimation(AnimationSetting *settings)
    {
        storage_.begin(ANIMATION_STORAGE_NAMESPACE);
        String key = "a" + String(settings->id);
        storage_.putBytes(key.c_str(), settings, sizeof(AnimationSetting));
        storage_.end();
    }

    bool AnimationManager::saveAnimationIndex(int id)
    {
        if (id < 0 || id >= MAX_ANIMATIONS)
            return false;
        if (animations_[id] == nullptr)
            return false;

        AnimationSetting settings;
        animations_[id]->getAnimationSetting(&settings);
        saveAnimation(&settings);
        return true;
    }

    void AnimationManager::deleteAnimation(int id)
    {
        if (id < 0 || id >= MAX_ANIMATIONS)
            return;

        String key = "a" + String(id);
        storage_.begin(ANIMATION_STORAGE_NAMESPACE, false);
        storage_.remove(key.c_str());
        storage_.end();
        delete animations_[id];
        animations_[id] = nullptr;
        animation_count_--;
    }

    int AnimationManager::createAnimationsFromStorage()
    {
        String key = "";
        int found = 0;
        for (int i = 0; i < MAX_ANIMATIONS; i++)
        {
            key = "a" + String(i);
            AnimationSetting tempSettings;
            storage_.begin(ANIMATION_STORAGE_NAMESPACE, false);
            size_t len = storage_.getBytes(key.c_str(), &tempSettings, sizeof(AnimationSetting));
            if (len == sizeof(AnimationSetting))
            {
                createAnimation(&tempSettings, false);
                found++;
            }
            storage_.end();
        }
        return found;
    }

    AnimationSetting *AnimationManager::createSettingsStaticColor(unsigned long color, uint8_t brightness, const String &name)
    {
        if (name.length() > ANIMATION_NAME_MAX_LEN)
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

    AnimationSetting *AnimationManager::createSettingsBlink(unsigned long color_on, unsigned long color_off, uint8_t cycle_ticks, uint8_t brightness, const String &name)
    {
        if (name.length() > ANIMATION_NAME_MAX_LEN)
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

    AnimationSetting *AnimationManager::createSettingsPalette(uint8_t paletteID, uint8_t speed, uint8_t delta, uint8_t brightness, const String &name)
    {
        if (name.length() > ANIMATION_NAME_MAX_LEN)
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
        return animation_count_;
    }
}
