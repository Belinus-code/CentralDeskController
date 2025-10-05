#include <FastLED.h>
class IAnimation
{
    public:
        virtual void ResetAnimation()
        {
            return;
        }
        virtual void ResetSettings() = 0;
        virtual bool Update(unsigned long tick) = 0; //return true if LEDs needs to be flushed. 
        virtual String GetAvailableSettings()
        {
            return "No Settings Available";
        }
        virtual bool UpdateSetting(int index, unsigned long value)
        {
            return;
        }
        virtual String GetName() = 0;
        virtual ~IAnimation() {}

};

class StaticColorAnimation: public IAnimation
{
    public:
        StaticColorAnimation(String AnimationName, struct CRGB *targetArray, int RGBCount)
        {
            name = AnimationName;
            leds = targetArray;
            rgb_count = RGBCount;
            update_needed=true;
        }
        void ResetSettings() override
        {
            brightness = 0xFF;
            color = 0xFFFFFF;
        }

        bool Update(unsigned long tick) override
        {
            if(tick == 0 || update_needed) //tick gets set to zero only if animation(program) got changed
            {
                fill_solid(leds, rgb_count, color);
                FastLED.setBrightness(brightness);
                update_needed=false;
                return true
            }
            return false;
        }

        bool UpdateSetting(int index, unsigned long value) override
        {
            switch(index)
            {
                case 0:
                    if(value > 0xFFFFFF)return false;
                    color = value;
                    update_needed = true;
                    break;
                
                case 1:
                    if(value > 0xFF)return false;
                    brightness = value;
                    update_needed = true;
                    break;
                default:
                    return false;
            }
            return true;
        }

        String GetAvailableSettings() override
        {
            return "0: Color\n1:Brightness"
        }

        String GetName() override
        {
            return name;
        }
    
    private:
        int brightness = 0;
        unsigned long color = 0xFFFFFF;
        String name = "";
        CRGB *leds;
        int rgb_count = 0;
        bool update_needed = false;
};