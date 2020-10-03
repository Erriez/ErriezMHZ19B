#/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

echo "Starting auto-build script..."


function autobuild()
{
    # Set environment variables
    BOARDS_AVR="--board uno --board micro --board pro16MHzatmega328 --board pro8MHzatmega328 --board megaatmega2560 --board leonardo"
    BOARDS_ARM="--board due"
    BOARDS_ESP8266="--board d1_mini --board nodemcuv2"
    BOARDS_ESP32="--board lolin_d32"

    echo "Installing library dependencies"
    platformio lib --global install https://github.com/Erriez/ErriezTM1637
    platformio lib --global install https://github.com/Erriez/ErriezRobotDyn4DigitDisplay
    platformio lib --global install https://github.com/knolleary/pubsubclient

    echo "Install ESPSoftwareSerial into framework-arduinoespressif32 to prevent conflicts with generic name SoftwareSerial"
    mkdir -p ~/.platformio/packages/framework-arduinoespressif32/libraries
    platformio lib --storage-dir ~/.platformio/packages/framework-arduinoespressif32/libraries install "ESPSoftwareSerial"

    echo "Build examples"
    platformio ci --lib="." ${BOARDS_AVR} ${BOARDS_ARM} ${BOARDS_ESP8266} ${BOARDS_ESP32} examples/ErriezMHZ19B7SegmentDisplay/ErriezMHZ19B7SegmentDisplay.ino
    platformio ci --lib="." ${BOARDS_AVR} ${BOARDS_ARM} ${BOARDS_ESP8266} ${BOARDS_ESP32} examples/ErriezMHZ19BGettingStarted/ErriezMHZ19BGettingStarted.ino
    platformio ci --lib="." ${BOARDS_AVR} ${BOARDS_ARM} ${BOARDS_ESP8266} ${BOARDS_ESP32} examples/ErriezMHZ19BSerialPlottter/ErriezMHZ19BSerialPlottter.ino
    platformio ci --lib="."                             ${BOARDS_ESP8266}                 examples/ErriezMHZ19BESP8266MQTT/ErriezMHZ19BESP8266MQTT.ino
}

function generate_doxygen()
{
    echo "Generate Doxygen HTML..."

    DOXYGEN_PDF="ErriezMHZ19B.pdf"

    # Cleanup
    rm -rf html latex

    # Generate Doxygen HTML and Latex
    doxygen Doxyfile

    # Allow filenames starting with an underscore    
    echo "" > html/.nojekyll

    # Generate PDF when script is not running on Travis-CI
    if [[ -z ${TRAVIS_BUILD_DIR} ]]; then
        # Generate Doxygen PDF
        make -C latex

        # Copy PDF to root directory
        cp latex/refman.pdf ./${DOXYGEN_PDF}
    fi
}

autobuild
generate_doxygen
