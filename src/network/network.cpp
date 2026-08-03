#include "../network/network.h"

namespace Desk
{
    void Network::init()
    {
        Serial.println("[Network] Setting up Network Connection");
        IPAddress dns(8, 8, 8, 8);
        WiFi.setDNS(dns);

        // Call Update to connect to Wifi during Setup
        Serial.println("[Network] Initial Connect to Wifi...");
        update();
        Serial.println("[Network] Init finished");
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
                    Serial.println("[Network] Wifi Connection Lost. Reconnecting");
                    was_connected_ = false;
                }
                WiFi.begin(ssid_, pass_);
            }
            else
            {
                if (!was_connected_)
                {
                    Serial.println("[Network] Connected to Wifi!");
                    Serial.print("[Network] IP-Adress: ");
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