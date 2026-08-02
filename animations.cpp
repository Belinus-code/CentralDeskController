#include "animations.h"

namespace Desk
{
    String IAnimation::GetAvailableSettings()
    {
        return "No Settings Available";
    }

    bool IAnimation::UpdateSetting(int index, unsigned long value)
    {
        return false;
    }

    StaticColorAnimation::StaticColorAnimation(struct CRGB *targetArray, int RGBCount)
    {
        leds = targetArray;
        rgb_count = RGBCount;
    }

    void StaticColorAnimation::ResetSettings()
    {
        brightness = 0xFF;
        color = 0xFFFFFF;
        update_needed = true;
    }

    void StaticColorAnimation::RestartAnimation()
    {
        fill_solid(leds, rgb_count, color);
        FastLED.setBrightness(brightness);
    }

    bool StaticColorAnimation::Update(unsigned long tick)
    {
        if (update_needed)
        {
            RestartAnimation();
            update_needed = false;
            return true;
        }
        return false;
    }

    bool StaticColorAnimation::UpdateSetting(int index, unsigned long value)
    {
        switch (index)
        {
        case 0:
            if (value > 0xFFFFFF)
                return false;
            color = value;
            update_needed = true;
            break;

        case 1:
            if (value > 0xFF)
                return false;
            brightness = (uint8_t)value;
            update_needed = true;
            break;
        default:
            return false;
        }
        return true;
    }

    int StaticColorAnimation::GetSetting(int index)
    {
        switch (index)
        {
        case 0:
            return color;
        case 1:
            return brightness;
        default:
            return -1;
        }
    }

    String StaticColorAnimation::GetAvailableSettings()
    {
        return "0: Color\n1:Brightness";
    }

    String StaticColorAnimation::GetName()
    {
        return name;
    }

    void StaticColorAnimation::getAnimationSetting(AnimationSetting *settings)
    {
        settings->id = id;
        settings->type = STATIC_COLOR;

        memset(settings->name, 0, sizeof(settings->name));
        int len = name.length();
        if (len > 13)
            len = 13;
        memcpy(settings->name, name.c_str(), len);

        settings->data[0] = brightness;
        settings->data[1] = (uint8_t)(color & 0xFF);
        settings->data[2] = (uint8_t)((color >> 8) & 0xFF);
        settings->data[3] = (uint8_t)((color >> 16) & 0xFF);
    }

    void StaticColorAnimation::applyAnimationSetting(AnimationSetting *settings)
    {
        id = settings->id;
        name = String(settings->name, strnlen(settings->name, 13));
        brightness = settings->data[0];
        color = 0;
        color |= settings->data[1];
        color |= (((unsigned long)settings->data[2]) << 8);
        color |= (((unsigned long)settings->data[3]) << 16);
    }

    BlinkAnimation::BlinkAnimation(struct CRGB *targetArray, int RGBCount)
    {
        leds = targetArray;
        rgb_count = RGBCount;
    }

    void BlinkAnimation::ResetSettings()
    {
        brightness = 0xFF;
        color_on = 0xFFFFFF;
        color_off = 0;
        cycle_ticks = 10;
        update_needed = true;
    }

    void BlinkAnimation::RestartAnimation()
    {
        fill_solid(leds, rgb_count, color_off);
        FastLED.setBrightness(brightness);
    }

    bool BlinkAnimation::Update(unsigned long tick)
    {
        if (!((tick + int(cycle_ticks / 2)) % cycle_ticks))
        {
            fill_solid(leds, RGB_COUNT, CRGB::Red);
            return true;
        }
        else if (!(tick % cycle_ticks))
        {
            fill_solid(leds, RGB_COUNT, CRGB::Black);
            return true;
        }
        return false;
    }

    bool BlinkAnimation::UpdateSetting(int index, unsigned long value)
    {
        switch (index)
        {
        case 0:
            if (value > 0xFFFFFF)
                return false;
            color_on = value;
            update_needed = true;
            break;

        case 1:
            if (value > 0xFFFFFF)
                return false;
            color_off = value;
            update_needed = true;
            break;

        case 2:
            if (value > 0xFF)
                return false;
            cycle_ticks = value;
            update_needed = true;
            break;

        case 3:
            if (value > 0xFF)
                return false;
            brightness = value;
            update_needed = true;
            break;
        default:
            return false;
        }

        return true;
    }

    int BlinkAnimation::GetSetting(int index)
    {
        switch (index)
        {
        case 0:
            return color_on;
        case 1:
            return color_off;
        case 2:
            return cycle_ticks;
        case 3:
            return brightness;
        default:
            return -1;
        }
    }

    String BlinkAnimation::GetAvailableSettings()
    {
        return "0: Color On\n1: Color Off\n2: Cycle duration in ms/100 \n3:Brightness";
    }

    String BlinkAnimation::GetName()
    {
        return name;
    }

