#pragma once
#include "Arduino.h"
#include "../../config.h"
#include <vector>
#include <stdint.h>
#include <MqttClient.h>
#include <WiFiS3.h>
#include "../network/network.h"
#include <new>

namespace Desk
{
    class MqttManager;

    class IMqttNode
    {
    public:
        // MqttManager calls this method on every node. If Node has smth to
        // publish, it can call publish() via the pointer to the MqttManager Object.
        virtual void publishCall(MqttManager *mqtt) = 0;

        // MqttMangager calls this method on every (re)connect to broker. So each Node
        // can subscribe to topics via the pointer to the MqttManager Object.
        virtual void subscribeCall(MqttManager *mqtt) = 0;

        // If MqttManager receives a Message, it passes it to all Nodes in order of
        // registering. If a Node returns true, all remaining Nodes are skipped.
        virtual bool onMqttMessage(const String &topic, const String &payload) = 0;
    };

    class MqttManager
    {
    public:
        void init(WiFiClient &wifi_client, Network *network);
        void registerNode(IMqttNode *node);
        void publish(const String &topic, const String &payload, bool retained, uint8_t qos);
        void subscribe(const String &topic);
        void update();

    private:
        static MqttManager *instance_;
        void onMqttMessage(int messageSize);
        static void onMqttMessageStatic(int messageSize)
        {
            if (instance_)
                instance_->onMqttMessage(messageSize);
        }
        std::vector<IMqttNode *> nodes_;
        uint32_t last_publish_time_ = 0;
        uint32_t last_connect_try_ = 0;

        alignas(MqttClient) uint8_t mqtt_buffer_[sizeof(MqttClient)];
        MqttClient *mqtt_client_ = nullptr;
        Network *network_ = nullptr;

        bool was_connected_before = false;
    };
}