#include "mqtt_manager.h"

namespace Desk
{
    MqttManager *MqttManager::instance_ = nullptr;

    void MqttManager::init(WiFiClient &wifi_client, Network *network)
    {
        instance_ = this;
        network_ = network;
        mqtt_client_ = new MqttClient(wifi_client);
        mqtt_client_->onMessage(onMqttMessageStatic);
        mqtt_client_->setUsernamePassword(mqtt_user, mqtt_pass);
        nodes_.clear();
    }

    void MqttManager::registerNode(IMqttNode *node)
    {
        nodes_.push_back(node);
    }

    void MqttManager::publish(const String &topic, const String &payload, bool retained, uint8_t qos)
    {
        mqtt_client_->beginMessage(topic, retained, qos); // topic, retained, qos
        mqtt_client_->print(payload);
        mqtt_client_->endMessage();
    }

    void MqttManager::subscribe(const String &topic)
    {
        mqtt_client_->subscribe(topic.c_str(), 2);
    }

    void MqttManager::update()
    {
        uint32_t time = millis();
        if (mqtt_client_->connected())
        {
            if (!was_connected_before)
            {
                // If this is first update() since (re)connecting, subscribe to all topics
                was_connected_before = true;
                for (int i = 0; i < nodes_.size(); i++)
                {
                    nodes_[i]->subscribeCall(this);
                }
            }
            mqtt_client_->poll();
            if (time - last_publish_time_ > PUBLISH_INTERVALL)
            {
                last_publish_time_ = time;
                for (int i = 0; i < nodes_.size(); i++)
                {
                    nodes_[i]->publishCall(this);
                }
            }
        }
        else
        {
            if (was_connected_before)
            {
                was_connected_before = false;
                String clientId = "arduino-uno-r4-" + String(random(0xffff), HEX);
                mqtt_client_->setId(clientId);
            }

            // Only try reconnect if wifi is connected, otherwise useless waste of time
            if (network_->isWifiConnected() && time - last_connect_try_ >= MQTT_RECONNECT_INTERVALL)
            {
                last_connect_try_ = time;
                mqtt_client_->connect(broker, port);
            }
        }
    }

    void MqttManager::onMqttMessage(int messageSize)
    {
        String topic = mqtt_client_->messageTopic();
        String payload = "";
        payload.reserve(messageSize);
        while (mqtt_client_->available())
        {
            payload += (char)mqtt_client_->read();
        }

        for (int i = 0; i < nodes_.size(); i++)
        {
            if (nodes_[i]->onMqttMessage(topic, payload))
                break;
        }
    }
}