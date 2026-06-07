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

#include "../lib/SkiMQTTConfig.h"

namespace {
const char* HA_DISCOVERY_TOPIC = "homeassistant/switch/device02/relay1/config";
const String HA_DEVICE_ID = "skywalker_01";

WiFiClient g_wifiClient;
PubSubClient g_mqttClient(g_wifiClient);
SkiMQTTCmdHandler g_cmdHandler = nullptr;

void publishHomeAssistantDiscovery() {
    String device_str = String("{\"name\":\"Skywalker Roaster\"}");
    String payloadStr = String("{\"name\":\"Living Room Light\",\"unique_id\":\"device02_relay1\",\"state_topic\":\"device02\/relay\/1\/state\",\"command_topic\":\"device02\/relay\/1\/set\",\"payload_on\":\"ON\",\"payload_off\":\"OFF\",\"device\":" + device_str + "}");

    if (!g_mqttClient.publish(HA_DISCOVERY_TOPIC, payloadStr.c_str(), true)) {
        ESP_LOGW("SkiMQTT", "Failed to publish Home Assistant discovery payload.");
    }
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
    Serial.println("\nWiFi connected");
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

void ensureMQTTConnected() {
    while (!g_mqttClient.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (g_mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS)) {
            Serial.println("connected");
            g_mqttClient.subscribe(MQTT_CMD_TOPIC);
            publishHomeAssistantDiscovery();
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

bool mqttIsConnected() {
    return g_mqttClient.connected();
}

const char* mqttCmdTopic() {
    return MQTT_CMD_TOPIC;
}

const char* mqttStatusTopic() {
    return MQTT_STATUS_TOPIC;
}