    void BlinkAnimation::getAnimationSetting(AnimationSetting *settings)
    {
        settings->id = id;
        settings->type = BLINK;

        memset(settings->name, 0, sizeof(settings->name));
        int len = name.length();
        if (len > 13)
            len = 13;
        memcpy(settings->name, name.c_str(), len);

        settings->data[0] = brightness;
        settings->data[1] = (uint8_t)(color_on & 0xFF);
        settings->data[2] = (uint8_t)((color_on >> 8) & 0xFF);
        settings->data[3] = (uint8_t)((color_on >> 16) & 0xFF);
        settings->data[4] = (uint8_t)(color_off & 0xFF);
        settings->data[5] = (uint8_t)((color_off >> 8) & 0xFF);
        settings->data[6] = (uint8_t)((color_off >> 16) & 0xFF);
        settings->data[7] = (uint8_t)cycle_ticks;
    }

    void BlinkAnimation::applyAnimationSetting(AnimationSetting *settings)
    {
        id = settings->id;
        name = String(settings->name, strnlen(settings->name, 13));
        brightness = settings->data[0];
        cycle_ticks = settings->data[7];
        color_on = 0;
        color_on |= settings->data[1];
        color_on |= (((unsigned long)settings->data[2]) << 8);
        color_on |= (((unsigned long)settings->data[3]) << 16);
        color_off = 0;
        color_off |= settings->data[4];
        color_off |= (((unsigned long)settings->data[5]) << 8);
        color_off |= (((unsigned long)settings->data[6]) << 16);
    }

    PaletteAnimation::PaletteAnimation(struct CRGB *targetArray, int RGBCount)
    {
        leds = targetArray;
        rgb_count = RGBCount;
        currentPalette = RainbowColors_p;
    }

    void PaletteAnimation::ResetSettings()
    {
        brightness = 255;
        speed = 10;
        delta = 3;
        paletteID = 0;
        update_needed = true;
    }

    void PaletteAnimation::RestartAnimation()
    {
        FastLED.setBrightness(brightness);
        ChangePalette(paletteID);
    }

    bool PaletteAnimation::Update(unsigned long tick)
    {
        uint8_t colorIndex = (uint8_t)((tick * speed) >> 2);

        CRGB color = ColorFromPalette(currentPalette, colorIndex, 255, LINEARBLEND);
        fill_solid(leds, rgb_count, color);

        if (FastLED.getBrightness() != brightness)
        {
            FastLED.setBrightness(brightness);
        }

        return true;
    }

    void PaletteAnimation::ChangePalette(uint8_t id)
    {
        paletteID = id;
        switch (id)
        {
        case 0:
            currentPalette = RainbowColors_p;
            break;
        case 1:
            currentPalette = PartyColors_p;
            break;
        case 2:
            currentPalette = OceanColors_p;
            break;
        case 3:
            currentPalette = ForestColors_p;
            break;
        case 4:
            currentPalette = HeatColors_p;
            break;
        case 5:
            currentPalette = LavaColors_p;
            break;
        case 6:
            currentPalette = CRGBPalette16(CRGB::Black, CRGB::Green, CRGB::Black, CRGB::DarkGreen);
            break;
        case 7:
            currentPalette = CRGBPalette16(
                CRGB(12, 12, 12),
                CRGB::Red,
                CRGB::Gold,
                CRGB::Red
            );
            break;
        default:
            currentPalette = RainbowColors_p;
            break;
        }
    }

    bool PaletteAnimation::UpdateSetting(int index, unsigned long value)
    {
        switch (index)
        {
        case 0:
            if (value > 255)
                return false;
            ChangePalette((uint8_t)value);
            break;
        case 1:
            if (value > 255)
                return false;
            speed = (uint8_t)value;
            break;
        case 2:
            if (value > 255)
                return false;
            delta = (uint8_t)value;
            break;
        case 3:
            if (value > 255)
                return false;
            brightness = (uint8_t)value;
            break;
        default:
            return false;
        }
        return true;
    }

    int PaletteAnimation::GetSetting(int index)
    {
        switch (index)
        {
        case 0:
            return paletteID;
        case 1:
            return speed;
        case 2:
            return delta;
        case 3:
            return brightness;
        default:
            return -1;
        }
    }

    String PaletteAnimation::GetAvailableSettings()
    {
        return "0: Palette ID (0=Rainbow, 1=Party, 2=Ocean, 3=Forest, 4=Heat, 5=Lava, 6=Matrix)\n1: Speed\n2: Delta\n3: Brightness";
    }

    String PaletteAnimation::GetName()
    {
        return name;
    }

    void PaletteAnimation::getAnimationSetting(AnimationSetting *settings)
    {
        settings->id = id;
        settings->type = PALETTE;

        memset(settings->name, 0, sizeof(settings->name));
        int len = name.length();
        if (len > 13)
            len = 13;
        memcpy(settings->name, name.c_str(), len);

        settings->data[0] = brightness;
        settings->data[1] = paletteID;
        settings->data[2] = speed;
        settings->data[3] = delta;
    }

    void PaletteAnimation::applyAnimationSetting(AnimationSetting *settings)
    {
        id = settings->id;
        name = String(settings->name, strnlen(settings->name, 13));

        brightness = settings->data[0];
        uint8_t newPalID = settings->data[1];
        speed = settings->data[2];
        delta = settings->data[3];

        if (speed == 0)
            speed = 1;
        ChangePalette(newPalID);
    }
}
