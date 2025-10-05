class IAnimation
{
    public:
    virtual void ResetAnimation()
    {
        return;
    }
    virtual void ResetSettings() = 0;
    virtual void Update(unsigned long tick, char* pointer) = 0;
    virtual int AmountSettings() {
        return 0;
    }
    virtual String GetAvailableSettings()
    {
        return "No Settings Available";
    }
    virtual void UpdateSetting(int index, int value)
    {
        return;
    }
    virtual ~IAnimation() {}

};

class StaticRedAnimation: public IAnimation()
{
    public:
    void ResetSettings() override
    {
        brightness = 0xFF;
    }
    void Update(unsigned long tick, char* pointer)
    
    private:
    int brightness = 0xFF;
};