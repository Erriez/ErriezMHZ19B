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
 * \file ErriezMHZ19B.cpp
 * \brief MH-Z19B CO2 sensor library for Arduino
 * \details
 *      This sensor library is re-build from scratch.
 *
 *      Design choices:
 *      - Keep code and memory size as small as possible.
 *      - Use documented functions as much as possible for reliability and to prevent bricking the
 *        sensor.
 *      - PWM not implemented in this library, because it is not accurate and reduces code size.
 *
 *      Source:         https://github.com/Erriez/ErriezMHZ19B
 *      Documentation:  https://erriez.github.io/ErriezMHZ19B
 */

#include "ErriezMHZ19B.h"


/*!
 * \brief Constructor with serial Stream
 * \param serial
 *      Serial Stream pointer.
 */
ErriezMHZ19B::ErriezMHZ19B(Stream *serial) : _serial(serial), _tLastReadCO2(0)
{
}

/*!
 * \brief Destructor
 * \details
 *      The serial Stream pointer is cleared and requires a new constructor to reuse it again.
 */
ErriezMHZ19B::~ErriezMHZ19B()
{
    _serial = nullptr;
}

/*!
 * \brief Detect MHZ19B sensor
 * \retval true
 *      Sensor detected.
 * \retval false
 *      Sensor not detected.
 */
bool ErriezMHZ19B::detect()
{
    // Check valid PPM range
    if (getRange() > 0) {
        return true;
    }

    // Sensor not detected, or invalid range returned
    // Try recover by calling setRange(MHZ19B_RANGE_5000);
    return false;
}

/*!
 * \brief Check if sensor is warming-up after power-on
 * \details
 *      The datasheet mentions a startup delay of 3 minutes before reading CO2.\n
 *      Experimentally discovered, the sensor may return CO2 data earlier. To speed-up the boot
 *      process, it is possible to check if the CO2 value changes to abort the warming-up, for
 *      example when the MCU is reset and keep the sensor powered.\n
 *      Recommended to disable this option for deployment by disabling macro
 *      MHZ19B_SMART_WARMING_UP in header file.
 * \retval true
 *      Sensor is warming-up.
 * \retval false
 *      Sensor is ready to use.
 */
bool ErriezMHZ19B::isWarmingUp()
{
    // Wait at least 3 minutes after power-on
    if (millis() < MHZ19B_WARMING_UP_TIME_MS) {
#ifdef MHZ19B_SMART_WARMING_UP
        static int16_t _lastCO2 = -1;
        int16_t co2;

        // Sensor returns valid data after CPU reset and keep sensor powered
        co2 = readCO2();
        if (_lastCO2 == -1) {
            _lastCO2 = co2;
        } else {
            if (_lastCO2 != co2) {
                // CO2 value changed since last read, no longer warming-up
                _tLastReadCO2 = 0;
                return false;
            }
        }
#endif
        // Warming-up
        return true;
    }

    // Not warming-up
    return false;
}

/*!
 * \brief Check minimum interval between CO2 reads
 * \details
 *      Not described in the datasheet, but it is the same frequency as the built-in LED blink.
 * \return
 */
bool ErriezMHZ19B::isReady()
{
    // Minimum CO2 read interval (Built-in LED flashes)
    if ((millis() - _tLastReadCO2) > MHZ19B_READ_INTERVAL_MS) {
        return true;
    }

    return false;
}

/*!
 * \brief Read CO2 from sensor
 * \retval <0
 *      MH-Z19B response error codes.
 * \retval 0..399 ppm
 *      Incorrect values. Minimum value starts at 400ppm outdoor fresh air.
 * \retval 400..1000 ppm
 *      Concentrations typical of occupied indoor spaces with good air exchange.
 * \retval 1000..2000 ppm
 *      Complaints of drowsiness and poor air quality. Ventilation is required.
 * \retval 2000..5000 ppm
 *      Headaches, sleepiness and stagnant, stale, stuffy air. Poor concentration, loss of
 *      attention, increased heart rate and slight nausea may also be present.\n
 *      Higher values are extremely dangerous and cannot be measured.
 */
int16_t ErriezMHZ19B::readCO2()
{
    int16_t result;

    // Set timestamp
    _tLastReadCO2 = millis();
    
    // Send command "Read CO2 concentration"
    result = sendCommand(MHZ19B_CMD_READ_CO2);

    // Check result
    if (result == MHZ19B_RESULT_OK) {
        result = (_response[2] << 8) | _response[3];
    }

    return result;
}

