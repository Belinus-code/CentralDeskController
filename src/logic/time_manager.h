#pragma once
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Arduino.h>
#include <time.h>
#include <new>

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
        alignas(NTPClient) uint8_t ntp_buffer_[sizeof(NTPClient)];
        NTPClient *timeClient_ = nullptr;
        long base_offset_ = 3600;

        bool isEuropeanSummerTime(uint32_t epoch_time);
    };
}