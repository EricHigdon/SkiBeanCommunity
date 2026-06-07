/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "../lib/SkiMQTT.h"

#include <PubSubClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>

#include "../lib/SkiMQTTConfig.h"

namespace {
const char* HA_DISCOVERY_PREFIX = "homeassistant";

WiFiClient g_wifiClient;
PubSubClient g_mqttClient(g_wifiClient);
SkiMQTTCmdHandler g_cmdHandler = nullptr;

String discoveryTopic(const char* component, const char* objectId) {
    char topic[128];
    snprintf(topic, sizeof(topic), "%s/%s/%s/config", HA_DISCOVERY_PREFIX, component, objectId);
    return String(topic);
}

template <size_t Capacity>
void addDeviceInfo(StaticJsonDocument<Capacity>& document) {
    JsonObject device = document.createNestedObject("device");
    JsonArray identifiers = device.createNestedArray("identifiers");
    identifiers.add(MQTT_DEVICE_ID);
    device["name"] = MQTT_DEVICE_NAME;
    device["manufacturer"] = MQTT_DEVICE_MANUFACTURER;
    device["model"] = MQTT_DEVICE_MODEL;
}

template <size_t Capacity>
void publishDiscoveryMessage(const String& topic, const StaticJsonDocument<Capacity>& document) {
    String payload;
    serializeJson(document, payload);
    g_mqttClient.publish(topic.c_str(), payload.c_str(), true);
}

void publishTemperatureDiscovery() {
    StaticJsonDocument<1024> document;
    document["name"] = "Roaster Temperature";
    document["unique_id"] = String(MQTT_DEVICE_ID) + "_temperature";
    document["state_topic"] = MQTT_TEMPERATURE_STATE_TOPIC;
    document["unit_of_measurement"] = "°C";
    document["device_class"] = "temperature";
    document["state_class"] = "measurement";
    addDeviceInfo(document);
    publishDiscoveryMessage(discoveryTopic("sensor", "temperature"), document);
}

void publishSetTemperatureDiscovery() {
    StaticJsonDocument<1024> document;
    document["name"] = "Set Temperature";
    document["unique_id"] = String(MQTT_DEVICE_ID) + "_set_temperature";
    document["state_topic"] = MQTT_SET_TEMPERATURE_STATE_TOPIC;
    document["unit_of_measurement"] = "°C";
    document["device_class"] = "temperature";
    document["state_class"] = "measurement";
    addDeviceInfo(document);
    publishDiscoveryMessage(discoveryTopic("sensor", "set_temperature"), document);
    mqttPublishSetTemperature(0); // Initialize set temperature state in MQTT
}

void publishHeatDiscovery() {
    StaticJsonDocument<1024> document;
    document["name"] = "Heat";
    document["unique_id"] = String(MQTT_DEVICE_ID) + "_heat";
    document["state_topic"] = MQTT_HEAT_STATE_TOPIC;
    document["unit_of_measurement"] = "%";
    document["device_class"] = "power_factor";
    document["state_class"] = "measurement";
    addDeviceInfo(document);
    publishDiscoveryMessage(discoveryTopic("sensor", "heat"), document);
    mqttPublishHeat(0); // Initialize heat state in MQTT
}

void publishVentDiscovery() {
    StaticJsonDocument<1024> document;
    document["name"] = "Vent";
    document["unique_id"] = String(MQTT_DEVICE_ID) + "_vent";
    document["state_topic"] = MQTT_VENT_STATE_TOPIC;
    document["unit_of_measurement"] = "%";
    document["device_class"] = "power_factor";
    document["state_class"] = "measurement";
    addDeviceInfo(document);
    publishDiscoveryMessage(discoveryTopic("sensor", "vent"), document);
    mqttPublishVent(0); // Initialize vent state in MQTT
}

void connectWiFi() {
    delay(10);

    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(WIFI_SSID);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.println("WiFi connected");
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    String message;
    for (unsigned int i = 0; i < length; i++) {
        message += static_cast<char>(payload[i]);
    }

    Serial.print("Message arrived on topic: ");
    Serial.print(topic);
    Serial.print(". Message: ");
    Serial.println(message);

    if (String(topic) == MQTT_CMD_TOPIC && g_cmdHandler != nullptr) {
        g_cmdHandler(message);
    }
}

void send_mqtt_discovery() {
    publishTemperatureDiscovery();
    publishSetTemperatureDiscovery();
    publishHeatDiscovery();
    publishVentDiscovery();
    Serial.println("Sent Home Assistant discovery payloads!");
}

void ensureMQTTConnected() {
    while (!g_mqttClient.connected()) {
        Serial.print("Attempting MQTT connection...");
        g_mqttClient.setBufferSize(1024);
        if (g_mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS)) {
            Serial.println("connected");
            g_mqttClient.subscribe(MQTT_CMD_TOPIC);
            send_mqtt_discovery(); // Send discovery messages upon connection
        } else {
            Serial.print("failed, rc=");
            Serial.print(g_mqttClient.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}
} // namespace

void initMQTT(SkiMQTTCmdHandler cmdHandler) {
    g_cmdHandler = cmdHandler;
    connectWiFi();
    g_mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    g_mqttClient.setCallback(mqttCallback);
    ensureMQTTConnected();
}

void mqttLoop() {
    ensureMQTTConnected();
    g_mqttClient.loop();
}

bool mqttPublish(const char* topic, const char* payload, bool retained) {
    ensureMQTTConnected();
    return g_mqttClient.publish(topic, payload, retained);
}

bool mqttPublish(const char* topic, const String& payload, bool retained) {
    return mqttPublish(topic, payload.c_str(), retained);
}

bool mqttPublishStatusOnline() {
    return mqttPublish(MQTT_STATUS_TOPIC, "online", false);
}

bool mqttPublishSetTemperature(double temperatureC) {
    String payload = String(temperatureC, 1);
    return mqttPublish(MQTT_SET_TEMPERATURE_STATE_TOPIC, payload, true);
}

bool mqttPublishTemperature(double temperatureC) {
    String payload = String(temperatureC, 1);
    return mqttPublish(MQTT_TEMPERATURE_STATE_TOPIC, payload, true);
}

bool mqttPublishHeat(double heat) {
    String payload = String(heat, 1);
    return mqttPublish(MQTT_HEAT_STATE_TOPIC, payload, true);
}

bool mqttPublishVent(double vent) {
    String payload = String(vent, 1);
    return mqttPublish(MQTT_VENT_STATE_TOPIC, payload, true);
}

bool mqttIsConnected() {
    return g_mqttClient.connected();
}

const char* mqttCmdTopic() {
    return MQTT_CMD_TOPIC;
}

const char* mqttStatusTopic() {
    return MQTT_STATUS_TOPIC;
}