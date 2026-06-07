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

#pragma once

// WiFi Settings
static const char* WIFI_SSID = "HigdonNet";
static const char* WIFI_PASSWORD = "eriQ2930";

// MQTT Broker Settings
static const char* MQTT_SERVER = "10.53.55.64";
static const int MQTT_PORT = 1883;
static const char* MQTT_USER = "eric.s.higdon@gmail.com";
static const char* MQTT_PASS = "eriQ2930";
static const char* MQTT_CLIENT_ID = "ESP32_Backyard_Client";

// Home Assistant Device Identity
static const char* MQTT_DEVICE_ID = "skywalker_01";
static const char* MQTT_DEVICE_NAME = "Skywalker Roaster";
static const char* MQTT_DEVICE_MANUFACTURER = "HiBean Community";
static const char* MQTT_DEVICE_MODEL = "Skywalker v1";

// Default Topics
static const char* MQTT_CMD_TOPIC = "skywalker/cmd";
static const char* MQTT_STATUS_TOPIC = "skywalker/status";
static const char* MQTT_SET_TEMPERATURE_STATE_TOPIC = "skywalker/set_temperature/state";
static const char* MQTT_TEMPERATURE_STATE_TOPIC = "skywalker/temperature/state";
static const char* MQTT_HEAT_STATE_TOPIC = "skywalker/heat/state";
static const char* MQTT_VENT_STATE_TOPIC = "skywalker/vent/state";