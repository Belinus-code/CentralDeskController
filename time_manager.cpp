#include "time_manager.h"

namespace Desk
{
    void TimeManager::init(WiFiUDP &udp, long offset_seconds)
    {
        timeClient_ = new NTPClient(udp, "pool.ntp.org", offset_seconds, 60000);
        timeClient_->begin();
    }

    void TimeManager::update()
    {
        if (timeClient_ != nullptr)
        {
            timeClient_->update();
        }
    }

    String TimeManager::getTimeString() const
    {
        if (timeClient_ != nullptr)
        {
            return timeClient_->getFormattedTime();
        }
        return "00:00:00";
    }

    uint32_t TimeManager::getUnixTime() const
    {
        if (timeClient_ != nullptr)
        {
            return timeClient_->getEpochTime();
        }
        return 0;
    }
}