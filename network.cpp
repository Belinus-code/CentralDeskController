#include "network.h"

namespace Desk
{
    void Network::init()
    {
        IPAddress dns(8, 8, 8, 8);
        WiFi.setDNS(dns);
    }

    void Network::update()
    {
        uint32_t current_time = millis();
        if (current_time - last_wifi_check_ >= WIFI_CHECK_INTERVAL)
        {
            last_wifi_check_ = current_time;
            if (WiFi.status() != WL_CONNECTED)
            {
                if (was_connected_)
                {
                    Serial.println("Wifi Connection Lost. Reconnecting...");
                    was_connected_ = false;
                }
                WiFi.begin(ssid_, pass_);
            }
            else
            {
                if (!was_connected_)
                {
                    Serial.println("Connected to Wifi!");
                    Serial.print("IP-Adress: ");
                    Serial.println(WiFi.localIP());
                    was_connected_ = true;
                }
            }
        }
    }

    bool Network::isWifiConnected() const
    {
        return WiFi.status() == WL_CONNECTED;
    }
}