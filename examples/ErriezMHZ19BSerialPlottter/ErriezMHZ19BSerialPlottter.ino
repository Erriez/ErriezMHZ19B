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
 * \brief Erriez MHZ19B CO2 sensor S Plotter example for Arduino
 * \details
 *      Start the Arduino IDE | Tools | Serial Plotter...
 *      Wait at least 3 minutes warming-up time to display graph
 *
 *      Source:         https://github.com/Erriez/ErriezMHZ19B
 *      Documentation:  https://erriez.github.io/ErriezMHZ19B
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


void setup()
{
    // Initialize serial
    Serial.begin(115200);

    // Initialize software serial at fixed baudrate
    mhzSerial.begin(9600);

    // Detect sensor
    while ( !mhz19b.detect() ) {
        delay(2000);
    };

    // Wait for warming-up completion
    while ( mhz19b.isWarmingUp() ) {
        delay(2000);
    };
}

void loop()
{
    // Minimum interval between CO2 reads
    if (mhz19b.isReady()) {
        // Read and print CO2
        Serial.println(mhz19b.readCO2());
    }
}