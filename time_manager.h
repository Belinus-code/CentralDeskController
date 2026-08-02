#pragma once
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Arduino.h>
#include <time.h>

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
        long base_offset_ = 3600;

        bool isEuropeanSummerTime(uint32_t epoch_time);
    };
}