/*!
 * \brief Get firmware version (NOT DOCUMENTED)
 * \details
 *      This is an undocumented command, but most sensors returns ASCII "0430 or "0443".
 * \param version
 *      Returned character pointer to version (must be at least 5 Bytes)\n
 *      Only valid when return is set to MHZ19B_RESULT_OK.
 * \param versionLen
 *      Number of characters including NULL of version buffer.
 * \return
 *      MH-Z19B response error codes.
 */
int8_t ErriezMHZ19B::getVersion(char *version, uint8_t versionLen)
{
    int8_t result;

    // Argument check
    if (versionLen < 5) {
        return MHZ19B_RESULT_ARGUMENT_ERROR;
    }

    // Clear version
    memset(version, 0, 5);

    // Send command "Read firmware version" (NOT DOCUMENTED)
    result = sendCommand(MHZ19B_CMD_GET_VERSION);

    // Check result
    if (result == MHZ19B_RESULT_OK) {
        // Copy 4 ASCII characters to version array like "0443"
        for (uint8_t i = 0; i < 4; i++) {
            version[i] = _response[i + 2];
        };
    }

    return result;
}

/*!
 * \brief Set CO2 range 2000 ppm
 * \return
 *      MH-Z19B response error codes.
 */
int8_t ErriezMHZ19B::setRange2000ppm()
{
    // Send "Set range" command
    return sendCommand(MHZ19B_CMD_SET_RANGE,
                       0x00, 0x00, 0x00, (MHZ19B_RANGE_2000 >> 8), (MHZ19B_RANGE_2000 & 0xff));
}

/*!
 * \brief Set CO2 range 5000 ppm
 * \return
 *      MH-Z19B response error codes.
 */
int8_t ErriezMHZ19B::setRange5000ppm()
{
    // Send "Set range" command
    return sendCommand(MHZ19B_CMD_SET_RANGE,
                       0x00, 0x00, 0x00, (MHZ19B_RANGE_5000 >> 8), (MHZ19B_RANGE_5000 & 0xff));
}

/*!
 * \brief Get CO2 range in PPM (NOT DOCUMENTED)
 * \details
 *      This function verifies valid read ranges of 2000 or 5000 ppm.\n
 *      Note: Other ranges may be returned, but are undocumented and marked as invalid.
 * \retval <0
 *      MH-Z19B response error codes.
 * \retval MHZ19B_RANGE_2000
 *      Range 2000 ppm.
 * \retval MHZ19B_RANGE_5000
 *      Range 5000 ppm (default).
 */
int16_t ErriezMHZ19B::getRange()
{
    int16_t result;

    // Send command "Read range" (NOT DOCUMENTED)
    result = sendCommand(MHZ19B_CMD_GET_RANGE);

    // Check result
    if (result == MHZ19B_RESULT_OK) {
        result = (_response[4] << 8) | _response[5];

        // Check range according to documented specification
        if ((result != MHZ19B_RANGE_2000) && (result != MHZ19B_RANGE_5000)) {
            result = MHZ19B_RESULT_ERROR;
        }
    }

    return result;
}

/*!
 * \brief Enable or disable automatic calibration
 * \param calibrationOn
 *      true: Automatic calibration on.\n
 *      false: Automatic calibration off.
 * \return
 *      MH-Z19B response error codes.
 */
int8_t ErriezMHZ19B::setAutoCalibration(bool calibrationOn)
{
    // Send command "Write Automatic Baseline Correction (ABC logic function)"
    return sendCommand(MHZ19B_CMD_SET_AUTO_CAL, (calibrationOn ? 0xA0 : 0x00));
}

/*!
 * \brief Get status automatic calibration (NOT DOCUMENTED)
 * \retval <0
 *      MH-Z19B response error codes.
 * \retval true
 *      Automatic calibration on.
 * \retval false
 *      Automatic calibration off.
 */
int8_t ErriezMHZ19B::getAutoCalibration()
{
    int8_t result;

    // Send command "Read Automatic Baseline Correction (ABC logic function)" (NOT DOCUMENTED)
    result = sendCommand(MHZ19B_CMD_GET_AUTO_CAL);

    // Check result
    if (result == MHZ19B_RESULT_OK) {
        if (_response[7] == 0x01) {
            // On
            result = 1;
        } else if (_response[7] == 0x00) {
            // Off
            result = 0;
        }
    }

    return result;
}

/*!
 * \brief Start Zero Point Calibration manually at 400ppm
 * \details
 *      The sensor must be powered-up for at least 20 minutes in fresh air at 400ppm room
 *      temperature. Then call this function once to execute self calibration.\n
 *      Recommended to use this function when auto calibrate turned off.
 * \return
 *      MH-Z19B response error codes.
 */
