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

const char* temp_config_topic = "homeassistant/device/mqtt_livingroom/skywalker_01/config";
const char* temp_state_topic  = "homeassistant/device/mqtt_livingroom/skywalker_01/state";

// Topics for Switch (LED)
const char* led_config_topic  = "homeassistant/device/mqtt_livingroom/skywalker_01/config";
const char* led_state_topic   = "homeassistant/device/mqtt_livingroom/skywalker_01/state";
const char* led_cmd_topic     = MQTT_CMD_TOPIC;
const char* device_id        = "esp32_controller_01";

namespace {
WiFiClient g_wifiClient;
PubSubClient g_mqttClient(g_wifiClient);
SkiMQTTCmdHandler g_cmdHandler = nullptr;

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

// Send Discovery Config Payloads to Home Assistant
void send_mqtt_discovery() {
  // 1. TEMPERATURE SENSOR DISCOVERY
  JsonDocument<400> tempDoc;
  tempDoc["name"] = "turned on";
  tempDoc["state_topic"] = "pump/timestamp_on";
  tempDoc["device_class"] = "timestamp";
  tempDoc["value_template"] = "{{ as_datetime(value) }}";
  tempDoc["unique_id"] = "hp_1231232_ts_on";

  JsonObject device = tempDoc.createNestedObject("device");
  device["name"] = "Heat pump";
  JsonArray identifiers = device.createNestedArray("identifiers");
  identifiers.add(device_id);

  char tempBuffer[400];
  serializeJson(tempDoc, tempBuffer);
  g_mqttClient.publish(temp_config_topic, tempBuffer, true); // Retain flag set to true
  Serial.println("Sent Temperature Discovery!");

//   // 2. SWITCH (LED) DISCOVERY
//   StaticJsonDocument<400> ledDoc;
//   ledDoc["name"] = "ESP32 Status LED";
//   ledDoc["state_topic"] = led_state_topic;
//   ledDoc["command_topic"] = led_cmd_topic;
//   ledDoc["unique_id"] = "esp32_mqtt_livingroom_led_01";
  
//   // Link to the same device mapping
//   JsonObject ledDevice = ledDoc.createNestedObject("device");
//   JsonArray ledIdentifiers = ledDevice.createNestedArray("identifiers");
//   ledIdentifiers.add(device_id);
//   ledDevice["name"] = "ESP32 Controller";
//   ledDevice["model"] = "ESP32 Dev Mode";
//   ledDevice["manufacturer"] = "DIY";

//   char ledBuffer[400];
//   serializeJson(ledDoc, ledBuffer);
//   g_mqttClient.publish(led_config_topic, ledBuffer, true); // Retain flag set to true
//   Serial.println("Sent LED Discovery!");
}

void ensureMQTTConnected() {
    while (!g_mqttClient.connected()) {
        Serial.print("Attempting MQTT connection...");
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

bool mqttIsConnected() {
    return g_mqttClient.connected();
}

const char* mqttCmdTopic() {
    return MQTT_CMD_TOPIC;
}

const char* mqttStatusTopic() {
    return MQTT_STATUS_TOPIC;
}