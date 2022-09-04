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
 * \file ErriezMHZ19B.h
 * \brief MH-Z19B CO2 sensor library for Arduino
 * \details
 *      Source:         https://github.com/Erriez/ErriezMHZ19B
 *      Documentation:  https://erriez.github.io/ErriezMHZ19B
 */

#ifndef ERRIEZ_MHZ19B_H_
#define ERRIEZ_MHZ19B_H_

#include <Arduino.h>

//! Enable smart warming-up to return false when CO2 value changes within
//! 3 minutes pre-heating time.
//! Can be used when MCU is reset and sensor powered-up for >3 minutes.
//! Recommended to disable for deployment to ensure warming-up timing.
// #define MHZ19B_SMART_WARMING_UP

//! 3 minutes warming-up time after power-on before valid data returned
#define MHZ19B_WARMING_UP_TIME_MS       (3UL * 60000UL)

//! Minimum response time between CO2 reads (EXPERIMENTALLY DEFINED)
#define MHZ19B_READ_INTERVAL_MS         (5UL * 1000UL)

//! Fixed 9 Bytes response
#define MHZ19B_SERIAL_RX_BYTES          9

//! Response timeout between 15..120 ms at 9600 baud works reliable for all commands
#define MHZ19B_SERIAL_RX_TIMEOUT_MS     120

// Documented commands
#define MHZ19B_CMD_SET_AUTO_CAL         0x79 //!< Command set auto calibration on/off
#define MHZ19B_CMD_READ_CO2             0x86 //!< Command read CO2 concentration
#define MHZ19B_CMD_CAL_ZERO_POINT       0x87 //!< Command calibrate zero point at 400ppm
#define MHZ19B_CMD_CAL_SPAN_PIONT       0x88 //!< Command calibrate span point (NOT IMPLEMENTED)
#define MHZ19B_CMD_SET_RANGE            0x99 //!< Command set detection range

// Not documented commands
#define MHZ19B_CMD_GET_AUTO_CAL         0x7D //!< Command get auto calibration status (NOT DOCUMENTED)
#define MHZ19B_CMD_GET_RANGE            0x9B //!< Command get range detection (NOT DOCUMENTED)
#define MHZ19B_CMD_GET_VERSION          0xA0 //!< Command get firmware version (NOT DOCUMENTED)

/*!
 * \brief Response on a command
 */
typedef enum {
    MHZ19B_RESULT_OK = 0,                //!< Response OK
    MHZ19B_RESULT_ERROR = -1,            //!< Response error
    MHZ19B_RESULT_ERR_CRC = -2,          //!< Response CRC error
    MHZ19B_RESULT_ERR_TIMEOUT = -3,      //!< Response timeout
    MHZ19B_RESULT_ARGUMENT_ERROR = -4,   //!< Response argument error
} MHZ19B_Result_e;

/*!
 * \brief PPM range
 */
typedef enum {
    MHZ19B_RANGE_2000 = 2000,            //!< Range 2000 ppm
    MHZ19B_RANGE_5000 = 5000,            //!< Range 5000 ppm (Default)
} MHZ19B_Range_e;


/*!
 * \brief Class ErriezMHZ19B
 */
class ErriezMHZ19B
{
public:
    ErriezMHZ19B(Stream *serial);
    ~ErriezMHZ19B();

    // Detect sensor by checking range response
    bool detect();

    // Minimum wait time after power-on
    bool isWarmingUp();

    // Minimum wait time between CO2 reads
    bool isReady();

    // Read CO2 value
    int16_t readCO2();

    // Get firmware version (NOT DOCUMENTED)
    int8_t getVersion(char *version, uint8_t versionLen);

    // Set/get CO2 range, default 5000ppm)
    int8_t setRange2000ppm();
    int8_t setRange5000ppm();
    int16_t getRange(); // (NOT DOCUMENTED)

    // Set and get ABC, default on (Automatic Baseline Correction)
    int8_t setAutoCalibration(bool calibrationOn);
    int8_t getAutoCalibration(); // (NOT DOCUMENTED)

    // Start Zero Point Calibration manually at 400ppm
    int8_t startZeroCalibration();
    
    // Serial communication
    int8_t sendCommand(uint8_t cmd, byte b3=0, byte b4=0, byte b5=0, byte b6=0, byte b7=0);

    // Global receive buffer
    uint8_t rxBuffer[MHZ19B_SERIAL_RX_BYTES];

private:
    Stream *_serial;                //!< Serial Stream pointer
    unsigned long _tLastReadCO2;    //!< Timestamp between readCO2() calls

    // CRC check on serial transmit/receive buffer
    uint8_t calcCRC(uint8_t *data);
};

#endif // ERRIEZ_MHZ19B_H_
