#include "MqttService.h"

namespace {
constexpr uint32_t kMqttReconnectIntervalMs = 30000;
constexpr uint16_t kMqttPacketSize = 2048;
}

void MqttService::configure(const MqttSettings& settings,
                            const String& device_id) {
    const bool changed =
        !configured_ || settings.enabled != settings_.enabled ||
        settings.host != settings_.host || settings.port != settings_.port ||
        settings.username != settings_.username ||
        settings.password != settings_.password ||
        settings.discovery_prefix != settings_.discovery_prefix ||
        settings.base_topic != settings_.base_topic ||
        device_id != device_id_;

    settings_ = settings;
    device_id_ = device_id;
    configured_ = true;
    if (settings_.discovery_prefix.isEmpty()) {
        settings_.discovery_prefix = "homeassistant";
    }
    if (changed) {
        disconnect();
        discovery_dirty_ = true;
    }
    client_.setBufferSize(kMqttPacketSize);
    client_.setCallback([this](char* topic, uint8_t* payload,
                               unsigned int length) {
        handleMessage(topic, payload, length);
    });
}

void MqttService::setCommandCallback(CommandCallback callback) {
    command_callback_ = callback;
}

bool MqttService::loop(bool wifi_connected) {
    if (!enabled() || !wifi_connected) {
        disconnect();
        return false;
    }

    if (!client_.connected()) {
        if (last_connect_attempt_ms_ != 0 &&
            millis() - last_connect_attempt_ms_ < kMqttReconnectIntervalMs) {
            return false;
        }
        last_connect_attempt_ms_ = millis();
        return connect();
    }

    client_.loop();
    return false;
}

bool MqttService::publish(const String& topic, const String& payload,
                          bool retained) {
    if (!client_.connected()) {
        return false;
    }
    return client_.publish(topic.c_str(), payload.c_str(), retained);
}

bool MqttService::connected() const {
    return client_.connected();
}

bool MqttService::enabled() const {
    return configured_ && settings_.enabled && !settings_.host.isEmpty();
}

String MqttService::availabilityTopic() const {
    return normalizedBaseTopic() + "/availability";
}

String MqttService::stateTopic() const {
    return normalizedBaseTopic() + "/state";
}

String MqttService::commandTopic(const String& command) const {
    return normalizedBaseTopic() + "/command/" + command;
}

String MqttService::discoveryTopic(const String& component,
                                   const String& object_id) const {
    return discoveryPrefix() + "/" + component + "/" + object_id + "/config";
}

String MqttService::baseTopic() const {
    return normalizedBaseTopic();
}

String MqttService::discoveryPrefix() const {
    return settings_.discovery_prefix.isEmpty() ? String("homeassistant")
                                                : settings_.discovery_prefix;
}

String MqttService::deviceId() const {
    return device_id_;
}

bool MqttService::discoveryNeeded() const {
    return discovery_dirty_;
}

void MqttService::markDiscoveryPublished() {
    discovery_dirty_ = false;
}

void MqttService::markDiscoveryDirty() {
    discovery_dirty_ = true;
}

bool MqttService::connect() {
    if (!enabled()) {
        return false;
    }
    client_.setServer(settings_.host.c_str(), settings_.port);
    const String client_id = device_id_ + "-client";
    const bool ok =
        settings_.username.isEmpty()
            ? client_.connect(client_id.c_str(), availabilityTopic().c_str(), 0,
                              true, "offline")
            : client_.connect(client_id.c_str(), settings_.username.c_str(),
                              settings_.password.c_str(),
                              availabilityTopic().c_str(), 0, true, "offline");
    if (!ok) {
        return false;
    }
    was_connected_ = true;
    client_.publish(availabilityTopic().c_str(), "online", true);
    const String commands = commandTopic("#");
    client_.subscribe(commands.c_str());
    discovery_dirty_ = true;
    Serial.printf("[mqtt] connected host=%s base=%s\n", settings_.host.c_str(),
                  normalizedBaseTopic().c_str());
    return true;
}

void MqttService::disconnect() {
    if (client_.connected()) {
        client_.publish(availabilityTopic().c_str(), "offline", true);
        client_.disconnect();
    }
    if (was_connected_) {
        Serial.println("[mqtt] disconnected");
    }
    was_connected_ = false;
}

String MqttService::normalizedBaseTopic() const {
    if (!settings_.base_topic.isEmpty()) {
        return settings_.base_topic;
    }
    return "m5paper-clock/" + device_id_;
}

void MqttService::handleMessage(char* topic, uint8_t* payload,
                                unsigned int length) {
    if (!command_callback_) {
        return;
    }
    String body;
    body.reserve(length);
    for (unsigned int i = 0; i < length; ++i) {
        body += static_cast<char>(payload[i]);
    }
    command_callback_(String(topic), body);
}
