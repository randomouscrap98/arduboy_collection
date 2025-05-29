#!/bin/sh

set -e

ardugotools fxdata generate

DEVICE="${1:-fx}"

arduino-cli compile --fqbn "arduboy-homemade:avr:arduboy-$DEVICE" --optimize-for-debug --output-dir dist --build-path build
