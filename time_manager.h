#pragma once
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Arduino.h>

namespace Desk
{
    class TimeManager
    {
    public:
        void init(WiFiUDP &udp, long offset_seconds = 3600);
        void update();

        String getTimeString() const;

        uint32_t getUnixTime() const;

    private:
        NTPClient *timeClient_ = nullptr;
    };
}