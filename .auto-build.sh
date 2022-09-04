#!/bin/bash
#
#  MIT License
#
#  Copyright (c) 2020-2022 Erriez
#
#  Permission is hereby granted, free of charge, to any person obtaining a copy
#  of this software and associated documentation files (the "Software"), to deal
#  in the Software without restriction, including without limitation the rights
#  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#  copies of the Software, and to permit persons to whom the Software is
#  furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

# Automatic build script
#
# To run locally, execute:
# sudo apt install doxygen graphviz texlive-latex-base texlive-latex-recommended texlive-pictures texlive-latex-extra

# Exit immediately if a command exits with a non-zero status.
set -e

DOXYGEN_PDF="ErriezMHZ19B.pdf"


echo "Starting auto-build script..."

function setup_virtualenv()
{
    if [ ! -d ".venv" ]; then
        virtualenv .venv
        source .venv/bin/activate
        pip3 install platformio==6.1.4
        deactivate
    fi

    source .venv/bin/activate
}

function autobuild()
{
    # Set environment variables
    BOARDS_AVR="--board uno --board micro --board pro16MHzatmega328 --board pro8MHzatmega328 --board megaatmega2560 --board leonardo"
    BOARDS_ARM="--board due"
    BOARDS_ESP8266="--board nodemcuv2"
    BOARDS_ESP32="--board lolin_d32"

    echo "Installing library dependencies"
    pio pkg install --global --library https://github.com/Erriez/ErriezTM1637
    pio pkg install --global --library https://github.com/Erriez/ErriezRobotDyn4DigitDisplay
    pio pkg install --global --library https://github.com/knolleary/pubsubclient

    echo "Install ESPSoftwareSerial into framework-arduinoespressif32 to prevent conflicts with generic name SoftwareSerial"
    mkdir -p ~/.platformio/packages/framework-arduinoespressif32/libraries
    platformio lib --storage-dir ~/.platformio/packages/framework-arduinoespressif32/libraries install "EspSoftwareSerial@6.16.1"
    # Cannot upgrade to pio pkg install command: https://github.com/platformio/platformio-core/issues/4410
    #pio pkg install --storage-dir ~/.platformio/packages/framework-arduinoespressif32/libraries --library "EspSoftwareSerial@6.16.1"
    
    echo "Building examples..."

    # Use option -O "lib_ldf_mode=chain+" to parse defines
    pio ci -O "lib_ldf_mode=chain+" --lib="." ${BOARDS_AVR} ${BOARDS_ARM} ${BOARDS_ESP8266} ${BOARDS_ESP32} examples/ErriezMHZ19B7SegmentDisplay/ErriezMHZ19B7SegmentDisplay.ino
    pio ci -O "lib_ldf_mode=chain+" --lib="." ${BOARDS_AVR} ${BOARDS_ARM} ${BOARDS_ESP8266} ${BOARDS_ESP32} examples/ErriezMHZ19BGettingStarted/ErriezMHZ19BGettingStarted.ino
    pio ci -O "lib_ldf_mode=chain+" --lib="." ${BOARDS_AVR} ${BOARDS_ARM} ${BOARDS_ESP8266} ${BOARDS_ESP32} examples/ErriezMHZ19BSerialPlottter/ErriezMHZ19BSerialPlottter.ino
    pio ci -O "lib_ldf_mode=chain+" --lib="."                             ${BOARDS_ESP8266}                 examples/ErriezMHZ19BESP8266MQTT/ErriezMHZ19BESP8266MQTT.ino
}

function generate_doxygen()
{
    if [ ! -f Doxyfile ]; then
        return
    fi

    echo "Generate Doxygen HTML..."

    # Generate Doxygen HTML and Latex
    doxygen Doxyfile

    # Allow filenames starting with an underscore
    echo "" > docs/html/.nojekyll

    # Generate Doxygen PDF
    make -C docs/latex

    # Copy PDF to root directory
    cp docs/latex/refman.pdf ./${DOXYGEN_PDF}

    # Cleanup
    #rm -rf docs
}

setup_virtualenv
autobuild
generate_doxygen
