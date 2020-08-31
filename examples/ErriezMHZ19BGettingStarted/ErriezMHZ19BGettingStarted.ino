/*
 * MIT License
 *
 * Copyright (c) 2020 Erriez
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
 * \brief Erriez MHZ19B CO2 sensor example for Arduino
 * \details
 *      Source:         https://github.com/Erriez/ErriezMHZ19B
 *      Documentation:  https://erriez.github.io/ErriezMHZ19B
 */

/* Possible output:
    Erriez MH-Z19B CO2 Sensor example
    Warming up...
    ...
    Warming up...
      Firmware: 0443
      Range: 5000ppm
      Auto calibrate: On
    695 ppm
    696 ppm
    696 ppm
    ...
*/

#include <ErriezMHZ19B.h>

// Pin defines
#if defined(ARDUINO_AVR_MEGA2560) || defined(ARDUINO_AVR_LEONARDO) || defined(ARDUINO_SAM_DUE)
    #define mhzSerial           Serial1 // Use second hardware serial

#elif defined(ARDUINO_ARCH_AVR)
    #define MHZ19B_TX_PIN        4
    #define MHZ19B_RX_PIN        5

    #include <SoftwareSerial.h>          // Use software serial
    SoftwareSerial mhzSerial(MHZ19B_TX_PIN, MHZ19B_RX_PIN);
#elif defined(ARDUINO_ARCH_ESP8266)
    #define MHZ19B_TX_PIN        D5
    #define MHZ19B_RX_PIN        D6

    #include <SoftwareSerial.h>          // Use software serial
    SoftwareSerial mhzSerial(MHZ19B_TX_PIN, MHZ19B_RX_PIN);
#elif defined(ARDUINO_ARCH_ESP32)
    #define MHZ19B_TX_PIN        18
    #define MHZ19B_RX_PIN        19

    #include <SoftwareSerial.h>          // Use software serial
    SoftwareSerial mhzSerial(MHZ19B_TX_PIN, MHZ19B_RX_PIN);
#else
    #error "May work, but not tested on this target"
#endif

// Create MHZ19B object
ErriezMHZ19B mhz19b(&mhzSerial);


void printErrorCode(int16_t result)
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

void setup()
{
    char firmwareVersion[5];

    // Initialize serial port to print diagnostics and CO2 output
    Serial.begin(115200);
    Serial.println(F("\nErriez MH-Z19B CO2 Sensor example"));

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
            printErrorCode(result);
        } else {
            // Print CO2 concentration in ppm
            Serial.print(result);
            Serial.println(F(" ppm"));
        }
    }
}