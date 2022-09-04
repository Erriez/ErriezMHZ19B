/*
 * MIT License
 *
 * Copyright (c) 2020-2022 Erriez
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*!
 * \brief Erriez MHZ19B CO2 sensor MQTT PubSubClient and ESP8266 Multi WiFi
 * \details
 *      Source:         https://github.com/Erriez/ErriezMHZ19B
 *      Documentation:  https://erriez.github.io/ErriezMHZ19B
 */

#include <ESP8266WiFi.h>        // https://github.com/esp8266/Arduino
#include <ESP8266WiFiMulti.h>   // https://github.com/esp8266/Arduino
#include <PubSubClient.h>       // https://github.com/knolleary/pubsubclient
#include <SoftwareSerial.h>     // https://github.com/plerup/espsoftwareserial
#include <ErriezMHZ19B.h>       // https://github.com/Erriez/ErriezMHZ19B

// Pin defines
#define MHZ19B_TX_PIN       D5
#define MHZ19B_RX_PIN       D6

// WiFi configuration
#define WIFI_SSID1          ""
#define WIFI_PASS1          ""
#define WIFI_SSID2          ""
#define WIFI_PASS2          ""
#define WIFI_CONNECT_TO_MS  6000

// MQTT configuration
#define MQTT_SERVER         ""
#define MQTT_PORT           1883
#define MQTT_USER           ""
#define MQTT_PASSWORD       ""
#define MQTT_TOPIC_PUB      "mh-z19b/co2"
#define MQTT_PUB_SEC        60

// Software serial for MH-Z19B
SoftwareSerial mhzSerial(MHZ19B_TX_PIN, MHZ19B_RX_PIN);

// MH-Z19B
ErriezMHZ19B mhz19b(&mhzSerial);

// Multi WiFi
ESP8266WiFiMulti wifiMulti;

// WiFi client
WiFiClient wifiClient;

// MQTT PubSubClient
PubSubClient mqtt(wifiClient);

// ESP8266 unique client ID
String clientId;


bool wifiConnect()
{
    // Maintain WiFi connection
    if (wifiMulti.run() == WL_CONNECTED) {
        Serial.print("WiFi: ");
        Serial.print(WiFi.SSID());
        Serial.print(" ");
        Serial.println(WiFi.localIP());
        return true;
    } else {
        Serial.println("WiFi not connected!");
        return false;
    }
}

bool mqttConnect()
{
    // Maintain WiFi connection
    if (wifiConnect()) {
        if (!mqtt.connected()) {
            Serial.print("MQTT connect...");

            // Connect to MQTT server
            if (mqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
                Serial.println("OK");
                return true;
            } else {
                Serial.printf(PSTR("Failed: %d\n"), mqtt.state());
            }
        }
    }

    return false;
}

void mqttPublish(const char *topic, const char *payload)
{
    // Maintain MQTT server connection
    if (mqttConnect()) {
        Serial.printf(PSTR("MQTT pub: %s, %s..."), topic, payload);
        if (mqtt.publish(topic, payload)) {
            Serial.println(F("OK"));
        } else {
            Serial.println(F("Failed"));
        }
    }
}

void mhz19bPrintErrorCode(int16_t result)
{
    // Print error code
    switch (result) {
        case MHZ19B_RESULT_ERR_CRC:
            Serial.println(F("CRC error"));
            break;
        case MHZ19B_RESULT_ERR_TIMEOUT:
            Serial.println(F("RX timeout"));
            break;
        default:
            Serial.print(F("Error: "));
            Serial.println(result);
            break;
    }
}

void mhz19bInit()
{
    char firmwareVersion[5];

    // Initialize senor software serial at fixed 9600 baudrate
    mhzSerial.begin(9600);

    // Optional: Detect MH-Z19B sensor (check wiring / power)
    while ( !mhz19b.detect() ) {
        Serial.println(F("Detecting MH-Z19B sensor..."));
        delay(2000);
    };

    // Sensor requires 3 minutes warming-up after power-on
    while (mhz19b.isWarmingUp()) {
        Serial.println(F("Warming up..."));
        delay(2000);
    };

    // Optional: Print firmware version
    Serial.print(F("  Firmware: "));
    mhz19b.getVersion(firmwareVersion, sizeof(firmwareVersion));
    Serial.println(firmwareVersion);

    // Optional: Set CO2 range 2000ppm or 5000ppm (default) once
    // Serial.print(F("Set range..."));
    // mhz19b.setRange2000ppm();
    // mhz19b.setRange5000ppm();

    // Optional: Print operating range
    Serial.print(F("  Range: "));
    Serial.print(mhz19b.getRange());
    Serial.println(F("ppm"));

    // Optional: Set automatic calibration on (true) or off (false) once
    // Serial.print(F("Set auto calibrate..."));
    // mhz19b.setAutoCalibration(true);

    // Optional: Print Automatic Baseline Calibration status
    Serial.print(F("  Auto calibrate: "));
    Serial.println(mhz19b.getAutoCalibration() ? F("On") : F("Off"));
}

void mhz19bPublishCO2(int16_t co2)
{
    static uint32_t tLast;
    uint32_t tNow;

    tNow = millis();
    if ((tNow - tLast) > (MQTT_PUB_SEC * 1000)) {
        tLast = tNow;

        // MQTT publish CO2 value
        mqttPublish(String(clientId + "/" + MQTT_TOPIC_PUB).c_str(),
                    String(co2).c_str());
    }
}

void setup()
{
    delay(500);
    Serial.begin(115200);
    Serial.println("\nErriez MH-Z19B ESP8266 Multi WiFi MQTT example");

    // Disconnect for testing purposes:
    // WiFi.disconnect();

    // Set WiFi to station mode
    WiFi.mode(WIFI_STA);

    // Register multi WiFi networks
    wifiMulti.addAP(WIFI_SSID1, WIFI_PASS1);
    wifiMulti.addAP(WIFI_SSID2, WIFI_PASS2);

    // Set MQTT server
    mqtt.setServer(MQTT_SERVER, MQTT_PORT);

    // Create unique MQTT address
    char buffer[15];
    snprintf_P(buffer, sizeof(buffer), PSTR("ESP8266-%04X"), ESP.getChipId());
    clientId = buffer;

    // Initialize MH-Z19B
    mhz19bInit();
}

void loop()
{
    int16_t result;

    // Minimum interval between CO2 reads is required
    if (mhz19b.isReady()) {
        // Read CO2 concentration from sensor
        result = mhz19b.readCO2();

        // Print result
        if (result < 0) {
            // An error occurred
            mhz19bPrintErrorCode(result);
        } else {
            // Print CO2 concentration in ppm
            Serial.print(result);
            Serial.println(F(" ppm"));

            // MQTT publish CO2 value
            mhz19bPublishCO2(result);
        }
    }
}