int8_t ErriezMHZ19B::startZeroCalibration()
{
    // Send command "Zero Point Calibration"
    return sendCommand(MHZ19B_CMD_CAL_ZERO_POINT);
}

/*!
 * \brief Send serial command to sensor
 * \details
 *      Send command to sensor. Then retrieve response from sensor with receiveResponse().
 * \param cmd
 *      Command Byte
 * \param b3
 *      Byte 3
 * \param b4
 *      Byte 4
 * \param b5
 *      Byte 5
 * \param b6
 *      Byte 6
 * \param b7
 *      Byte 7
 */
int8_t ErriezMHZ19B::sendCommand(uint8_t cmd, byte b3, byte b4, byte b5, byte b6, byte b7)
{
    uint8_t txBuffer[9] = { 0xFF, 0x01, cmd, b3, b4, b5, b6, b7, calcCRC(txBuffer) };

    // Save command for response verification
    _cmd = cmd;

    // Write data to sensor
    serialWrite(txBuffer, sizeof(txBuffer));

    // Read response
    return receiveResponse(_response, sizeof(_response));
}

/*!
 * \brief Receive serial response from sensor
 * \param rxBuffer
 *      Receive buffer (must be 9 Bytes).
 * \param rxBufferLength
 *      Receive buffer size.
 * \return
 *      MH-Z19B response error codes.
 */
int8_t ErriezMHZ19B::receiveResponse(uint8_t *rxBuffer, uint8_t rxBufferLength)
{
    int8_t result = MHZ19B_RESULT_OK;
    unsigned long tStart;
    
    // Argument check
    if (rxBufferLength < MHZ19B_RESPONSE_LENGTH) {
        return MHZ19B_RESULT_ARGUMENT_ERROR;
    }

    // Clear receive buffer
    memset(rxBuffer, 0, MHZ19B_RESPONSE_LENGTH);
    
    // Wait until all data received from sensor
    tStart = millis();
    while (serialAvailable() < rxBufferLength) {
        if ((millis() - tStart) >= MHZ19B_SERIAL_RX_TIMEOUT_MS) {
            return MHZ19B_RESULT_ERR_TIMEOUT;
        }
    }

    // Read response from serial buffer
    serialRead(rxBuffer, MHZ19B_RESPONSE_LENGTH);
    
    // Check received Byte[0] == 0xFF and Byte[1] == transmit command
    if ((rxBuffer[0] != 0xFF) || (rxBuffer[1] != _cmd)) {
        result = MHZ19B_RESULT_ERROR;
    }
    
    // Check received Byte[8] CRC
    if (rxBuffer[8] != calcCRC(rxBuffer)) {
        result = MHZ19B_RESULT_ERR_CRC;
    }
    
    // Return result
    return result;
}

// ----------------------------------------------------------------------------
// Private functions
// ----------------------------------------------------------------------------

/*!
 * \brief Calculate CRC
 * \param data
 *      Buffer pointer to calculate CRC.
 * \return
 *      Calculated 8-bit CRC.
 */
uint8_t ErriezMHZ19B::calcCRC(uint8_t *data)
{
    byte crc = 0;

    for (uint8_t i = 1; i < 8; i++) {
        crc += data[i];
    }
    crc = 0xFF - crc;
    crc++;

    return crc;
}

/*!
 * \brief Get number of serial Bytes received
 * \return
 *      Number of Bytes in receive buffer.
 */
int ErriezMHZ19B::serialAvailable()
{
    if (_serial) {
        return _serial->available();
    }

    return 0;
}

/*!
 * \brief Write buffer to serial stream
 * \param txBuffer
 *      Transmit buffer pointer.
 * \param txLen
 *      Transmit buffer length in Bytes.
 */
void ErriezMHZ19B::serialWrite(uint8_t *txBuffer, uint8_t txLen)
{
    if (_serial) {
        // Clear receive buffer
        while (_serial->available()) {
            _serial->read();
        }

        // Write serial data
        _serial->write(txBuffer, txLen);

        // Flush serial data
        _serial->flush();
    }
}

/*!
 * \brief Read buffer from serial stream
 * \param rxBuffer
 *      Receive buffer pointer.
 * \param rxLen
 *      Receive buffer length.
 */
void ErriezMHZ19B::serialRead(uint8_t *rxBuffer, uint8_t rxLen)
{
    if (_serial) {
        _serial->readBytes(rxBuffer, rxLen);
    }
}
