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
        leds_ = targetArray;
        rgb_count_ = RGBCount;
    }

    void StaticColorAnimation::ResetSettings()
    {
        brightness_ = 0xFF;
        color_ = 0xFFFFFF;
        update_needed_ = true;
    }

    void StaticColorAnimation::RestartAnimation()
    {
        fill_solid(leds_, rgb_count_, color_);
        FastLED.setBrightness(brightness_);
    }

    bool StaticColorAnimation::Update(unsigned long tick)
    {
        if (update_needed_)
        {
            RestartAnimation();
            update_needed_ = false;
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
            color_ = value;
            update_needed_ = true;
            break;

        case 1:
            if (value > 0xFF)
                return false;
            brightness_ = (uint8_t)value;
            update_needed_ = true;
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
            return color_;
        case 1:
            return brightness_;
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
        return name_;
    }

    void StaticColorAnimation::getAnimationSetting(AnimationSetting *settings)
    {
        settings->id = id_;
        settings->type = STATIC_COLOR;

        memset(settings->name, 0, sizeof(settings->name));
        int len = name_.length();
        if (len > 13)
            len = 13;
        memcpy(settings->name, name_.c_str(), len);

        settings->data[0] = brightness_;
        settings->data[1] = (uint8_t)(color_ & 0xFF);
        settings->data[2] = (uint8_t)((color_ >> 8) & 0xFF);
        settings->data[3] = (uint8_t)((color_ >> 16) & 0xFF);
    }

    void StaticColorAnimation::applyAnimationSetting(AnimationSetting *settings)
    {
        id_ = settings->id;
        name_ = String(settings->name, strnlen(settings->name, 13));
        brightness_ = settings->data[0];
        color_ = 0;
        color_ |= settings->data[1];
        color_ |= (((unsigned long)settings->data[2]) << 8);
        color_ |= (((unsigned long)settings->data[3]) << 16);
    }

    BlinkAnimation::BlinkAnimation(struct CRGB *targetArray, int RGBCount)
    {
        leds_ = targetArray;
        rgb_count_ = RGBCount;
    }

    void BlinkAnimation::ResetSettings()
    {
        brightness_ = 0xFF;
        color_on_ = 0xFFFFFF;
        color_off_ = 0;
        cycle_ticks_ = 10;
        update_needed_ = true;
    }

    void BlinkAnimation::RestartAnimation()
    {
        fill_solid(leds_, rgb_count_, color_off_);
        FastLED.setBrightness(brightness_);
    }

    bool BlinkAnimation::Update(unsigned long tick)
    {
        if (!((tick + int(cycle_ticks_ / 2)) % cycle_ticks_))
        {
            fill_solid(leds_, rgb_count_, CRGB::Red);
            return true;
        }
        else if (!(tick % cycle_ticks_))
        {
            fill_solid(leds_, rgb_count_, CRGB::Black);
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
            color_on_ = value;
            update_needed_ = true;
            break;

        case 1:
            if (value > 0xFFFFFF)
                return false;
            color_off_ = value;
            update_needed_ = true;
            break;

        case 2:
            if (value > 0xFF)
                return false;
            cycle_ticks_ = value;
            update_needed_ = true;
            break;

        case 3:
            if (value > 0xFF)
                return false;
            brightness_ = value;
            update_needed_ = true;
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
            return color_on_;
        case 1:
            return color_off_;
        case 2:
            return cycle_ticks_;
        case 3:
            return brightness_;
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
        return name_;
    }

    void BlinkAnimation::getAnimationSetting(AnimationSetting *settings)
    {
        settings->id = id_;
        settings->type = BLINK;

        memset(settings->name, 0, sizeof(settings->name));
        int len = name_.length();
        if (len > 13)
            len = 13;
        memcpy(settings->name, name_.c_str(), len);

        settings->data[0] = brightness_;
        settings->data[1] = (uint8_t)(color_on_ & 0xFF);
        settings->data[2] = (uint8_t)((color_on_ >> 8) & 0xFF);
        settings->data[3] = (uint8_t)((color_on_ >> 16) & 0xFF);
        settings->data[4] = (uint8_t)(color_off_ & 0xFF);
        settings->data[5] = (uint8_t)((color_off_ >> 8) & 0xFF);
        settings->data[6] = (uint8_t)((color_off_ >> 16) & 0xFF);
        settings->data[7] = (uint8_t)cycle_ticks_;
    }

    void BlinkAnimation::applyAnimationSetting(AnimationSetting *settings)
    {
        id_ = settings->id;
        name_ = String(settings->name, strnlen(settings->name, 13));
        brightness_ = settings->data[0];
        cycle_ticks_ = settings->data[7];
        color_on_ = 0;
        color_on_ |= settings->data[1];
        color_on_ |= (((unsigned long)settings->data[2]) << 8);
        color_on_ |= (((unsigned long)settings->data[3]) << 16);
        color_off_ = 0;
        color_off_ |= settings->data[4];
        color_off_ |= (((unsigned long)settings->data[5]) << 8);
        color_off_ |= (((unsigned long)settings->data[6]) << 16);
    }

    PaletteAnimation::PaletteAnimation(struct CRGB *targetArray, int RGBCount)
    {
        leds_ = targetArray;
        rgb_count_ = RGBCount;
        current_palette_ = RainbowColors_p;
    }

    void PaletteAnimation::ResetSettings()
    {
        brightness_ = 255;
        speed_ = 10;
        delta_ = 3;
        palette_id_ = 0;
        update_needed_ = true;
    }

    void PaletteAnimation::RestartAnimation()
    {
        FastLED.setBrightness(brightness_);
        ChangePalette(palette_id_);
    }

    bool PaletteAnimation::Update(unsigned long tick)
    {
        uint8_t colorIndex = (uint8_t)((tick * speed_) >> 2);

        CRGB color = ColorFromPalette(current_palette_, colorIndex, 255, LINEARBLEND);
        fill_solid(leds_, rgb_count_, color);

        if (FastLED.getBrightness() != brightness_)
        {
            FastLED.setBrightness(brightness_);
        }

        return true;
    }

    void PaletteAnimation::ChangePalette(uint8_t id)
    {
        palette_id_ = id;
        switch (id)
        {
        case 0:
            current_palette_ = RainbowColors_p;
            break;
        case 1:
            current_palette_ = PartyColors_p;
            break;
        case 2:
            current_palette_ = OceanColors_p;
            break;
        case 3:
            current_palette_ = ForestColors_p;
            break;
        case 4:
            current_palette_ = HeatColors_p;
            break;
        case 5:
            current_palette_ = LavaColors_p;
            break;
        case 6:
            current_palette_ = CRGBPalette16(CRGB::Black, CRGB::Green, CRGB::Black, CRGB::DarkGreen);
            break;
        case 7:
            current_palette_ = CRGBPalette16(
                CRGB(12, 12, 12),
                CRGB::Red,
                CRGB::Gold,
                CRGB::Red);
            break;
        default:
            current_palette_ = RainbowColors_p;
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
            speed_ = (uint8_t)value;
            break;
        case 2:
            if (value > 255)
                return false;
            delta_ = (uint8_t)value;
            break;
        case 3:
            if (value > 255)
                return false;
            brightness_ = (uint8_t)value;
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
            return palette_id_;
        case 1:
            return speed_;
        case 2:
            return delta_;
        case 3:
            return brightness_;
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
        return name_;
    }

    void PaletteAnimation::getAnimationSetting(AnimationSetting *settings)
    {
        settings->id = id_;
        settings->type = PALETTE;

        memset(settings->name, 0, sizeof(settings->name));
        int len = name_.length();
        if (len > 13)
            len = 13;
        memcpy(settings->name, name_.c_str(), len);

        settings->data[0] = brightness_;
        settings->data[1] = palette_id_;
        settings->data[2] = speed_;
        settings->data[3] = delta_;
    }

    void PaletteAnimation::applyAnimationSetting(AnimationSetting *settings)
    {
        id_ = settings->id;
        name_ = String(settings->name, strnlen(settings->name, 13));

        brightness_ = settings->data[0];
        uint8_t new_pal_id = settings->data[1];
        speed_ = settings->data[2];
        delta_ = settings->data[3];

        if (speed_ == 0)
            speed_ = 1;
        ChangePalette(new_pal_id);
    }
}
