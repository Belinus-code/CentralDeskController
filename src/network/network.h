#pragma once
#include <WiFiS3.h>
#include <WiFiUdp.h>
#include <Arduino.h>
#include <WDT.h>
#include "../../config.h"

namespace Desk
{
    class Network
    {
    public:
        void init();

        void update();

        bool isWifiConnected() const;
        String getIpString() { return WiFi.localIP().toString(); }
        WiFiClient &getWifiClient() { return wifiClient_; }
        WiFiUDP &getWifiUDP() { return udp_; }

    private:
        const char *ssid_ = WIFI_SSID;
        const char *pass_ = WIFI_PASS;

        uint32_t last_wifi_check_ = 0;

        bool was_connected_ = false;

        // Needed for Mqtt
        WiFiClient wifiClient_;
        // Needed for ntp
        WiFiUDP udp_;
    };
}