#pragma once

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFiClient.h>

#include <functional>

#include "SettingsStore.h"

class MqttService {
public:
    using CommandCallback =
        std::function<void(const String& topic, const String& payload)>;

    void configure(const MqttSettings& settings, const String& device_id);
    void setCommandCallback(CommandCallback callback);
    bool loop(bool wifi_connected);
    bool publish(const String& topic, const String& payload,
                 bool retained = false);
    bool connected() const;
    bool enabled() const;
    String availabilityTopic() const;
    String stateTopic() const;
    String commandTopic(const String& command) const;
    String discoveryTopic(const String& component,
                          const String& object_id) const;
    String baseTopic() const;
    String discoveryPrefix() const;
    String deviceId() const;
    bool discoveryNeeded() const;
    void markDiscoveryPublished();
    void markDiscoveryDirty();

private:
    bool connect();
    void disconnect();
    String normalizedBaseTopic() const;
    void handleMessage(char* topic, uint8_t* payload, unsigned int length);

    WiFiClient wifi_client_;
    mutable PubSubClient client_ {wifi_client_};
    MqttSettings settings_;
    String device_id_;
    CommandCallback command_callback_;
    bool configured_ = false;
    bool was_connected_ = false;
    bool discovery_dirty_ = true;
    uint32_t last_connect_attempt_ms_ = 0;
};
