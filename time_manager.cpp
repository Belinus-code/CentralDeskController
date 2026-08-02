#include "time_manager.h"

namespace Desk
{
    void TimeManager::init(WiFiUDP &udp, long offset_seconds)
    {
        Serial.println("[TimeManager] Init started. Creating NTPClient.");
        base_offset_ = offset_seconds;
        timeClient_ = new (ntp_buffer_) NTPClient(udp, "pool.ntp.org", offset_seconds, 60000);
        timeClient_->begin();
        Serial.println("[TimeManager] NTPClient created and initialized.");
    }

    void TimeManager::update()
    {
        if (timeClient_ != nullptr)
        {
            timeClient_->update();
            uint32_t current_epoch = timeClient_->getEpochTime();
            if (current_epoch > 100000)
            {
                if (isEuropeanSummerTime(current_epoch))
                {
                    timeClient_->setTimeOffset(base_offset_ + 3600);
                }
                else
                {
                    timeClient_->setTimeOffset(base_offset_);
                }
            }
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

    bool TimeManager::isEuropeanSummerTime(uint32_t epoch)
    {
        time_t t = epoch;
        struct tm *ptm = gmtime(&t);

        int month = ptm->tm_mon + 1;
        int day = ptm->tm_mday;
        int dow = ptm->tm_wday;
        int hr = ptm->tm_hour;

        if (month > 3 && month < 10)
            return true;
        if (month < 3 || month > 10)
            return false;

        int previousSunday = day - dow;
        if (month == 3)
        {
            return (previousSunday >= 25) ? (dow == 0 ? hr >= 1 : true) : false;
        }
        if (month == 10)
        {
            return (previousSunday >= 25) ? (dow == 0 ? hr < 1 : false) : true;
        }

        return false;
    }
}