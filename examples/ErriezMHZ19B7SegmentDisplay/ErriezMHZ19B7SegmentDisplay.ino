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
 * \brief Erriez MHZ19B CO2 sensor project with 7-segment display and buzzer
 * \details
 *      Source:         https://github.com/Erriez/ErriezMHZ19B
 *      Documentation:  https://erriez.github.io/ErriezMHZ19B
 */

#include <ErriezMHZ19B.h>
#include <ErriezRobotDyn4DigitDisplay.h> // https://github.com/Erriez/ErriezRobotDyn4DigitDisplay

// Pin defines
#if defined(ARDUINO_AVR_MEGA2560) || defined(ARDUINO_AVR_LEONARDO) || defined(ARDUINO_SAM_DUE)
    #define TM1637_CLK_PIN      2
    #define TM1637_DIO_PIN      3
    #define BUZZER_PIN          6
    #define mhzSerial           Serial1 // Use second hardware serial
#elif defined(ARDUINO_ARCH_AVR)
    #define TM1637_CLK_PIN      2
    #define TM1637_DIO_PIN      3
    #define MHZ19B_TX_PIN       4
    #define MHZ19B_RX_PIN       5
    #define BUZZER_PIN          6

    #include <SoftwareSerial.h>          // Use software serial
    SoftwareSerial mhzSerial(MHZ19B_TX_PIN, MHZ19B_RX_PIN);
#elif defined(ARDUINO_ARCH_ESP8266)
    #define TM1637_CLK_PIN      D3
    #define TM1637_DIO_PIN      D4
    #define MHZ19B_TX_PIN       D5
    #define MHZ19B_RX_PIN       D6
    #define BUZZER_PIN          D7

    #include <SoftwareSerial.h>          // Use software serial
    SoftwareSerial mhzSerial(MHZ19B_TX_PIN, MHZ19B_RX_PIN);
#elif defined(ARDUINO_ARCH_ESP32)
    #define TM1637_CLK_PIN      16
    #define TM1637_DIO_PIN      17
    #define MHZ19B_TX_PIN       18
    #define MHZ19B_RX_PIN       19
    #define BUZZER_PIN          4

    #include <SoftwareSerial.h>          // Use software serial
    SoftwareSerial mhzSerial(MHZ19B_TX_PIN, MHZ19B_RX_PIN);
#else
    #error "May work, but not tested on this target"
#endif

// Create MHZ19B object
ErriezMHZ19B mhz19b(&mhzSerial);

// Create 7Segment display object
RobotDyn4DigitDisplay display(TM1637_CLK_PIN, TM1637_DIO_PIN);


void buzzerBeep(uint8_t beeps)
{
    // Beep a number of times
    for (uint8_t i = 0; i < beeps; i++) {
        digitalWrite(BUZZER_PIN, LOW);
        delay(50);
        digitalWrite(BUZZER_PIN, HIGH);
        delay(100);
    }
}

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

    // Initialize serial
    delay(500);
    Serial.begin(115200);
    Serial.println(F("\nErriez MH-Z19B CO2 sensor example"));

    // Initialize TM1637 display
    display.begin();
    display.clear();
    display.setBrightness(1);
    display.overflow();

    // Initialize software serial at fixed baudrate
    mhzSerial.begin(9600);

    // Detect sensor
    while ( !mhz19b.detect() ) {
        Serial.println(F("Detecting MH-Z19B sensor..."));
        delay(2000);
    };

    // Sensor requires 3 minutes warming-up after power-on
    while (mhz19b.isWarmingUp()) {
        Serial.println(F("Warming up..."));
        delay(2000);
    };

    // Print firmware version
    Serial.print(F("  Firmware: "));
    mhz19b.getVersion(firmwareVersion, sizeof(firmwareVersion));
    Serial.println(firmwareVersion);

    // Optional: Set CO2 range 2000ppm or 5000ppm (default) once
    // Serial.print(F("Set range..."));
    // mhz19b.setRange2000ppm();
    // mhz19b.setRange5000ppm();

    // Print operating range
    Serial.print(F("  Range: "));
    Serial.print(mhz19b.getRange());
    Serial.println(F("ppm"));

    // Set automatic calibration on (true) or off (false)
    // Serial.print(F("Set auto calibrate..."));
    // mhz19b.setAutoCalibration(true);

    // Print Automatic Baseline Calibration status
    Serial.print(F("  Auto calibrate: "));
    Serial.println(mhz19b.getAutoCalibration() ? F("On") : F("Off"));

    // Initialize buzzer pin
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, HIGH);
    buzzerBeep(3);
}

void loop()
{
    int16_t result;

    // Minimum interval between CO2 reads
    if (mhz19b.isReady()) {
        // Read CO2 from sensor
        result = mhz19b.readCO2();

        // Print result
        if (result < 0) {
            // Print to serial
            printErrorCode(result);

            // Update display
            display.overflow();
        } else {
            // Print to serial
            Serial.print(result);
            Serial.println(F(" ppm"));

            // Update display
            display.clear();
            display.dec(result);

            // Buzzer control
            if (result > 2000) {
                buzzerBeep(3);
            } else if (result > 1500) {
                buzzerBeep(2);
            } else if (result > 1000) {
                buzzerBeep(1);
            }
        }
    }
}
